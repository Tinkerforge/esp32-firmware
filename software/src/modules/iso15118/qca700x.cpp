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

// QCA7000/QCA7005 SPI driver for tinkerforge hal with simplet netif setup.
// With this QCA can be used in parallel with Bricklets on same SPI port.

#include "qca700x.h"

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "build.h"

#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include "esp_vfs_l2tap.h"
#include "esp_netif.h"
#include "esp_mac.h"

#include "lwip/ip_addr.h"
#include "bindings/hal_common.h"

extern TF_HAL hal;

QCA700x qca700x;

void QCA700x::spi_select()
{
    int rc = tf_hal_chip_select(&hal, QCA700X_HAL_SPI_PORT, true);
    if (rc != 0) {
        logger.printfln("spi_select error %d", rc);
    }
}

void QCA700x::spi_deselect()
{
    int rc = tf_hal_chip_select(&hal, QCA700X_HAL_SPI_PORT, false);
    if (rc != 0) {
        logger.printfln("spi_deselect error %d", rc);
    }
}

void QCA700x::spi_write(const uint8_t *data, const uint16_t length)
{
    uint8_t tmp_read_buf[length] = {0};
    int rc = tf_hal_transceive(&hal, QCA700X_HAL_SPI_PORT, data, tmp_read_buf, length);
    if (rc != 0) {
        logger.printfln("spi_write error %d", rc);
    }
}

void QCA700x::spi_read(uint8_t *data, const uint32_t length)
{
    const uint8_t tmp_write_buf[length] = {0};
    int rc = tf_hal_transceive(&hal, QCA700X_HAL_SPI_PORT, tmp_write_buf, data, length);
    if (rc != 0) {
        logger.printfln("spi_write error %d", rc);
    }
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
    const uint16_t available = read_register(QCA700X_SPI_REG_RDBUF_BYTE_AVA);
    if (available == 0) {
        return 0;
    }

    if (available <= length) {
        write_register(QCA700X_SPI_REG_BFR_SIZE, available);

        spi_select();
        spi_write_16bit_value(QCA700X_SPI_READ | QCA700X_SPI_EXTERNAL);
        spi_read(data, length);
        spi_deselect();

        iso15118.trace_array("QCA700x packet recv", data, available);
        return available;
    } else {
        logger.printfln("read_burst error: available %u > max length %u", available, length);
    }

    return 0;
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

    iso15118.trace_array("QCA700x packet send", data, length);
}

// Returns ethernet frame length if frame is valid, otherwise negative number
int16_t QCA700x::check_receive_frame(const uint8_t *data, const uint16_t length)
{
    // Check packet length
    uint32_t packet_length = data[3] | (data[2] << 8) | (data[1] << 16) | (data[0] << 24);
    if (packet_length > QCA700X_BUFFER_SIZE) {
        logger.printfln("QCA700x: Packet length too long: %lu > %u", packet_length, QCA700X_BUFFER_SIZE);
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

    // Check EOF (end of frame)
    if ((data[packet_length+QCA700X_HW_PKT_SIZE-2] != 0x55) || (data[packet_length+QCA700X_HW_PKT_SIZE-1] != 0x55)) {
        logger.printfln("QCA700x: Footer mismatch");
        return -4;
    }

    const uint32_t ethernet_frame_length = data[8] | (data[9] << 8);
    if (ethernet_frame_length < QCA700X_ETHERNET_FRAME_MIN_SIZE) {
        logger.printfln("QCA700x: Ethernet frame length too short: %lu < %u", ethernet_frame_length, QCA700X_ETHERNET_FRAME_MIN_SIZE);
        return -5;
    }

    if (ethernet_frame_length > QCA700X_BUFFER_SIZE) {
        logger.printfln("QCA700x: Ethernet frame length too long: %lu > %u", ethernet_frame_length, QCA700X_BUFFER_SIZE);
        return -6;
    }

    return ethernet_frame_length;
}


void QCA700x::setup_netif()
{
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
                qca700x.write_burst(static_cast<const uint8_t *>(data_copy), length);
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
                qca700x.write_burst(static_cast<const uint8_t *>(data_copy), length);
                free(data_copy);
            }, 0_ms);
            return ESP_OK;
        },
        .driver_free_rx_buffer = [](void *handle, void *buffer) {
            free(buffer);
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



/*
    ESP_ERROR_CHECK(esp_vfs_l2tap_intf_register(NULL));

    tap = open("/dev/net/tap", O_NONBLOCK);
    if (tap < 0) {
        logger.printfln("Failed to open /dev/net/tap");
        return;
    }

    // Set Ethernet interface on which to get raw frames
    int ret = ioctl(tap, L2TAP_S_INTF_DEVICE, "ETH_QCA");
    if (ret < 0) {
        logger.printfln("Failed to set interface device");
        return;
    }

    uint16_t eth_type_filter_homeplug = 0x88E1;
    ret = ioctl(tap, L2TAP_S_RCV_FILTER, &eth_type_filter_homeplug);
    if (ret < 0) {
        logger.printfln("Failed to set receive filter");
        return;
    }*/
}

void QCA700x::link_up()
{
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
    if (esp_netif_is_netif_up(netif)) {
        esp_netif_action_stop(netif, 0, 0, nullptr);
    }

}

void QCA700x::received_data_to_netif(const uint8_t *data, const uint16_t length)
{
    if (netif == NULL) {
        logger.printfln("QCA700x: netif not setup");
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

void QCA700x::get_ip6_linklocal(esp_ip6_addr_t *if_ip6)
{
    if (netif == NULL) {
        logger.printfln("get_ip6_linklocal: netif not setup");
        return;
    }

    esp_netif_get_ip6_linklocal(netif, if_ip6);
}

void QCA700x::state_machine_loop()
{
    /*
    uint8_t buffer[QCA700X_BUFFER_SIZE + QCA700X_HW_PKT_SIZE];

    uint16_t length = qca700x.read_burst(buffer, QCA700X_BUFFER_SIZE + QCA700X_HW_PKT_SIZE);
    if (length > 0) {
        logger.printfln("QCA700x: state_machine_loop length %d", length);
        received_data_to_netif(buffer, length);
        //write(tap, buffer, length);
    }

    // TODO: read_burst -> write(tap) here and read(tap) in slac.cpp
    */
}
