/* esp32-firmware
 * Copyright (C) 2024 Olaf Lüke <olaf@tinkerforge.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

// QCA7000/QCA7005 SPI driver for ESP32

#include "qca700x.h"

#include "event_log_prefix.h"
#include "generated/module_dependencies.h"
#include "build.h"

#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <esp_heap_caps.h>
#include "esp_vfs_l2tap.h"
#include "esp_netif.h"
#include "esp_mac.h"

#include "slac.h"

#include "lwip/ip_addr.h"
#include "bindings/hal_common.h"

#include "tools/freertos.h"
#include "tools/string_builder.h"

#include "gcc_warnings.h"


/**** Future considerations ****
If we need faster QCA data transfer in the future, we can drive the dma trigger by the io_scheduler:

// io_scheduler API (new)
io_scheduler.register_poll_hook([this]() { qca700x.io_tick(); });

// task_loop becomes:
for (;;) {
    watchdog reset
    tf_hal_tick(&hal, 0);
    run_poll_hooks(); // <- QCA gets serviced here, every ~1 ms
    ... pending_await handling ...
    ... due jobs at JOB priority ...
}

Ownership: the QCA SPI device becomes io-task-owned, like the Bricklet HAL.
Frame delivery: esp_netif_receive() and the l2tap filter are already designed to be called from a driver's own task


Variant A: IO task only drains, main task parses
Variant B: Parsing also moves to the io task (full starvation immunity against http task)

Variant B may actually be necessary for ISO 15118-20, because of the TLS timing constraints.
*/

#define QCA700X_SPI_CHIP_SELECT_PIN 4
#define QCA700X_SPI_MISO_PIN 39 // SENSOR_VN
#define QCA700X_SPI_MOSI_PIN 15
#define QCA700X_SPI_CLOCK_PIN 2

bool QCA700x::spi_init()
{
    spi_bus_config_t bus_config = {};
    bus_config.mosi_io_num     = QCA700X_SPI_MOSI_PIN;
    bus_config.miso_io_num     = QCA700X_SPI_MISO_PIN;
    bus_config.sclk_io_num     = QCA700X_SPI_CLOCK_PIN;
    bus_config.quadwp_io_num   = -1;
    bus_config.quadhd_io_num   = -1;
    bus_config.max_transfer_sz = QCA700X_DMA_SCRATCH_SIZE + 4;

    esp_err_t err = spi_bus_initialize(SPI3_HOST, &bus_config, SPI_DMA_CH_AUTO);
    if (err != ESP_OK) {
        iso15118.trace("QCA700x: spi_bus_initialize failed: %s", esp_err_to_name(err));
        return false;
    }

    // Mode 3 (CPOL=1, CPHA=1), 1.4 MHz, 16-bit command phase, hardware CS.
    spi_device_interface_config_t device_config = {};
    device_config.command_bits   = 16;
    device_config.mode           = 3;
    device_config.clock_speed_hz = 1400000;
    device_config.spics_io_num   = QCA700X_SPI_CHIP_SELECT_PIN;
    device_config.queue_size     = 1;

    err = spi_bus_add_device(SPI3_HOST, &device_config, &spi_dev);
    if (err != ESP_OK) {
        iso15118.trace("QCA700x: spi_bus_add_device failed: %s", esp_err_to_name(err));
        spi_bus_free(SPI3_HOST);
        spi_dev = nullptr;
        return false;
    }

    dma_scratch = static_cast<uint8_t *>(heap_caps_malloc(QCA700X_DMA_SCRATCH_SIZE + 4, MALLOC_CAP_DMA));
    if (dma_scratch == nullptr) {
        iso15118.trace("QCA700x: Failed to allocate DMA scratch buffer");
        spi_bus_remove_device(spi_dev);
        spi_dev = nullptr;
        spi_bus_free(SPI3_HOST);
        return false;
    }

    return true;
}

uint16_t QCA700x::read_register(const uint16_t reg)
{
    if (spi_dev == nullptr) {
        return 0xFFFF; // Same value as reading from a disconnected bus
    }

    finish_pending_op(true);

    spi_transaction_t transaction = {};
    transaction.cmd    = static_cast<uint16_t>(QCA700X_SPI_READ | QCA700X_SPI_INTERNAL | reg);
    transaction.length = 16; // Data phase: 16-bit register value
    transaction.flags  = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;

    esp_err_t err = spi_device_polling_transmit(spi_dev, &transaction);
    if (err != ESP_OK) {
        iso15118.trace("QCA700x: Read of register 0x%04X failed: %s", reg, esp_err_to_name(err));
        return 0xFFFF;
    }

    return static_cast<uint16_t>((transaction.rx_data[0] << 8) | transaction.rx_data[1]);
}

