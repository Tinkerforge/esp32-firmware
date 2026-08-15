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

#pragma once

#include "module.h"
#include "config.h"

#include <driver/spi_master.h>

#include "esp_netif.h"
#include "esp_eth_netif_glue.h"

#define QCA700X_HAL_SPI_PORT (0 | (1 << 7))

#define QCA700X_SPI_READ     (1 << 15)
#define QCA700X_SPI_WRITE    (0 << 15)
#define QCA700X_SPI_INTERNAL (1 << 14)
#define QCA700X_SPI_EXTERNAL (0 << 14)

#define	QCA700X_SPI_REG_BFR_SIZE        0x0100
#define QCA700X_SPI_REG_WRBUF_SPC_AVA   0x0200
#define QCA700X_SPI_REG_RDBUF_BYTE_AVA  0x0300
#define QCA700X_SPI_REG_SPI_CONFIG      0x0400
#define QCA700X_SPI_REG_INTR_CAUSE      0x0C00
#define QCA700X_SPI_REG_INTR_ENABLE     0x0D00
#define QCA700X_SPI_REG_RDBUF_WATERMARK 0x1200
#define QCA700X_SPI_REG_WRBUF_WATERMARK 0x1300
#define QCA700X_SPI_REG_SIGNATURE       0x1A00
#define QCA700X_SPI_REG_ACTION_CTRL     0x1B00

#define QCA700X_SPI_GOOD_SIGNATURE      0xAA55

#define QCA700X_SPI_INT_WRBUF_BELOW_WM (1 << 10)
#define QCA700X_SPI_INT_CPU_ON         (1 << 6)
#define QCA700X_SPI_INT_ADDR_ERR       (1 << 3)
#define QCA700X_SPI_INT_WRBUF_ERR      (1 << 2)
#define QCA700X_SPI_INT_RDBUF_ERR      (1 << 1)
#define QCA700X_SPI_INT_PKT_AVLBL      (1 << 0)

#define QCA700X_ETHERNET_FRAME_MIN_SIZE 60

#define QCA700X_BUFFER_SIZE 3163
#define QCA700X_HW_PKT_SIZE 4
#define QCA700X_MAC_SIZE 6

// Internal-RAM scratch buffer for SPI DMA transfers.
// The classic ESP32's SPIDMA cannot access PSRAM, so this will end up in DRAM.
// Sized to hold a full Ethernet frame (1522 bytes) plus
// the QCA700x send framing (8 byte header + 2 byte footer).
#define QCA700X_DMA_SCRATCH_SIZE 1536

// Maximum number of frames waiting to be sent to the modem. Writes are
// asynchronous. Frames are sent in FIFO order. On overflow new frames are dropped.
#define QCA700X_TX_QUEUE_LENGTH 8

#define QCA700X_SEND_HEADER_SIZE (4+2+2)
#define QCA700X_SEND_FOOTER_SIZE 2

#define QCA700X_RECV_HEADER_SIZE        (4+4+2+2)
#define QCA700X_RECV_FOOTER_SIZE        2
#define QCA700X_RECV_BUFFER_MIN_SIZE    (QCA700X_ETHERNET_FRAME_MIN_SIZE + QCA700X_RECV_HEADER_SIZE + QCA700X_RECV_FOOTER_SIZE)

typedef struct esp_qca700x_netif_driver_s {
    esp_netif_driver_base_t base;
    void *qca700x;
} esp_qca700x_netif_driver_t;

class QCA700x final
{
private:
    bool spi_init();
    bool external_read_to_scratch(const uint16_t length);
    void start_burst_read();
    bool finish_burst_read(const bool wait);
    void start_burst_write();
    bool finish_burst_write(const bool wait);
    bool finish_pending_op(const bool wait);
    void start_next_op();
    void setup_l2tap();
    void flush_receive_buffer();  // Flush hardware buffer only
    int16_t find_sof_marker(const uint8_t *data, uint16_t length);

    spi_device_handle_t spi_dev = nullptr;

    uint8_t *dma_scratch = nullptr;

    // Asynchronous burst read state.
    bool read_in_flight = false;
    uint16_t read_length = 0;
    spi_transaction_t read_transaction = {};

    // Asynchronous burst write state.
    struct TxFrame {
        uint8_t *buffer; // DMA-capable, [header | payload | footer]
        uint16_t total_length;
    };

    bool write_in_flight = false;
    spi_transaction_t write_transaction = {};
    TxFrame tx_queue[QCA700X_TX_QUEUE_LENGTH] = {};
    size_t tx_queue_head = 0;
    size_t tx_queue_count = 0;

    // SPI receive buffer for polling
    uint8_t *spi_buffer = nullptr;
    uint16_t spi_buffer_length = 0;

    // Set when an IPv6 packet arrives (for SLAC WaitForSDP -> LinkDetected transition)
    bool ipv6_packet_received = false;

    bool spi_initialized = false;

    // Modem presence verified via SPI signature check; frame processing is
    // paused while false to avoid parsing garbage from a disconnected bus.
    bool modem_detected = false;

    // Previous frame info for corruption debugging
    uint16_t prev_frame_length = 0;
    uint16_t prev_frame_eth_type = 0;
    uint8_t prev_frame_header[32] = {0};
    // Previous buffer state for debugging
    uint16_t prev_total_frame_length = 0;
    int32_t prev_remaining_length = 0;
    uint16_t prev_buf_len_before = 0;

public:
    esp_qca700x_netif_driver_t driver;
    esp_eth_netif_glue_handle_t handle;
    esp_netif_t *netif;
    int tap = -1;
    uint8_t mac[QCA700X_MAC_SIZE];

    QCA700x(){}
    uint16_t read_register(const uint16_t reg);
    void write_register(const uint16_t reg, const uint16_t value);
    void write_burst(const uint8_t *data, const uint16_t length);
    int16_t check_receive_frame(const uint8_t *data, const uint16_t length, uint16_t *total_frame_length_out = nullptr);

    void setup_netif();
    void received_data_to_netif(const uint8_t *data, const uint16_t length);
    bool get_ip6_linklocal(esp_ip6_addr_t *if_ip6);
    void link_up();
    void link_down();
    void state_machine_loop();
    void flush_all_buffers();  // Flush both hardware and local buffers

    // Check if l2tap is ready for use
    bool is_l2tap_ready() const { return tap >= 0; }

    // Check and clear IPv6 packet received flag (for SLAC WaitForSDP -> LinkDetected transition)
    bool check_and_clear_ipv6_received() {
        bool received = ipv6_packet_received;
        ipv6_packet_received = false;
        return received;
    }

    void set_modem_detected(const bool detected) { modem_detected = detected; }
    bool is_modem_detected() const { return modem_detected; }
    bool is_spi_ready() const { return spi_initialized; }
};
