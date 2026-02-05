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
#include "module_dependencies.h"
#include "build.h"

#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include "esp_vfs_l2tap.h"
#include "esp_netif.h"
#include "esp_mac.h"

#include "slac.h"

#include "lwip/ip_addr.h"
#include "bindings/hal_common.h"

#include "tools/string_builder.h"

#define QCA700X_SPI_CHIP_SELECT_PIN 4
#define QCA700X_SPI_MISO_PIN 39 // SENSOR_VN
#define QCA700X_SPI_MOSI_PIN 15
#define QCA700X_SPI_CLOCK_PIN 2

extern TF_HAL hal;

void QCA700x::spi_transceive(const uint8_t *write_buffer, uint8_t *read_buffer, const uint32_t length) {
    memcpy(read_buffer, write_buffer, length);
    vspi->transfer(read_buffer, length);
}

void QCA700x::spi_select()
{
    vspi->beginTransaction(spi_settings);
    REG_WRITE(GPIO_OUT_W1TC_REG, 1 << QCA700X_SPI_CHIP_SELECT_PIN);
}

void QCA700x::spi_deselect()
{
    REG_WRITE(GPIO_OUT_W1TS_REG, 1 << QCA700X_SPI_CHIP_SELECT_PIN);
    vspi->endTransaction();
}

void QCA700x::spi_write(const uint8_t *data, const uint16_t length)
{
    uint8_t tmp_read_buf[length] = {0};
    spi_transceive(data, tmp_read_buf, length);
}

void QCA700x::spi_read(uint8_t *data, const uint32_t length)
{
    const uint8_t tmp_write_buf[length] = {0};
    spi_transceive(tmp_write_buf, data, length);
}

void QCA700x::spi_write_16bit_value(const uint16_t value)
{
    const uint8_t value_be[2] = {
        (uint8_t)(value >> 8),
        (uint8_t)(value & 0xFF)
    };

    spi_write(value_be, 2);
}

uint16_t QCA700x::spi_read_16bit_value()
{
    uint8_t value[2] = {0, 0};
    spi_read(value, 2);

    return (value[0] << 8) | value[1];
}

void QCA700x::spi_write_header(const uint16_t length)
{
    const uint8_t data[QCA700X_SEND_HEADER_SIZE] = {
        0xAA, 0xAA, 0xAA, 0xAA,
        (uint8_t)( length       & 0xFF),
        (uint8_t)((length >> 8) & 0xFF),
        0, 0
    };
    spi_write(data, 8);
}

void QCA700x::spi_write_footer()
{
    const uint8_t data[QCA700X_SEND_FOOTER_SIZE] = {0x55, 0x55};
    spi_write(data, 2);
}

void QCA700x::spi_init()
{
    pinMode(QCA700X_SPI_CHIP_SELECT_PIN, OUTPUT);
    REG_WRITE(GPIO_OUT_W1TS_REG, 1 << QCA700X_SPI_CHIP_SELECT_PIN);

    spi_settings = SPISettings(1400000, SPI_MSBFIRST, SPI_MODE3);
    vspi = new SPIClass(VSPI);
    vspi->begin(QCA700X_SPI_CLOCK_PIN, QCA700X_SPI_MISO_PIN, QCA700X_SPI_MOSI_PIN);
}

uint16_t QCA700x::read_register(const uint16_t reg)
{
    spi_select();
    spi_write_16bit_value(QCA700X_SPI_READ | QCA700X_SPI_INTERNAL | reg);
    const uint16_t value = spi_read_16bit_value();
    spi_deselect();

    return value;
}

void QCA700x::write_register(const uint16_t reg, const uint16_t value)
{
    spi_select();
    spi_write_16bit_value(QCA700X_SPI_WRITE | QCA700X_SPI_INTERNAL | reg);
    spi_write_16bit_value(value);
    spi_deselect();
}

uint16_t QCA700x::read_burst(uint8_t *data, const uint16_t length)
{
    const uint16_t available = std::min(length, read_register(QCA700X_SPI_REG_RDBUF_BYTE_AVA));
    if (available == 0) {
        return 0;
    }

    if (available <= length) {
        write_register(QCA700X_SPI_REG_BFR_SIZE, available);

        spi_select();
        spi_write_16bit_value(QCA700X_SPI_READ | QCA700X_SPI_EXTERNAL);
        spi_read(data, available);
        spi_deselect();

        iso15118.trace_packet(data, available);
        return available;
    } else {
        logger.printfln("read_burst error: available %u > max length %u", available, length);
    }

    return 0;
}

