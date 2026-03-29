/*
 * Copyright (C) 2020 Erik Fleckstein <erik@tinkerforge.com>
 * Copyright (C) 2026 Olaf Lüke <olaf@tinkerforge.com>
 *
 * Redistribution and use in source and binary forms of this file,
 * with or without modification, are permitted. See the Creative
 * Commons Zero (CC0 1.0) License for more details.
 */

#if TF_HAL_ESPIDF == 1

#include "hal_espidf_esp32_brick.h"

#include <stdio.h>

#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "bindings/config.h"
#include "bindings/errors.h"

#define TF_SPI_BUS_HSPI SPI2_HOST
#define TF_SPI_BUS_VSPI SPI3_HOST

typedef struct TF_Port {
    uint8_t chip_select_pin;
    uint8_t spi;
    char port_name;

    TF_PortCommon port_common;
} TF_Port;

#define TF_PORT(chip_select_pin, spi, port_name) {chip_select_pin, spi, port_name, {._to_init = 0}}

static TF_Port ports[6] = {
    TF_PORT(16, TF_SPI_BUS_VSPI, 'A'),
    TF_PORT(33, TF_SPI_BUS_VSPI, 'B'),
    TF_PORT(17, TF_SPI_BUS_VSPI, 'C'),

    TF_PORT(25, TF_SPI_BUS_HSPI, 'D'),
    TF_PORT(26, TF_SPI_BUS_HSPI, 'E'),
    TF_PORT(27, TF_SPI_BUS_HSPI, 'F')
};

#define PORT_COUNT (sizeof(ports) / sizeof(ports[0]))

// Default IOMUX pin numbers for HSPI (SPI2) on ESP32
#define HSPI_PIN_CLK  14
#define HSPI_PIN_MISO 12
#define HSPI_PIN_MOSI 13

// Default IOMUX pin numbers for VSPI (SPI3) on ESP32
#define VSPI_PIN_CLK  18
#define VSPI_PIN_MISO 19
#define VSPI_PIN_MOSI 23

int tf_hal_create(TF_HAL *hal) {
    int rc = tf_hal_common_create(hal);

    if (rc != TF_E_OK) {
        return rc;
    }

    // Initialize HSPI bus
    spi_bus_config_t hspi_bus_config = {};
    hspi_bus_config.mosi_io_num = HSPI_PIN_MOSI;
    hspi_bus_config.miso_io_num = HSPI_PIN_MISO;
    hspi_bus_config.sclk_io_num = HSPI_PIN_CLK;
    hspi_bus_config.quadwp_io_num = -1;
    hspi_bus_config.quadhd_io_num = -1;
    hspi_bus_config.max_transfer_sz = TF_SPITFP_MAX_MESSAGE_LENGTH;

    esp_err_t err = spi_bus_initialize(TF_SPI_BUS_HSPI, &hspi_bus_config, SPI_DMA_CH_AUTO);

    if (err != ESP_OK) {
        return TF_E_NOT_SUPPORTED;
    }

    // Initialize VSPI bus
    spi_bus_config_t vspi_bus_config = {};
    vspi_bus_config.mosi_io_num      = VSPI_PIN_MOSI;
    vspi_bus_config.miso_io_num      = VSPI_PIN_MISO;
    vspi_bus_config.sclk_io_num      = VSPI_PIN_CLK;
    vspi_bus_config.quadwp_io_num    = -1;
    vspi_bus_config.quadhd_io_num    = -1;
    vspi_bus_config.max_transfer_sz  = TF_SPITFP_MAX_MESSAGE_LENGTH;

    err = spi_bus_initialize(TF_SPI_BUS_VSPI, &vspi_bus_config, SPI_DMA_CH_AUTO);

    if (err != ESP_OK) {
        spi_bus_free(TF_SPI_BUS_HSPI);
        return TF_E_NOT_SUPPORTED;
    }

    // SPI device config: Mode 3 (CPOL=1, CPHA=1), 1.4 MHz, no auto-CS
    spi_device_interface_config_t device_config = {};
    device_config.mode                          = 3;
    device_config.clock_speed_hz                = 1400000;
    device_config.spics_io_num                  = -1;  // Manual CS management via GPIO
    device_config.queue_size                    = 1;

    err = spi_bus_add_device(TF_SPI_BUS_HSPI, &device_config, &hal->hspi_dev);

    if (err != ESP_OK) {
        spi_bus_free(TF_SPI_BUS_HSPI);
        spi_bus_free(TF_SPI_BUS_VSPI);
        return TF_E_NOT_SUPPORTED;
    }

    err = spi_bus_add_device(TF_SPI_BUS_VSPI, &device_config, &hal->vspi_dev);

    if (err != ESP_OK) {
        spi_bus_remove_device(hal->hspi_dev);
        spi_bus_free(TF_SPI_BUS_HSPI);
        spi_bus_free(TF_SPI_BUS_VSPI);
        return TF_E_NOT_SUPPORTED;
    }

    for (int i = 0; i < PORT_COUNT; ++i) {
        gpio_set_direction((gpio_num_t)ports[i].chip_select_pin, GPIO_MODE_OUTPUT);
        gpio_set_level((gpio_num_t)ports[i].chip_select_pin, 1);
    }

    return tf_hal_common_prepare(hal, PORT_COUNT, 50000);
}