void QCA700x::write_register(const uint16_t reg, const uint16_t value)
{
    if (spi_dev == nullptr) {
        return;
    }

    finish_pending_op(true);

    spi_transaction_t transaction = {};
    transaction.cmd        = static_cast<uint16_t>(QCA700X_SPI_WRITE | QCA700X_SPI_INTERNAL | reg);
    transaction.length     = 16; // Data phase: 16-bit register value
    transaction.flags      = SPI_TRANS_USE_TXDATA;
    transaction.tx_data[0] = static_cast<uint8_t>(value >> 8);
    transaction.tx_data[1] = static_cast<uint8_t>(value & 0xFF);

    esp_err_t err = spi_device_polling_transmit(spi_dev, &transaction);
    if (err != ESP_OK) {
        iso15118.trace("QCA700x: Write of register 0x%04X failed: %s", reg, esp_err_to_name(err));
    }
}

// Reads length bytes from the QCA700x external read buffer into dma_scratch.
// The BFR_SIZE register write must directly precede the external read, so
// nothing else may use the SPI device in between.
bool QCA700x::external_read_to_scratch(const uint16_t length)
{
    write_register(QCA700X_SPI_REG_BFR_SIZE, length);

    spi_transaction_t transaction = {};
    transaction.cmd       = static_cast<uint16_t>(QCA700X_SPI_READ | QCA700X_SPI_EXTERNAL);
    transaction.length    = static_cast<size_t>(length) * 8;
    transaction.rx_buffer = dma_scratch;

    esp_err_t err = spi_device_polling_transmit(spi_dev, &transaction);
    if (err != ESP_OK) {
        iso15118.trace("QCA700x: External read of %u bytes failed: %s", length, esp_err_to_name(err));
        return false;
    }

    return true;
}

// Starts an asynchronous DMA burst read into dma_scratch if the modem has data available.
void QCA700x::start_burst_read()
{
    if (spi_dev == nullptr || read_in_flight || write_in_flight) {
        return;
    }

    const uint16_t rdbuf_byte_ava = read_register(QCA700X_SPI_REG_RDBUF_BYTE_AVA);

    // 0xFFFF typically indicates no modem connected (SPI returns all 1s when no device)
    if (rdbuf_byte_ava == 0xFFFF) {
        return;
    }

    // Cap at the free space in the reassembly buffer and at the DMA scratch
    // buffer size. A bigger backlog is drained over multiple polls.
    // Frames split between reads are reassembled in spi_buffer.
    const uint16_t space = static_cast<uint16_t>(QCA700X_BUFFER_SIZE + QCA700X_HW_PKT_SIZE - spi_buffer_length);
    const uint16_t available = std::min({rdbuf_byte_ava, space, static_cast<uint16_t>(QCA700X_DMA_SCRATCH_SIZE)});
    if (available == 0) {
        return;
    }

    // The BFR_SIZE register write must directly precede the external read,
    // so nothing else may use the SPI device until the read completed.
    write_register(QCA700X_SPI_REG_BFR_SIZE, available);

    read_transaction           = {};
    read_transaction.cmd       = static_cast<uint16_t>(QCA700X_SPI_READ | QCA700X_SPI_EXTERNAL);
    read_transaction.length    = static_cast<size_t>(available) * 8;
    read_transaction.rx_buffer = dma_scratch;

    esp_err_t err = spi_device_queue_trans(spi_dev, &read_transaction, 0);
    if (err != ESP_OK) {
        iso15118.trace("QCA700x: Failed to queue burst read of %u bytes: %s", available, esp_err_to_name(err));
        return;
    }

    read_length = available;
    read_in_flight = true;
}

// Picks up a completed asynchronous burst read and appends the data to spi_buffer.
// Returns true if no read is in flight.
bool QCA700x::finish_burst_read(const bool wait)
{
    if (!read_in_flight) {
        return true;
    }

    spi_transaction_t *finished = nullptr;
    esp_err_t err = spi_device_get_trans_result(spi_dev, &finished, wait ? portMAX_DELAY_nowarn : 0);
    if (err == ESP_ERR_TIMEOUT) {
        return false; // Still running
    }

    read_in_flight = false;

    if (err != ESP_OK) {
        iso15118.trace("QCA700x: Burst read of %u bytes failed: %s", read_length, esp_err_to_name(err));
        return true;
    }

    memcpy(&spi_buffer[spi_buffer_length], dma_scratch, read_length);
    spi_buffer_length = static_cast<uint16_t>(spi_buffer_length + read_length);

    iso15118.trace_packet(dma_scratch, read_length);

    return true;
}