void QCA700x::flush_receive_buffer()
{
    uint16_t total_flushed = 0;
    uint16_t available;
    uint8_t discard[256];

    while ((available = read_register(QCA700X_SPI_REG_RDBUF_BYTE_AVA)) > 0) {
        uint16_t to_read = std::min(available, (uint16_t)sizeof(discard));
        write_register(QCA700X_SPI_REG_BFR_SIZE, to_read);
        spi_select();
        spi_write_16bit_value(QCA700X_SPI_READ | QCA700X_SPI_EXTERNAL);
        spi_read(discard, to_read);
        spi_deselect();
        total_flushed += to_read;
    }

    if (total_flushed > 0) {
        logger.printfln("QCA700x: Flushed %u bytes from hardware buffer", total_flushed);
    }
}

void QCA700x::flush_all_buffers()
{
    // Flush hardware receive buffer (QCA700X)
    flush_receive_buffer();

    // Flush local SPI buffer
    if (spi_buffer_length > 0) {
        logger.printfln("QCA700x: Discarding %u bytes from local buffer", spi_buffer_length);
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
            return i;
        }
    }
    return -1;  // Not found
}

void QCA700x::write_burst(const uint8_t *data, const uint16_t length)
{
    write_register(QCA700X_SPI_REG_BFR_SIZE, length + QCA700X_SEND_HEADER_SIZE + QCA700X_SEND_FOOTER_SIZE);

    spi_select();
    spi_write_16bit_value(QCA700X_SPI_WRITE | QCA700X_SPI_EXTERNAL);
    spi_write_header(length);
    spi_write(data, length);
    spi_write_footer();
    spi_deselect();

    iso15118.trace_packet(data, length);
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
    uint32_t packet_length = data[3] | (data[2] << 8) | (data[1] << 16) | (data[0] << 24);
    if (packet_length > QCA700X_BUFFER_SIZE) {
        // Log diagnostic info for SPI corruption debugging
        const uint16_t rdbuf_byte_ava = read_register(QCA700X_SPI_REG_RDBUF_BYTE_AVA);
        logger.printfln("QCA700x: Packet length too long: %lu > %u (RDBUF_BYTE_AVA=%u, buf_len=%u)",
                        packet_length, QCA700X_BUFFER_SIZE, rdbuf_byte_ava, length);
        logger.printfln("QCA700x: First 16 bytes: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
                        data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7],
                        data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15]);
        return -1;
    }

    if (packet_length < (QCA700X_RECV_BUFFER_MIN_SIZE - QCA700X_HW_PKT_SIZE)) {
        logger.printfln("QCA700x: Packet length too short: %lu < %u", packet_length, QCA700X_RECV_BUFFER_MIN_SIZE);
        return -2;
    }

    // Check SOF (start of frame)
    if ((data[4] != 0xAA) || (data[5] != 0xAA) || (data[6] != 0xAA) || (data[7] != 0xAA)) {
        logger.printfln("QCA700x: Header mismatch");
        return -3;
    }

    // Total frame size for buffer removal:
    // The QCA700X packet format has EOF at position (packet_length + 2) and (packet_length + 3)
    // So total bytes = packet_length + 4 = packet_length + QCA700X_HW_PKT_SIZE
    const uint16_t total_frame_length = packet_length + QCA700X_HW_PKT_SIZE;

    // Check if we have enough data for the complete frame including EOF
    if (length < total_frame_length) {
        // Partial frame: Wait for more data
        return -4;
    }

    // We  have enough data: Check EOF (end of frame)
    // EOF is at packet_length + 2 and packet_length + 3 (i.e., total_frame_length - 2 and - 1)
    if ((data[total_frame_length - 2] != 0x55) || (data[total_frame_length - 1] != 0x55)) {
        logger.printfln("QCA700x: Footer mismatch at offset %u (found 0x%02x 0x%02x)",
                        total_frame_length - 2, data[total_frame_length - 2], data[total_frame_length - 1]);
        return -5;
    }

    const uint32_t ethernet_frame_length = data[8] | (data[9] << 8);
    if (ethernet_frame_length < QCA700X_ETHERNET_FRAME_MIN_SIZE) {
        logger.printfln("QCA700x: Ethernet frame length too short: %lu < %u", ethernet_frame_length, QCA700X_ETHERNET_FRAME_MIN_SIZE);
        return -6;
    }

    if (ethernet_frame_length > QCA700X_BUFFER_SIZE) {
        logger.printfln("QCA700x: Ethernet frame length too long: %lu > %u", ethernet_frame_length, QCA700X_BUFFER_SIZE);
        return -7;
    }

    // Verify consistency: packet_length should equal ethernet_frame_length + 10
    // (SOF:4 + eth_len:2 + reserved:2 + ethernet_frame_length + EOF:2 = eth_frame + 10)
    const uint32_t expected_packet_length = ethernet_frame_length + 10;
    if (packet_length != expected_packet_length) {
        logger.printfln("QCA700x: Length mismatch! packet_length=%lu, expected=%lu (eth_frame_len=%lu)",
                        packet_length, expected_packet_length, ethernet_frame_length);
        // We use the packet length here (hardware value) as authoritative and accept it,
        // but still log this for debugging for now.
    }

    // Return total frame length via out parameter
    if (total_frame_length_out != nullptr) {
        *total_frame_length_out = total_frame_length;
    }

    return ethernet_frame_length;
}


