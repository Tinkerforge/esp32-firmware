/*
 * Copyright (C) 2020 Erik Fleckstein <erik@tinkerforge.com>
 * Copyright (C) 2026 Olaf Lüke <olaf@tinkerforge.com>
 *
 * Redistribution and use in source and binary forms of this file,
 * with or without modification, are permitted. See the Creative
 * Commons Zero (CC0 1.0) License for more details.
 */

#if TF_HAL_ESPIDF == 1

#include "hal_espidf_esp32_ethernet_brick.h"

#include <stdio.h>

#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <soc/gpio_reg.h>

#include "bindings/config.h"
#include "bindings/errors.h"

typedef struct TF_Port {
    char port_name;

    TF_PortCommon port_common;
} TF_Port;

#define TF_PORT(port_name) {port_name, {._to_init = 0}}

TF_Port *ports;

static const int CS_PIN_0 = 12;
static const int CS_PIN_1 = 13;
static const int CS_PIN_2 = 14;
static const int CLK_PIN  = 32;
static const int MOSI_PIN = 33;
static const int MISO_PIN = 35;

static void select_demux(uint8_t port_id) {
    // As we always alternate between select_demux and deselect_demux, we can assume,
    // that all three pins are high. Clearing the pins we want to set to 0 is sufficient.

    // invert to use the W1TC (write 1 to clear) register
    uint8_t inv = ~port_id;

    REG_WRITE(GPIO_OUT_W1TC_REG, (inv & 0x07) << CS_PIN_0);
}

static void deselect_demux() {
    REG_WRITE(GPIO_OUT_W1TS_REG, 0x07 << CS_PIN_0);
}

int tf_hal_create(TF_HAL *hal, uint8_t max_ports) {
    int rc = tf_hal_common_create(hal);

    if (rc != TF_E_OK) {
        return rc;
    }

    ports = new TF_Port[max_ports];
    for(size_t i = 0; i < max_ports; ++i) {
        ports[i] = TF_Port{(char)('A' + i), {._to_init = 0}};
    };

    // Initialize HSPI bus
    spi_bus_config_t bus_config = {};
    bus_config.mosi_io_num      = MOSI_PIN;
    bus_config.miso_io_num      = MISO_PIN;
    bus_config.sclk_io_num      = CLK_PIN;
    bus_config.quadwp_io_num    = -1;
    bus_config.quadhd_io_num    = -1;
    bus_config.max_transfer_sz  = TF_SPITFP_MAX_MESSAGE_LENGTH;

    esp_err_t err = spi_bus_initialize(SPI2_HOST, &bus_config, SPI_DMA_CH_AUTO);

    if (err != ESP_OK) {
        return TF_E_NOT_SUPPORTED;
    }

    // SPI device config: Mode 3 (CPOL=1, CPHA=1), 1.4 MHz, no auto-CS
    spi_device_interface_config_t device_config = {};
    device_config.mode                          = 3;
    device_config.clock_speed_hz                = 1400000;
    device_config.spics_io_num                  = -1;  // Manual CS management via demux
    device_config.queue_size                    = 1;

    err = spi_bus_add_device(SPI2_HOST, &device_config, &hal->hspi_dev);

    if (err != ESP_OK) {
        spi_bus_free(SPI2_HOST);
        return TF_E_NOT_SUPPORTED;
    }

    gpio_set_direction((gpio_num_t)CS_PIN_0, GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t)CS_PIN_1, GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t)CS_PIN_2, GPIO_MODE_OUTPUT);
    deselect_demux();

    return tf_hal_common_prepare(hal, max_ports, 50000);
}

int tf_hal_destroy(TF_HAL *hal) {
    spi_bus_remove_device(hal->hspi_dev);
    spi_bus_free(SPI2_HOST);

    delete[] ports;

    return TF_E_OK;
}

int tf_hal_chip_select(TF_HAL *hal, uint8_t port_id, bool enable) {
    if (enable) {
        select_demux(port_id);
    } else {
        deselect_demux();
    }

    return TF_E_OK;
}

int tf_hal_transceive(TF_HAL *hal, uint8_t port_id, const uint8_t *write_buffer, uint8_t *read_buffer, uint32_t length) {
    if (length == 0) {
        return TF_E_OK;
    }

    spi_transaction_t transaction = {};
    transaction.length            = length * 8;  // Transaction length in bits
    transaction.tx_buffer         = write_buffer;
    transaction.rx_buffer         = read_buffer;

    esp_err_t err = spi_device_transmit(hal->hspi_dev, &transaction);

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