// Starts an asynchronous DMA burst write of the oldest pending TX frame.
void QCA700x::start_burst_write()
{
    if (spi_dev == nullptr || read_in_flight || write_in_flight || tx_queue_count == 0) {
        return;
    }

    TxFrame *frame = &tx_queue[tx_queue_head];

    // Modem write buffer flow control at issue time. 0xFFFF indicates no
    // modem connected. Issue the write anyway (it goes into the void), so
    // the queue can't clog while the modem is absent or resetting.
    const uint16_t wrbuf_spc_ava = read_register(QCA700X_SPI_REG_WRBUF_SPC_AVA);
    if (wrbuf_spc_ava != 0xFFFF && wrbuf_spc_ava < frame->total_length) {
        // No space yet. Retried on an upcoming poll.
        return;
    }

    // The BFR_SIZE register write must directly precede the external write,
    // so nothing else may use the SPI device until the write completed.
    write_register(QCA700X_SPI_REG_BFR_SIZE, frame->total_length);

    write_transaction           = {};
    write_transaction.cmd       = static_cast<uint16_t>(QCA700X_SPI_WRITE | QCA700X_SPI_EXTERNAL);
    write_transaction.length    = static_cast<size_t>(frame->total_length) * 8;
    write_transaction.tx_buffer = frame->buffer;

    esp_err_t err = spi_device_queue_trans(spi_dev, &write_transaction, 0);
    if (err != ESP_OK) {
        iso15118.trace("QCA700x: Failed to queue burst write of %u bytes: %s", frame->total_length, esp_err_to_name(err));
        return;
    }

    write_in_flight = true;
}

// Picks up a completed asynchronous burst write and frees the sent frame.
// With wait == false it returns false if the transfer is still running.
// With wait == true it blocks until the transfer completed.
// Returns true if no write is in flight.
bool QCA700x::finish_burst_write(const bool wait)
{
    if (!write_in_flight) {
        return true;
    }

    spi_transaction_t *finished = nullptr;
    esp_err_t err = spi_device_get_trans_result(spi_dev, &finished, wait ? portMAX_DELAY_nowarn : 0);
    if (err == ESP_ERR_TIMEOUT) {
        return false; // Still running
    }

    write_in_flight = false;

    TxFrame *frame = &tx_queue[tx_queue_head];

    if (err != ESP_OK) {
        iso15118.trace("QCA700x: Burst write of %u bytes failed: %s", frame->total_length, esp_err_to_name(err));
    }

    free(frame->buffer);
    frame->buffer = nullptr;
    frame->total_length = 0;
    tx_queue_head = (tx_queue_head + 1) % QCA700X_TX_QUEUE_LENGTH;
    tx_queue_count--;

    return true;
}

// Completes whichever burst transfer is in flight (at most one at a time).
bool QCA700x::finish_pending_op(const bool wait)
{
    if (read_in_flight) {
        return finish_burst_read(wait);
    }

    if (write_in_flight) {
        return finish_burst_write(wait);
    }

    return true;
}

// Starts the next asynchronous transfer.
void QCA700x::start_next_op()
{
    if (read_in_flight || write_in_flight) {
        return;
    }

    if (tx_queue_count > 0) {
        start_burst_write();
    }

    start_burst_read(); // No-op if the write was issued.
}

void QCA700x::flush_receive_buffer()
{
    uint16_t total_flushed = 0;
    uint16_t available;

    while ((available = read_register(QCA700X_SPI_REG_RDBUF_BYTE_AVA)) > 0) {
        // 0xFFFF indicates no modem connected.
        if (available == 0xFFFF) {
            break;
        }

        const uint16_t to_read = std::min(available, static_cast<uint16_t>(QCA700X_DMA_SCRATCH_SIZE));
        if (!external_read_to_scratch(to_read)) {
            break;
        }

        total_flushed = static_cast<uint16_t>(total_flushed + to_read);
    }

    if (total_flushed > 0) {
        iso15118.trace("QCA700x: Flushed %u bytes from hardware buffer", total_flushed);
    }
}

void QCA700x::flush_all_buffers()
{
    // Flush hardware receive buffer (QCA700X)
    flush_receive_buffer();

    // Flush local SPI buffer
    if (spi_buffer_length > 0) {
        iso15118.trace("QCA700x: Discarding %u bytes from local buffer", spi_buffer_length);
        spi_buffer_length = 0;
    }

    // Reset previous frame tracking
    prev_frame_length = 0;
    prev_frame_eth_type = 0;
    memset(prev_frame_header, 0, sizeof(prev_frame_header));
    prev_total_frame_length = 0;
    prev_remaining_length = 0;
    prev_buf_len_before = 0;
}