void QCA700x::setup_l2tap()
{
    esp_err_t err = esp_vfs_l2tap_intf_register(NULL);
    if (err != ESP_OK) {
        logger.printfln("Failed to register l2tap VFS: %s", esp_err_to_name(err));
        return;
    }

    tap = open("/dev/net/tap", O_NONBLOCK);
    if (tap < 0) {
        logger.printfln("Failed to open /dev/net/tap: errno %d", errno);
        return;
    }

    // Bind l2tap to our custom netif using IO driver handle
    int ret = ioctl(tap, L2TAP_S_DEVICE_DRV_HNDL, &driver);
    if (ret < 0) {
        logger.printfln("Failed to bind l2tap to device driver handle: errno %d", errno);
        close(tap);
        tap = -1;
        iso15118.set_poll_fd(FDS_TAP_INDEX, -1);
        return;
    }

    // Filter only HomePlug frames (0x88E1)
    uint16_t eth_type_filter_homeplug = SLAC_ETHERNET_TYPE_HOMEPLUG;
    ret = ioctl(tap, L2TAP_S_RCV_FILTER, &eth_type_filter_homeplug);
    if (ret < 0) {
        logger.printfln("Failed to set l2tap receive filter: errno %d", errno);
        close(tap);
        tap = -1;
        iso15118.set_poll_fd(FDS_TAP_INDEX, -1);
        return;
    }

    // Register tap in central poll array
    iso15118.set_poll_fd(FDS_TAP_INDEX, tap);

    logger.printfln("l2tap initialized successfully (fd=%d, filter=0x%04X)", tap, eth_type_filter_homeplug);
}

