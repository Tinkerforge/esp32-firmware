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
    void spi_select();
    void spi_deselect();
    void spi_write_16bit_value(const uint16_t value);
    uint16_t spi_read_16bit_value();
    void spi_read(uint8_t *data, const uint32_t length);
    void spi_write(const uint8_t *data, const uint16_t length);
    void spi_write_header(const uint16_t length);
    void spi_write_footer();

public:
    esp_qca700x_netif_driver_t driver;
    esp_eth_netif_glue_handle_t handle;
    esp_netif_t *netif;
    int tap;
    uint8_t mac[QCA700X_MAC_SIZE];

    QCA700x(){}
    uint16_t read_register(const uint16_t reg);
    void write_register(const uint16_t reg, const uint16_t value);
    uint16_t read_burst(uint8_t *data, const uint16_t length);
    void write_burst(const uint8_t *data, const uint16_t length);
    int16_t check_receive_frame(const uint8_t *data, const uint16_t length);

    void setup_netif();
    void received_data_to_netif(const uint8_t *data, const uint16_t length);
    void get_ip6_linklocal(esp_ip6_addr_t *if_ip6);
    void link_up();
    void link_down();
    void state_machine_loop();
};