int16_t QCA700x::find_sof_marker(const uint8_t *data, uint16_t length)
{
    // SOF marker is at offset 4 in the packet (after 4-byte length field)
    // Need at least 8 bytes to check: 4 bytes length + 4 bytes SOF
    if (length < 8) {
        return -1;
    }

    // Start at offset 1 to skip current (corrupted) packet start
    for (uint16_t i = 1; i <= length - 8; i++) {
        if (data[i+4] == 0xAA && data[i+5] == 0xAA &&
            data[i+6] == 0xAA && data[i+7] == 0xAA) {
            return static_cast<int16_t>(i);
        }
    }
    return -1;  // Not found
}

void QCA700x::write_burst(const uint8_t *data, const uint16_t length)
{
    if (spi_dev == nullptr) {
        return;
    }

    constexpr uint16_t max_payload = QCA700X_DMA_SCRATCH_SIZE - QCA700X_SEND_HEADER_SIZE - QCA700X_SEND_FOOTER_SIZE;
    if (length > max_payload) {
        iso15118.trace("QCA700x: write_burst frame too long: %u > %u", length, max_payload);
        return;
    }

    if (tx_queue_count >= QCA700X_TX_QUEUE_LENGTH) {
        iso15118.trace("QCA700x: TX queue full, dropping frame of %u bytes", length);
        return;
    }

    const uint16_t total_length = static_cast<uint16_t>(length + QCA700X_SEND_HEADER_SIZE + QCA700X_SEND_FOOTER_SIZE);

    // Build [header | payload | footer] contiguously in a DMA-capable
    // buffer, so everything goes out in a single transaction with CS
    // continuously asserted, as the QCA700x requires. The buffer is freed
    // by finish_burst_write() once the frame was sent.
    uint8_t *buffer = static_cast<uint8_t *>(heap_caps_malloc(total_length, MALLOC_CAP_DMA));
    if (buffer == nullptr) {
        iso15118.trace("QCA700x: Failed to allocate TX frame of %u bytes", total_length);
        return;
    }

    buffer[0] = 0xAA;
    buffer[1] = 0xAA;
    buffer[2] = 0xAA;
    buffer[3] = 0xAA;
    buffer[4] = static_cast<uint8_t>( length       & 0xFF);
    buffer[5] = static_cast<uint8_t>((length >> 8) & 0xFF);
    buffer[6] = 0;
    buffer[7] = 0;
    memcpy(&buffer[QCA700X_SEND_HEADER_SIZE], data, length);
    buffer[QCA700X_SEND_HEADER_SIZE + length]     = 0x55;
    buffer[QCA700X_SEND_HEADER_SIZE + length + 1] = 0x55;

    TxFrame *frame = &tx_queue[(tx_queue_head + tx_queue_count) % QCA700X_TX_QUEUE_LENGTH];
    frame->buffer = buffer;
    frame->total_length = total_length;
    tx_queue_count++;

    iso15118.trace_packet(data, length);

    // Issue immediately when possible to keep TX latency low.
    if (!write_in_flight) {
        finish_burst_read(true);
        start_burst_write();
    }
}