int tf_hal_destroy(TF_HAL *hal) {
    spi_bus_remove_device(hal->hspi_dev);
    spi_bus_remove_device(hal->vspi_dev);
    spi_bus_free(TF_SPI_BUS_HSPI);
    spi_bus_free(TF_SPI_BUS_VSPI);

    return TF_E_OK;
}

static spi_device_handle_t get_spi_device(TF_HAL *hal, uint8_t port_id) {
    if (ports[port_id].spi == TF_SPI_BUS_HSPI) {
        return hal->hspi_dev;
    }

    return hal->vspi_dev;
}

int tf_hal_chip_select(TF_HAL *hal, uint8_t port_id, bool enable) {
    if (enable) {
        gpio_set_level((gpio_num_t)ports[port_id].chip_select_pin, 0);
    } else {
        gpio_set_level((gpio_num_t)ports[port_id].chip_select_pin, 1);
    }

    return TF_E_OK;
}

int tf_hal_transceive(TF_HAL *hal, uint8_t port_id, const uint8_t *write_buffer, uint8_t *read_buffer, uint32_t length) {
    if (length == 0) {
        return TF_E_OK;
    }

    spi_device_handle_t dev = get_spi_device(hal, port_id);

    spi_transaction_t transaction = {};
    transaction.length            = length * 8;  // Transaction length in bits
    transaction.tx_buffer         = write_buffer;
    transaction.rx_buffer         = read_buffer;

    esp_err_t err = spi_device_transmit(dev, &transaction);

    return (err == ESP_OK) ? TF_E_OK : TF_E_NOT_SUPPORTED;
}

uint32_t tf_hal_current_time_us(TF_HAL *hal) {
    return esp_timer_get_time();
}

void tf_hal_sleep_us(TF_HAL *hal, uint32_t us) {
    while (us > 16000) {
        vTaskDelay(pdMS_TO_TICKS(16));
        us -= 16000;
    }

    int64_t end = esp_timer_get_time() + us;
    while (esp_timer_get_time() < end) {
        // busy wait
    }
}

TF_HALCommon *tf_hal_get_common(TF_HAL *hal) {
    return &hal->hal_common;
}

void tf_hal_log_message(const char *msg, size_t len) {
    printf("%.*s", (int)len, msg);
}

void tf_hal_log_newline(void) {
    printf("\n");
}

#if TF_IMPLEMENT_STRERROR != 0
const char *tf_hal_strerror(int e_code) {
    switch (e_code) {
        #include "bindings/error_cases.h"

        default:
            return "unknown error";
    }
}
#endif

char tf_hal_get_port_name(TF_HAL *hal, uint8_t port_id) {
    return ports[port_id].port_name;
}

TF_PortCommon *tf_hal_get_port_common(TF_HAL *hal, uint8_t port_id) {
    return &ports[port_id].port_common;
}

#endif
