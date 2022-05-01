/*
 * Copyright (C) 2020 Erik Fleckstein <erik@tinkerforge.com>
 *
 * Redistribution and use in source and binary forms of this file,
 * with or without modification, are permitted. See the Creative
 * Commons Zero (CC0 1.0) License for more details.
 */

#include "hal_arduino_esp32_brick.h"

#include <Arduino.h>

#include "bindings/config.h"
#include "bindings/errors.h"

typedef struct TF_Port {
    uint8_t chip_select_pin;
    uint8_t spi;
    char port_name;

    TF_PortCommon port_common;
} TF_Port;

#define TF_PORT(chip_select_pin, spi, port_name) {chip_select_pin, spi, port_name, {._to_init = 0}}

static TF_Port ports[6] = {
    TF_PORT(16, VSPI, 'A'),
    TF_PORT(33, VSPI, 'B'),
    TF_PORT(17, VSPI, 'C'),

    TF_PORT(25, HSPI, 'D'),
    TF_PORT(26, HSPI, 'E'),
    TF_PORT(27, HSPI, 'F')
};

#define PORT_COUNT (sizeof(ports) / sizeof(ports[0]))

int tf_hal_create(TF_HAL *hal) {
    int rc = tf_hal_common_create(hal);

    if (rc != TF_E_OK) {
        return rc;
    }

    hal->spi_settings = SPISettings(1400000, SPI_MSBFIRST, SPI_MODE3);

    hal->hspi = SPIClass(HSPI);
    hal->hspi.begin();

    hal->vspi = SPIClass(VSPI);
    hal->vspi.begin();

    for (int i = 0; i < PORT_COUNT; ++i) {
        pinMode(ports[i].chip_select_pin, OUTPUT);
        digitalWrite(ports[i].chip_select_pin, HIGH);
    }

    return tf_hal_common_prepare(hal, PORT_COUNT, 50000);
}

int tf_hal_destroy(TF_HAL *hal) {
    hal->hspi.end();
    hal->vspi.end();

    return TF_E_OK;
}

static SPIClass *get_spi(TF_HAL *hal, uint8_t port_id) {
    SPIClass *spi = NULL;

    if (ports[port_id].spi == HSPI) {
        spi = &hal->hspi;
    } else if (ports[port_id].spi == VSPI) {
        spi = &hal->vspi;
    }

    return spi;
}

int tf_hal_chip_select(TF_HAL *hal, uint8_t port_id, bool enable) {
    SPIClass *spi = get_spi(hal, port_id);

    if (spi == NULL) {
        return TF_E_DEVICE_NOT_FOUND;
    }

    if (enable) {
        spi->beginTransaction(hal->spi_settings);
        digitalWrite(ports[port_id].chip_select_pin, LOW);
    } else {
        digitalWrite(ports[port_id].chip_select_pin, HIGH);
        spi->endTransaction();
    }

    return TF_E_OK;
}

int tf_hal_transceive(TF_HAL *hal, uint8_t port_id, const uint8_t *write_buffer, uint8_t *read_buffer, uint32_t length) {
    SPIClass *spi = get_spi(hal, port_id);

    if (spi == NULL) {
        return TF_E_DEVICE_NOT_FOUND;
    }

    memcpy(read_buffer, write_buffer, length);
    spi->transfer(read_buffer, length);

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