// Checks frame validity and returns ethernet frame length if valid.
// Also stores the total frame length (for removal from buffer) via out parameter.
// Returns negative number on error.
int16_t QCA700x::check_receive_frame(const uint8_t *data, const uint16_t length, uint16_t *total_frame_length_out)
{
    // Need at least 12 bytes to read packet_length, SOF, and ethernet_frame_length fields
    if (length < QCA700X_RECV_HEADER_SIZE) {
        return -4;  // Partial frame, we need more data
    }

    // Check packet length (this is the authoritative length from QCA700X hardware)
    uint32_t packet_length = static_cast<uint32_t>(data[3]) | (static_cast<uint32_t>(data[2]) << 8) | (static_cast<uint32_t>(data[1]) << 16) | (static_cast<uint32_t>(data[0]) << 24);
    if (packet_length > QCA700X_BUFFER_SIZE) {
        // Log diagnostic info for SPI corruption debugging
        const uint16_t rdbuf_byte_ava = read_register(QCA700X_SPI_REG_RDBUF_BYTE_AVA);
        iso15118.trace("QCA700x: Packet length too long: %lu > %d (RDBUF_BYTE_AVA=%u, buf_len=%u)",
                        packet_length, QCA700X_BUFFER_SIZE, rdbuf_byte_ava, length);
        iso15118.trace("QCA700x: First 16 bytes: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
                        data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7],
                        data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15]);
        return -1;
    }

    if (packet_length < (QCA700X_RECV_BUFFER_MIN_SIZE - QCA700X_HW_PKT_SIZE)) {
        iso15118.trace("QCA700x: Packet length too short: %lu < %d", packet_length, QCA700X_RECV_BUFFER_MIN_SIZE);
        return -2;
    }

    // Check SOF (start of frame)
    if ((data[4] != 0xAA) || (data[5] != 0xAA) || (data[6] != 0xAA) || (data[7] != 0xAA)) {
        iso15118.trace("QCA700x: Header mismatch");
        return -3;
    }

    // Total frame size for buffer removal:
    // The QCA700X packet format has EOF at position (packet_length + 2) and (packet_length + 3)
    // So total bytes = packet_length + 4 = packet_length + QCA700X_HW_PKT_SIZE
    const uint16_t total_frame_length = static_cast<uint16_t>(packet_length + QCA700X_HW_PKT_SIZE);

    // Check if we have enough data for the complete frame including EOF
    if (length < total_frame_length) {
        // Partial frame: Wait for more data
        return -4;
    }

    // We have enough data: Check EOF (end of frame)
    // EOF is at packet_length + 2 and packet_length + 3 (i.e., total_frame_length - 2 and - 1)
    if ((data[total_frame_length - 2] != 0x55) || (data[total_frame_length - 1] != 0x55)) {
        iso15118.trace("QCA700x: Footer mismatch at offset %d (found 0x%02x 0x%02x)",
                        total_frame_length - 2, data[total_frame_length - 2], data[total_frame_length - 1]);
        return -5;
    }

    const uint32_t ethernet_frame_length = static_cast<uint32_t>(data[8]) | (static_cast<uint32_t>(data[9]) << 8);
    if (ethernet_frame_length < QCA700X_ETHERNET_FRAME_MIN_SIZE) {
        iso15118.trace("QCA700x: Ethernet frame length too short: %lu < %d", ethernet_frame_length, QCA700X_ETHERNET_FRAME_MIN_SIZE);
        return -6;
    }

    if (ethernet_frame_length > QCA700X_BUFFER_SIZE) {
        iso15118.trace("QCA700x: Ethernet frame length too long: %lu > %d", ethernet_frame_length, QCA700X_BUFFER_SIZE);
        return -7;
    }

    // Verify consistency: packet_length should equal ethernet_frame_length + 10
    // (SOF:4 + eth_len:2 + reserved:2 + ethernet_frame_length + EOF:2 = eth_frame + 10)
    const uint32_t expected_packet_length = ethernet_frame_length + 10;
    if (packet_length != expected_packet_length) {
        iso15118.trace("QCA700x: Length mismatch! packet_length=%lu, expected=%lu (eth_frame_len=%lu)",
                        packet_length, expected_packet_length, ethernet_frame_length);
        // We use the packet length here (hardware value) as authoritative and accept it,
        // but still log this for debugging for now.
    }

    // Return total frame length via out parameter
    if (total_frame_length_out != nullptr) {
        *total_frame_length_out = total_frame_length;
    }

    return static_cast<int16_t>(ethernet_frame_length);
}


void QCA700x::setup_l2tap()
{
    esp_err_t err = esp_vfs_l2tap_intf_register(NULL);
    if (err != ESP_OK) {
        iso15118.trace("Failed to register l2tap VFS: %s", esp_err_to_name(err));
        return;
    }

    tap = open("/dev/net/tap", O_NONBLOCK);
    if (tap < 0) {
        iso15118.trace("Failed to open /dev/net/tap: errno %d", errno);
        return;
    }

    // Bind l2tap to our custom netif using IO driver handle
    int ret = ioctl(tap, L2TAP_S_DEVICE_DRV_HNDL, &driver);
    if (ret < 0) {
        iso15118.trace("Failed to bind l2tap to device driver handle: errno %d", errno);
        close(tap);
        tap = -1;
        iso15118.set_poll_fd(FDS_TAP_INDEX, -1);
        return;
    }

    // Filter only HomePlug frames (0x88E1)
    uint16_t eth_type_filter_homeplug = SLAC_ETHERNET_TYPE_HOMEPLUG;
    ret = ioctl(tap, L2TAP_S_RCV_FILTER, &eth_type_filter_homeplug);
    if (ret < 0) {
        iso15118.trace("Failed to set l2tap receive filter: errno %d", errno);
        close(tap);
        tap = -1;
        iso15118.set_poll_fd(FDS_TAP_INDEX, -1);
        return;
    }

    // Register tap in central poll array
    iso15118.set_poll_fd(FDS_TAP_INDEX, tap);

    iso15118.trace("l2tap initialized successfully (fd=%d, filter=0x%04X)", tap, eth_type_filter_homeplug);
}

