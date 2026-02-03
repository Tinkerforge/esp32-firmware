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
        return;
    }

    // Filter only HomePlug frames (0x88E1)
    uint16_t eth_type_filter_homeplug = SLAC_ETHERNET_TYPE_HOMEPLUG;
    ret = ioctl(tap, L2TAP_S_RCV_FILTER, &eth_type_filter_homeplug);
    if (ret < 0) {
        logger.printfln("Failed to set l2tap receive filter: errno %d", errno);
        close(tap);
        tap = -1;
        return;
    }

    logger.printfln("l2tap initialized successfully (fd=%d, filter=0x%04X)", tap, eth_type_filter_homeplug);
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

            logger.printfln("driver_set_mac_filter called with handle %p; add %d; mac:%s", handle, add, mac_str);

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
        int16_t ethernet_frame_length = check_receive_frame(spi_buffer, spi_buffer_length);

        // Error -4 means partial frame - need to read more data
        if (ethernet_frame_length == -4) {
            break;
        }

        if (ethernet_frame_length < 0) {
            // Invalid frame, discard buffer
            logger.printfln("QCA700x: Ethernet frame error: %d", ethernet_frame_length);
            spi_buffer_length = 0;
            break;
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

        // Remove processed frame from buffer
        const uint16_t total_frame_length = ethernet_frame_length + QCA700X_RECV_HEADER_SIZE + QCA700X_RECV_FOOTER_SIZE;
        const int32_t remaining_length = spi_buffer_length - total_frame_length;

        if (remaining_length > 0) {
            // Move remaining data to start of buffer
            memmove(spi_buffer, spi_buffer + total_frame_length, remaining_length);
            spi_buffer_length = remaining_length;
        } else {
            spi_buffer_length = 0;
        }
    }
}