void QCA700x::setup_netif()
{
    // Check if netif already setup
    if (netif != nullptr) {
        return;
    }

    // Base config is default eth base config, with different key and description
    esp_netif_inherent_config_t base_config = {
        .flags = (esp_netif_flags_t)(ESP_NETIF_FLAG_GARP | ESP_NETIF_FLAG_EVENT_IP_MODIFIED | ESP_NETIF_FLAG_AUTOUP),
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
        .transmit = [](void *handle, void *buffer, size_t length) -> esp_err_t {
            void *data_copy = malloc(length);
            if (data_copy == NULL) {
                logger.printfln("QCA700x: Failed to allocate memory for transmit data");
                return ESP_OK;
            }
            memcpy(data_copy, buffer, length);
            task_scheduler.scheduleOnce([data_copy, length]() {
                iso15118.qca700x.write_burst(static_cast<const uint8_t *>(data_copy), length);
                free(data_copy);
            }, 0_ms);
            return ESP_OK;
        },
        .transmit_wrap = [](void *handle, void *buffer, size_t length, void *netstack_buffer) -> esp_err_t {
            void *data_copy = malloc(length);
            if (data_copy == NULL) {
                logger.printfln("QCA700x: Failed to allocate memory for transmit data");
                return ESP_OK;
            }
            memcpy(data_copy, buffer, length);
            task_scheduler.scheduleOnce([data_copy, length]() {
                iso15118.qca700x.write_burst(static_cast<const uint8_t *>(data_copy), length);
                free(data_copy);
            }, 0_ms);
            return ESP_OK;
        },
        .driver_free_rx_buffer = [](void *handle, void *buffer) {
            free(buffer);
        },
        .driver_set_mac_filter = [](void *handle, const uint8_t *mac, size_t mac_len, bool add) {
            char mac_str[40];
            StringWriter sw(mac_str, std::size(mac_str));

            for (size_t i = 0; i < mac_len; ++i) {
                sw.printf(":%02x", mac[i]);
            }
            mac_str[0] = ' '; // Replace leading colon.

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
        logger.printfln("Failed to create QCA700x netif");
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
        logger.printfln("QCA700x: link_up called but netif is not initialized");
        return;
    }

    if (!esp_netif_is_netif_up(netif)) {
        esp_netif_action_start(netif, 0, 0, nullptr);
        ESP_ERROR_CHECK(esp_netif_create_ip6_linklocal(netif));
    } else {
        logger.printfln("QCA700x netif already up");
    }

    if (!esp_netif_is_netif_up(netif)) {
        logger.printfln("Failed to start QCA700x netif");
    }
}

void QCA700x::link_down()
{
    if (netif == nullptr) {
        logger.printfln("QCA700x: link_up called but netif is not initialized");
        return;
    }

    if (esp_netif_is_netif_up(netif)) {
        esp_netif_action_stop(netif, 0, 0, nullptr);
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
        logger.printfln("QCA700x: Failed to allocate memory for received data");
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
    if (iso15118.debug_mode) {
        esp_netif_t *eth_netif = esp_netif_get_handle_from_ifkey("ETH_DEF");
        if (eth_netif != NULL) {
            esp_err_t err = esp_netif_get_ip6_linklocal(eth_netif, if_ip6);
            if (err == ESP_OK) {
                return true;
            }
            logger.printfln("get_ip6_linklocal: esp_netif_get_ip6_linklocal failed: %s", esp_err_to_name(err));
            return false;
        }
        logger.printfln("get_ip6_linklocal: Debug mode but ETH_DEF not found");
        return false;
    }

    if (netif == NULL) {
        logger.printfln("get_ip6_linklocal: netif not setup");
        return false;
    }

    esp_err_t err = esp_netif_get_ip6_linklocal(netif, if_ip6);
    if (err != ESP_OK) {
        logger.printfln("get_ip6_linklocal: esp_netif_get_ip6_linklocal failed: %s", esp_err_to_name(err));
        return false;
    }
    return true;
}

void QCA700x::state_machine_loop()
{
    // Initialize SPI on first use
    if (!spi_initialized) {
        spi_init();
        spi_initialized = true;
    }

    // Allocate SPI buffer on first use
    if (spi_buffer == nullptr) {
        spi_buffer = (uint8_t *)calloc_psram_or_dram(QCA700X_BUFFER_SIZE + QCA700X_HW_PKT_SIZE + 1, sizeof(uint8_t));
        if (spi_buffer == nullptr) {
            logger.printfln("QCA700x: Failed to allocate SPI buffer");
            return;
        }
    }

    // Poll SPI for data from QCA700x
    spi_buffer_length += read_burst(&spi_buffer[spi_buffer_length],
                                    QCA700X_BUFFER_SIZE + QCA700X_HW_PKT_SIZE - spi_buffer_length);

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
            logger.printfln("QCA700x: Frame error %d, buf_len=%u, attempting recovery...",
                            ethernet_frame_length, spi_buffer_length);
            logger.printfln("QCA700x: Previous frame: len=%u, eth_type=0x%04X",
                            prev_frame_length, prev_frame_eth_type);
            logger.printfln("QCA700x: Previous buffer state: buf_before=%u, total_frame=%u, remaining=%ld",
                            prev_buf_len_before, prev_total_frame_length, (long)prev_remaining_length);
            logger.printfln("QCA700x: Previous frame header: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
                            prev_frame_header[0], prev_frame_header[1], prev_frame_header[2], prev_frame_header[3],
                            prev_frame_header[4], prev_frame_header[5], prev_frame_header[6], prev_frame_header[7],
                            prev_frame_header[8], prev_frame_header[9], prev_frame_header[10], prev_frame_header[11],
                            prev_frame_header[12], prev_frame_header[13], prev_frame_header[14], prev_frame_header[15]);
            logger.printfln("QCA700x: Previous frame header: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
                            prev_frame_header[16], prev_frame_header[17], prev_frame_header[18], prev_frame_header[19],
                            prev_frame_header[20], prev_frame_header[21], prev_frame_header[22], prev_frame_header[23],
                            prev_frame_header[24], prev_frame_header[25], prev_frame_header[26], prev_frame_header[27],
                            prev_frame_header[28], prev_frame_header[29], prev_frame_header[30], prev_frame_header[31]);

            // Try to find next SOF marker in buffer
            int16_t sof_offset = find_sof_marker(spi_buffer, spi_buffer_length);

            if (sof_offset > 0) {
                // Found SOF - shift buffer and continue
                logger.printfln("QCA700x: Found SOF at offset %d, skipping %d corrupted bytes",
                                sof_offset, sof_offset);
                memmove(spi_buffer, spi_buffer + sof_offset, spi_buffer_length - sof_offset);
                spi_buffer_length -= sof_offset;
                // Continue loop to try parsing from new position
                continue;
            } else {
                // No SOF found - flush hardware buffer and discard local buffer
                logger.printfln("QCA700x: No SOF found in %u bytes, flushing buffers", spi_buffer_length);
                flush_receive_buffer();
                spi_buffer_length = 0;
                break;
            }
        }

        // Frame starts after QCA700x header
        uint8_t *frame = spi_buffer + QCA700X_RECV_HEADER_SIZE;

        // Get EtherType (bytes 12-13 of Ethernet frame, big-endian)
        uint16_t eth_type = (frame[12] << 8) | frame[13];

        switch (eth_type) {
            case SLAC_ETHERNET_TYPE_HOMEPLUG: {
                // HomePlug frames go to l2tap for SLAC to read
                if (tap >= 0) {
                    // L2TAP expects a heap-allocated buffer that it will free after read()
                    void *frame_copy = malloc(ethernet_frame_length);
                    if (frame_copy != NULL) {
                        memcpy(frame_copy, frame, ethernet_frame_length);
                        size_t size = ethernet_frame_length;
                        esp_vfs_l2tap_eth_filter_frame(&driver, frame_copy, &size, nullptr);
                    } else {
                        logger.printfln("QCA700x: Failed to allocate memory for HomePlug frame");
                    }
                }
                break;
            }

            case SLAC_ETHERNET_TYPE_IPV6: {
                // IPv6 frames go directly to netif/lwIP
                // Set flag so SLAC can transition from WaitForSDP to LinkDetected
                ipv6_packet_received = true;
                received_data_to_netif(frame, ethernet_frame_length);
                break;
            }

            case SLAC_ETHERNET_TYPE_IPV4: {
                logger.printfln("QCA700x: Received IPv4 packet (len=%d), not supported in ISO15118", ethernet_frame_length);
                break;
            }

            default: {
                logger.printfln("QCA700x: Unknown EtherType: 0x%04X", eth_type);
                break;
            }
        }

        // Save frame info for debugging (in case next frame is corrupted)
        prev_frame_length = ethernet_frame_length;
        prev_frame_eth_type = eth_type;
        memcpy(prev_frame_header, frame, std::min((int)ethernet_frame_length, 32));

        // Remove processed frame from buffer using total_frame_length from check_receive_frame
        // (calculated from hardware packet_length, not derived from ethernet_frame_length)
        const int32_t remaining_length = spi_buffer_length - total_frame_length;

        // Save buffer state for debugging (in case next frame is corrupted)
        prev_buf_len_before = spi_buffer_length;
        prev_total_frame_length = total_frame_length;
        prev_remaining_length = remaining_length;

        // This should never happen if check_receive_frame() works correctly
        if (remaining_length < 0) {
            logger.printfln("QCA700x: CRITICAL BUG - negative remaining! buf_len=%u, total_frame=%u, remaining=%ld",
                            spi_buffer_length, total_frame_length, (long)remaining_length);
            logger.printfln("QCA700x: This indicates check_receive_frame() approved a frame larger than buffer!");
            // Recovery: discard everything and start fresh
            flush_receive_buffer();
            spi_buffer_length = 0;
            break;
        }

        if (remaining_length > 0) {
            // Move remaining data to start of buffer
            memmove(spi_buffer, spi_buffer + total_frame_length, remaining_length);
            spi_buffer_length = remaining_length;
        } else {
            spi_buffer_length = 0;
        }
    }
}