void QCA700x::setup_netif()
{
    // Check if netif already setup
    if (netif != nullptr) {
        return;
    }

    // Base config is default eth base config, with different key and description
    esp_netif_inherent_config_t base_config = {
        .flags = static_cast<esp_netif_flags_t>(ESP_NETIF_FLAG_GARP | ESP_NETIF_FLAG_EVENT_IP_MODIFIED | ESP_NETIF_FLAG_AUTOUP),
        ESP_COMPILER_DESIGNATED_INIT_AGGREGATE_TYPE_EMPTY(mac)
        ESP_COMPILER_DESIGNATED_INIT_AGGREGATE_TYPE_EMPTY(ip_info)
        .get_ip_event = IP_EVENT_ETH_GOT_IP,
        .lost_ip_event = IP_EVENT_ETH_LOST_IP,
        .if_key = "ETH_QCA",
        .if_desc = "qca",
        .route_prio = 50,
        .bridge_info = nullptr
    };

    esp_netif_driver_ifconfig_t driver_config = {
        .handle = static_cast<esp_netif_iodriver_handle>(this),
        .transmit = [](void *h, void *buf, size_t len) -> esp_err_t {
            void *data_copy = malloc(len);
            if (data_copy == NULL) {
                iso15118.trace("QCA700x: Failed to allocate memory for transmit data");
                return ESP_OK;
            }
            memcpy(data_copy, buf, len);
            task_scheduler.scheduleOnce([data_copy, len]() {
                iso15118.qca700x.write_burst(static_cast<const uint8_t *>(data_copy), static_cast<uint16_t>(len));
                free(data_copy);
            }, 0_ms);
            return ESP_OK;
        },
        .transmit_wrap = [](void *h, void *buf, size_t len, void *netstack_buf) -> esp_err_t {
            void *data_copy = malloc(len);
            if (data_copy == NULL) {
                iso15118.trace("QCA700x: Failed to allocate memory for transmit data");
                return ESP_OK;
            }
            memcpy(data_copy, buf, len);
            task_scheduler.scheduleOnce([data_copy, len]() {
                iso15118.qca700x.write_burst(static_cast<const uint8_t *>(data_copy), static_cast<uint16_t>(len));
                free(data_copy);
            }, 0_ms);
            return ESP_OK;
        },
        .driver_free_rx_buffer = [](void *h, void *buf) {
            free(buf);
        },
        .driver_set_mac_filter = [](void *h, const uint8_t *mac_addr, size_t mac_len, bool add) {
            char mac_str[40];
            StringWriter sw(mac_str, std::size(mac_str));

            for (size_t i = 0; i < mac_len; ++i) {
                sw.printf(":%02x", mac_addr[i]);
            }
            mac_str[0] = ' '; // Replace leading colon.

            return ESP_OK;
        }
    };

    esp_netif_config_t netif_config = {
        .base = &base_config,
        .driver = &driver_config,
        .stack = ESP_NETIF_NETSTACK_DEFAULT_ETH,
    };

    netif = esp_netif_new(&netif_config);
    if (netif == NULL) {
        iso15118.trace("Failed to create QCA700x netif");
        return;
    }

    // Derive MAC address for QCA700x from ethernet MAC
    uint8_t eth_mac[QCA700X_MAC_SIZE];
    esp_read_mac(eth_mac, ESP_MAC_ETH);
    esp_derive_local_mac(mac, eth_mac);
    ESP_ERROR_CHECK(esp_netif_set_mac(netif, mac) != ESP_OK);

    //esp_eth_netif_glue_handle_t netif_glue = esp_eth_new_netif_glue(handle);
    driver.qca700x = this;
    ESP_ERROR_CHECK(esp_netif_attach(netif, &driver));

    // Set up l2tap for HomePlug frame access
    setup_l2tap();
}

void QCA700x::link_up()
{
    if (netif == nullptr) {
        iso15118.trace("QCA700x: link_up called but netif is not initialized");
        return;
    }

    if (!esp_netif_is_netif_up(netif)) {
        esp_netif_action_start(netif, nullptr, 0, nullptr);
        ESP_ERROR_CHECK(esp_netif_create_ip6_linklocal(netif));
    } else {
        iso15118.trace("QCA700x netif already up");
    }

    if (!esp_netif_is_netif_up(netif)) {
        iso15118.trace("Failed to start QCA700x netif");
    }
}

void QCA700x::link_down()
{
    if (netif == nullptr) {
        iso15118.trace("QCA700x: link_down called but netif is not initialized");
        return;
    }

    if (esp_netif_is_netif_up(netif)) {
        esp_netif_action_stop(netif, nullptr, 0, nullptr);
    }
}

