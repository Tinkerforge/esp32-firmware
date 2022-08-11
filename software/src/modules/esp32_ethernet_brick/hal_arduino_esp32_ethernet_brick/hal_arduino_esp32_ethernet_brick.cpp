/*
 * Copyright (C) 2020 Erik Fleckstein <erik@tinkerforge.com>
 *
 * Redistribution and use in source and binary forms of this file,
 * with or without modification, are permitted. See the Creative
 * Commons Zero (CC0 1.0) License for more details.
 */

#include "hal_arduino_esp32_ethernet_brick.h"

#include <Arduino.h>

#include "bindings/config.h"
#include "bindings/errors.h"

typedef struct TF_Port {
    char port_name;

    TF_PortCommon port_common;
} TF_Port;

#define TF_PORT(port_name) {port_name, {._to_init = 0}}

TF_Port ports[6] = {
    TF_PORT('A'),
    TF_PORT('B'),
    TF_PORT('C'),
    TF_PORT('D'),
    TF_PORT('E'),
    TF_PORT('F')
};

#define PORT_COUNT (sizeof(ports) / sizeof(ports[0]))

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

int tf_hal_create(TF_HAL *hal) {
    int rc = tf_hal_common_create(hal);

    if (rc != TF_E_OK) {
        return rc;
    }

    hal->spi_settings = SPISettings(1400000, SPI_MSBFIRST, SPI_MODE3);
    hal->hspi = new SPIClass(HSPI);
    hal->hspi->begin(CLK_PIN, MISO_PIN, MOSI_PIN);

    pinMode(CS_PIN_0, OUTPUT);
    pinMode(CS_PIN_1, OUTPUT);
    pinMode(CS_PIN_2, OUTPUT);
    deselect_demux();

    return tf_hal_common_prepare(hal, PORT_COUNT, 50000);
}

int tf_hal_destroy(TF_HAL *hal) {
    hal->hspi->end();

    return TF_E_OK;
}

int tf_hal_chip_select(TF_HAL *hal, uint8_t port_id, bool enable) {
    if (enable) {
        hal->hspi->beginTransaction(hal->spi_settings);
        select_demux(port_id);
    } else {
        deselect_demux();
        hal->hspi->endTransaction();
    }

    return TF_E_OK;
}

int tf_hal_transceive(TF_HAL *hal, uint8_t port_id, const uint8_t *write_buffer, uint8_t *read_buffer, uint32_t length) {
    memcpy(read_buffer, write_buffer, length);
    hal->hspi->transfer(read_buffer, length);

    return TF_E_OK;
}

uint32_t tf_hal_current_time_us(TF_HAL *hal) {
    return micros();
}

void tf_hal_sleep_us(TF_HAL *hal, uint32_t us) {
    while (us > 16000) {
        delay(16);
        us -= 16000;
    }

    delayMicroseconds(us);
}

TF_HALCommon *tf_hal_get_common(TF_HAL *hal) {
    return &hal->hal_common;
}

void tf_hal_log_message(const char *msg, size_t len) {
    Serial.write((const uint8_t *)msg, len);
}

void tf_hal_log_newline(void) {
    Serial.println("");
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