void QCA700x::received_data_to_netif(const uint8_t *data, const uint16_t length)
{
    if (netif == NULL) {
        // netif is only set up once modem is found.
        // The received_data_to_netif can be called before that.
        // This is expected behavior, we just return in that case.
        return;
    }

    // ESP netif expects a newly malloced buffer that it can use as scratch buffer internally.
    // The buffer will be freed by driver_free_rx_buffer function.
    void *data_copy = malloc(length);
    if (data_copy == NULL) {
        iso15118.trace("QCA700x: Failed to allocate memory for received data");
        return;
    }
    memcpy(data_copy, data, length);

    esp_netif_receive(netif, data_copy, length, nullptr);
}

bool QCA700x::get_ip6_linklocal(esp_ip6_addr_t *if_ip6)
{
    // Always initialize output to zero
    memset(if_ip6, 0, sizeof(esp_ip6_addr_t));

    // In debug mode, use the default Ethernet interface
    if (iso15118.debug_mode.is_enabled()) {
        esp_netif_t *eth_netif = esp_netif_get_handle_from_ifkey("ETH_DEF");
        if (eth_netif != NULL) {
            esp_err_t err = esp_netif_get_ip6_linklocal(eth_netif, if_ip6);
            if (err == ESP_OK) {
                return true;
            }
            iso15118.trace("get_ip6_linklocal: esp_netif_get_ip6_linklocal failed: %s", esp_err_to_name(err));
            return false;
        }
        iso15118.trace("get_ip6_linklocal: Debug mode but ETH_DEF not found");
        return false;
    }

    if (netif == NULL) {
        iso15118.trace("get_ip6_linklocal: netif not setup");
        return false;
    }

    esp_err_t err = esp_netif_get_ip6_linklocal(netif, if_ip6);
    if (err != ESP_OK) {
        iso15118.trace("get_ip6_linklocal: esp_netif_get_ip6_linklocal failed: %s", esp_err_to_name(err));
        return false;
    }
    return true;
}

void QCA700x::state_machine_loop()
{
    // Initialize SPI on first use
    if (!spi_initialized) {
        if (!spi_init()) {
            return;
        }
        spi_initialized = true;
    }

    // Allocate SPI buffer on first use
    if (spi_buffer == nullptr) {
        spi_buffer = static_cast<uint8_t *>(calloc_psram_or_dram(QCA700X_BUFFER_SIZE + QCA700X_HW_PKT_SIZE + 1, sizeof(uint8_t)));
        if (spi_buffer == nullptr) {
            iso15118.trace("QCA700x: Failed to allocate SPI buffer");
            return;
        }
    }

    // Pick up a completed burst transfer.
    // If there is nothing to do the main loop stays free and we check again on the next tick.
    if (!finish_pending_op(false)) {
        return;
    }

    // Only process frames if modem has been detected via signature check
    // This prevents processing garbage data when no modem is connected
    if (!modem_detected) {
        spi_buffer_length = 0;
        start_next_op(); // Keep sending and draining the modem's hardware buffer.
        return;
    }

    // Process all complete frames in the buffer
    while (spi_buffer_length >= QCA700X_RECV_BUFFER_MIN_SIZE) {
        // Get both ethernet frame length and total frame length from check_receive_frame
        // total_frame_length is calculated from packet_length
        uint16_t total_frame_length = 0;
        int16_t ethernet_frame_length = check_receive_frame(spi_buffer, spi_buffer_length, &total_frame_length);

        // Error -4 means partial frame: Wait for more data
        if (ethernet_frame_length == -4) {
            break;
        }

        if (ethernet_frame_length < 0) {
            // Log detailed diagnostics including previous frame info
            iso15118.trace("QCA700x: Frame error %d, buf_len=%u, attempting recovery...",
                            ethernet_frame_length, spi_buffer_length);
            iso15118.trace("QCA700x: Previous frame: len=%u, eth_type=0x%04X",
                            prev_frame_length, prev_frame_eth_type);
            iso15118.trace("QCA700x: Previous buffer state: buf_before=%u, total_frame=%u, remaining=%ld",
                            prev_buf_len_before, prev_total_frame_length, static_cast<long>(prev_remaining_length));
            iso15118.trace("QCA700x: Previous frame header: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
                            prev_frame_header[0], prev_frame_header[1], prev_frame_header[2], prev_frame_header[3],
                            prev_frame_header[4], prev_frame_header[5], prev_frame_header[6], prev_frame_header[7],
                            prev_frame_header[8], prev_frame_header[9], prev_frame_header[10], prev_frame_header[11],
                            prev_frame_header[12], prev_frame_header[13], prev_frame_header[14], prev_frame_header[15]);
            iso15118.trace("QCA700x: Previous frame header: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
                            prev_frame_header[16], prev_frame_header[17], prev_frame_header[18], prev_frame_header[19],
                            prev_frame_header[20], prev_frame_header[21], prev_frame_header[22], prev_frame_header[23],
                            prev_frame_header[24], prev_frame_header[25], prev_frame_header[26], prev_frame_header[27],
                            prev_frame_header[28], prev_frame_header[29], prev_frame_header[30], prev_frame_header[31]);

            // Try to find next SOF marker in buffer
            int16_t sof_offset = find_sof_marker(spi_buffer, spi_buffer_length);

            if (sof_offset > 0) {
                // Found SOF - shift buffer and continue
                iso15118.trace("QCA700x: Found SOF at offset %d, skipping %d corrupted bytes",
                                sof_offset, sof_offset);
                memmove(spi_buffer, spi_buffer + sof_offset, static_cast<size_t>(spi_buffer_length - static_cast<uint16_t>(sof_offset)));
                spi_buffer_length -= static_cast<uint16_t>(sof_offset);
                // Continue loop to try parsing from new position
                continue;
            } else {
                // No SOF found - flush hardware buffer and discard local buffer
                iso15118.trace("QCA700x: No SOF found in %u bytes, flushing buffers", spi_buffer_length);
                flush_receive_buffer();
                spi_buffer_length = 0;
                break;
            }
        }

        // Frame starts after QCA700x header
        uint8_t *frame = spi_buffer + QCA700X_RECV_HEADER_SIZE;

        // Get EtherType (bytes 12-13 of Ethernet frame, big-endian)
        uint16_t eth_type = static_cast<uint16_t>((frame[12] << 8) | frame[13]);

        switch (eth_type) {
            case SLAC_ETHERNET_TYPE_HOMEPLUG: {
                // HomePlug frames go to l2tap for SLAC to read
                if (tap >= 0) {
                    // L2TAP expects a heap-allocated buffer that it will free after read()
                    void *frame_copy = malloc(static_cast<size_t>(ethernet_frame_length));
                    if (frame_copy != NULL) {
                        memcpy(frame_copy, frame, static_cast<size_t>(ethernet_frame_length));
                        size_t size = static_cast<size_t>(ethernet_frame_length);
                        esp_vfs_l2tap_eth_filter_frame(&driver, frame_copy, &size, nullptr);
                    } else {
                        iso15118.trace("QCA700x: Failed to allocate memory for HomePlug frame");
                    }
                }
                break;
            }

            case SLAC_ETHERNET_TYPE_IPV6: {
                // IPv6 frames go directly to netif/lwIP
                // Set flag so SLAC can transition from WaitForSDP to LinkDetected
                ipv6_packet_received = true;
                received_data_to_netif(frame, static_cast<uint16_t>(ethernet_frame_length));
                break;
            }

            case SLAC_ETHERNET_TYPE_IPV4: {
                iso15118.trace("QCA700x: Received IPv4 packet (len=%d), not supported in ISO15118", ethernet_frame_length);
                break;
            }

            default: {
                iso15118.trace("QCA700x: Unknown EtherType: 0x%04X", eth_type);
                break;
            }
        }

        // Save frame info for debugging (in case next frame is corrupted)
        prev_frame_length = static_cast<uint16_t>(ethernet_frame_length);
        prev_frame_eth_type = eth_type;
        memcpy(prev_frame_header, frame, std::min(static_cast<size_t>(ethernet_frame_length), static_cast<size_t>(32)));

        // Remove processed frame from buffer using total_frame_length from check_receive_frame
        // (calculated from hardware packet_length, not derived from ethernet_frame_length)
        const int32_t remaining_length = spi_buffer_length - total_frame_length;

        // Save buffer state for debugging (in case next frame is corrupted)
        prev_buf_len_before = spi_buffer_length;
        prev_total_frame_length = total_frame_length;
        prev_remaining_length = remaining_length;

        // This should never happen if check_receive_frame() works correctly
        if (remaining_length < 0) {
            iso15118.trace("QCA700x: CRITICAL BUG - negative remaining! buf_len=%u, total_frame=%u, remaining=%ld",
                            spi_buffer_length, total_frame_length, static_cast<long>(remaining_length));
            iso15118.trace("QCA700x: This indicates check_receive_frame() approved a frame larger than buffer!");
            // Recovery: discard everything and start fresh
            flush_receive_buffer();
            spi_buffer_length = 0;
            break;
        }

        if (remaining_length > 0) {
            // Move remaining data to start of buffer
            memmove(spi_buffer, spi_buffer + total_frame_length, static_cast<size_t>(remaining_length));
            spi_buffer_length = static_cast<uint16_t>(remaining_length);
        } else {
            spi_buffer_length = 0;
        }
    }

    // Kick off the next asynchronous transfer.
    start_next_op();
}
