/*
 * Copyright (C) 2020 Erik Fleckstein <erik@tinkerforge.com>
 *
 * Redistribution and use in source and binary forms of this file,
 * with or without modification, are permitted. See the Creative
 * Commons Zero (CC0 1.0) License for more details.
 */

#include "hal_common.h"

#include <string.h>
#include <stdio.h>

#include "tfp.h"
#include "bricklet_unknown.h"
#include "base58.h"
#include "macros.h"
#include "errors.h"

int tf_hal_common_create(TF_HAL *hal) {
    TF_HALCommon *hal_common = tf_hal_get_common(hal);

    memset(hal_common, 0, sizeof(TF_HALCommon));

    return TF_E_OK;
}

int tf_hal_common_prepare(TF_HAL *hal, uint8_t port_count, uint32_t port_discovery_timeout_us) {
    TF_HALCommon *hal_common = tf_hal_get_common(hal);

    hal_common->timeout = port_discovery_timeout_us;
    hal_common->port_count = port_count;
    hal_common->tfps_used = 0;
    hal_common->device_overflow_count = 0;

    for (uint8_t port_id = 0; port_id < port_count; ++port_id) {
        TF_PortCommon *port_common = tf_hal_get_port_common(hal, port_id);
        TF_TFP tfp;
        TF_Unknown unknown;

        tf_spitfp_create(&port_common->spitfp, hal, port_id);
        tf_tfp_create(&tfp, 0, 0, &port_common->spitfp);

        int rc = tf_unknown_create(&unknown, &tfp);

        if (rc != TF_E_OK) {
            return rc;
        }

        rc = tf_unknown_comcu_enumerate(&unknown);

        if (rc == TF_E_OK) {
            tf_unknown_callback_tick(&unknown, port_discovery_timeout_us);
        }

        tf_unknown_destroy(&unknown);
    }

    if (hal_common->device_overflow_count > 0) {
        tf_hal_log_info("Additional devices found, but the inventory is full. Please increase TF_INVENTORY_SIZE.\n");

        return TF_E_TOO_MANY_DEVICES;
    }

    hal_common->timeout = 2500000;

    return TF_E_OK;
}

void tf_hal_enumerate_handler(TF_HAL *hal, uint8_t port_id, TF_PacketBuffer *payload) {
    TF_HALCommon *hal_common = tf_hal_get_common(hal);
    char uid[8]; tf_packet_buffer_pop_n(payload, (uint8_t *)uid, 8);
    char connected_uid[8]; tf_packet_buffer_pop_n(payload, (uint8_t *)connected_uid, 8);
    tf_packet_buffer_remove(payload, 7); // drop position, hardware version and firmware version
    uint16_t device_id = tf_packet_buffer_read_uint16_t(payload);
    tf_packet_buffer_remove(payload, 1); // drop enumeration type

    if (hal_common->tfps_used >= sizeof(hal_common->tfps) / sizeof(hal_common->tfps[0])) {
        ++hal_common->device_overflow_count;
        return;
    }

    uint32_t uid_num;

    if (tf_base58_decode(uid, &uid_num) != TF_E_OK) {
        return;
    }

    if (tf_hal_get_tfp(hal, NULL, &uid_num, NULL, NULL) != NULL) {
        return; // device already known
    }

    tf_hal_log_info("Found device %s of type %d at port %c\n", uid, device_id, tf_hal_get_port_name(hal, port_id));

    TF_PortCommon *port_common = tf_hal_get_port_common(hal, port_id);

    tf_tfp_create(&hal_common->tfps[hal_common->tfps_used++], uid_num, device_id, &port_common->spitfp);
}

static const char *alphabet = "0123456789abcdef";

static void log_unsigned(uint64_t value, uint8_t base) {
    if (base < 2 || base > 16) {
        return;
    }

    char buffer[64] = {0};
    uint64_t len = 0;

    do {
        uint64_t digit = value % base;

        buffer[64 - len - 1] = alphabet[digit];
        ++len;
        value /= base;
    } while (value > 0);

    tf_hal_log_message(buffer + (64 - len), len);

    return;
}

static void log_signed(int64_t value, uint8_t base) {
    if (value < 0) {
        tf_hal_log_message("-", 1);
        value = -value;
    }

    log_unsigned((uint64_t) value, base);

    return;
}

static void write_chunk(const char *fmt, const char *cursor) {
     if (cursor > fmt) {
        // cursor is on the first character that must not be printed
        size_t chunk_len = (size_t)(cursor - fmt) - 1;

        if (chunk_len != 0) {
            tf_hal_log_message(fmt, chunk_len);
        }
    }
}

void tf_hal_printf(const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);

    char character;
    const char *cursor = fmt;

    while ((character = *(cursor++))) {
        if (character == '\n') {
            write_chunk(fmt, cursor);
            fmt = cursor;

            tf_hal_log_newline();

            continue;
        }

        if (character != '%') {
            continue;
        }

        write_chunk(fmt, cursor);
        fmt = cursor;

        // Parse integer prefixes
        int width = 0;

        if (strncmp(cursor, "I8", 2) == 0) {
            width = 1;
            cursor += 2;
        } else if (strncmp(cursor, "I16", 3) == 0) {
            width = 2;
            cursor += 3;
        } else if (strncmp(cursor, "I32", 3) == 0) {
            width = 4;
            cursor += 3;
        } else if (strncmp(cursor, "I64", 3) == 0) {
            width = 8;
            cursor += 3;
        }

        // Parse and handle non-integer placeholders
        character = *(cursor++);

        switch (character) {
            case '\n': {
                write_chunk(fmt, cursor);
                tf_hal_log_newline();
                fmt = cursor;

                continue;
            }

            case '\0': {
                write_chunk(fmt, cursor);
                va_end(va);

                return;
            }

            case 'c': {
                char c = (char) va_arg(va, int);

                tf_hal_log_message(&c, 1);
                fmt = cursor;

                continue;
            }

            case 's': {
                const char *str = va_arg(va, char*);

                tf_hal_log_message(str, strlen(str));
                fmt = cursor;

                continue;
            }

            case '%': {
                tf_hal_log_message("%", 1);
                fmt = cursor;

                continue;
            }
        }

        // Parse integer placeholders
        bool sign = 0;
        uint8_t base = 0;

        switch (character) {
            case 'u': {
                if (width == 0) {
                    width = sizeof(unsigned int);
                }

                base = 10;
                sign = false;

                break;
            }

            case 'b': {
                if (width == 0) {
                    width = sizeof(unsigned int);
                }

                base = 2;
                sign = false;

                break;
            }

            case 'd': {
                if (width == 0) {
                    width = sizeof(int);
                }

                base = 10;
                sign = true;

                break;
            }

            case 'X':
            case 'x': {
                if (width == 0) {
                    width = sizeof(unsigned int);
                }

                base = 16;
                sign = false;

                break;
            }

            default: {
                tf_hal_log_message("%", 1);
                tf_hal_log_message(&character, 1);
                fmt = cursor;

                continue;
            }
        }

        // Handle integer placeholders with optional prefix.
        uint64_t value = 0;

        // double casts fix implicit conversion signedness change warnings
        switch (width) {
            case 1:
                if (sign) {
                    value = (uint64_t)((int8_t)va_arg(va, int));
                } else {
                    value = (uint8_t)va_arg(va, unsigned int);
                }

                break;

            case 2:
                if (sign) {
                    value = (uint64_t)((int16_t)va_arg(va, int));
                } else {
                    value = (uint16_t)va_arg(va, unsigned int);
                }

                break;

            case 4:
                if (sign) {
                    value = (uint64_t)va_arg(va, int32_t);
                } else {
                    value = va_arg(va, uint32_t);
                }

                break;

            case 8:
                if (sign) {
                    value = (uint64_t)va_arg(va, int64_t);
                } else {
                    value = va_arg(va, uint64_t);
                }

                break;
        }

        if (sign) {
            log_signed((int64_t)value, base);
        } else {
            log_unsigned((uint64_t)value, base);
        }

        fmt = cursor;
    }

    write_chunk(fmt, cursor);

    va_end(va);
}

void tf_hal_set_timeout(TF_HAL *hal, uint32_t timeout_us) {
    tf_hal_get_common(hal)->timeout = timeout_us;
}

uint32_t tf_hal_get_timeout(TF_HAL *hal) {
    return tf_hal_get_common(hal)->timeout;
}

int tf_hal_get_device_info(TF_HAL *hal, uint16_t index, char ret_uid[7], char *ret_port_name, uint16_t *ret_device_id) {
    TF_HALCommon *hal_common = tf_hal_get_common(hal);

    if (index >= hal_common->tfps_used) {
        return TF_E_DEVICE_NOT_FOUND;
    }

    if (ret_uid != NULL) {
        tf_base58_encode(hal_common->tfps[index].uid, ret_uid);
    }

    if (ret_port_name != NULL) {
        *ret_port_name = tf_hal_get_port_name(hal, hal_common->tfps[index].spitfp->port_id);
    }

    if (ret_port_name != NULL) {
        *ret_device_id = hal_common->tfps[index].device_id;
    }

    return TF_E_OK;
}

static TF_TFP *next_callback_tick_tfp(TF_HAL *hal) {
    TF_HALCommon *hal_common = tf_hal_get_common(hal);

    ++hal_common->callback_tick_index;

    if (hal_common->callback_tick_index >= hal_common->tfps_used) {
        hal_common->callback_tick_index -= hal_common->tfps_used;
    }

    for (uint16_t i = hal_common->callback_tick_index; i < hal_common->callback_tick_index + hal_common->tfps_used; ++i) {
        uint16_t k = i;

        if (k >= hal_common->tfps_used) {
            k -= hal_common->tfps_used;
        }

        TF_TFP *tfp = &hal_common->tfps[k];

        if (tfp->needs_callback_tick) {
            hal_common->callback_tick_index = k;
            return tfp;
        }
    }

    return NULL;
}

#if TF_NET_ENABLE != 0
static uint8_t enumerate_request[8] = {
    0, 0, 0, 0, // uid 1
    8, // length 8
    254, // fid 254
    0x40, // seq num 4
    0 // no error
};

static TF_TFPHeader enumerate_request_header = {
    .uid=0,
    .length=8,
    .fid=254,
    .seq_num=4,
    .response_expected=false,
    .options=0,
    .error_code=0,
    .flags=0
};
#endif

int tf_hal_tick(TF_HAL *hal, uint32_t timeout_us) {
#if TF_NET_ENABLE != 0
    uint32_t deadline_us = tf_hal_current_time_us(hal) + timeout_us;
    TF_HALCommon *hal_common = tf_hal_get_common(hal);
    TF_Net *net = hal_common->net;
    uint8_t ignored;
    int result;

    if (net != NULL) {
        tf_net_tick(net);

        for (uint16_t i = 0; i < hal_common->tfps_used; ++i) {
            if (hal_common->tfps[i].send_enumerate_request) {
                if (hal_common->tfps[i].spitfp->send_buf[0] == 0) {
                    tf_tfp_inject_packet(&hal_common->tfps[i], &enumerate_request_header, enumerate_request);
                    // TODO: What timeout to use here? If decided, use return value to check for the timeout, maybe increase an error count
                    result = tf_tfp_send_packet(&hal_common->tfps[i], false, deadline_us, &ignored);
                    (void)! tf_tfp_finish_send(&hal_common->tfps[i], result, deadline_us); // ignore result for now: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66425#c34
                    hal_common->tfps[i].send_enumerate_request = false;
                }
            }
        }

        TF_TFPHeader header;
        int packet_id = -1;

        while (!tf_hal_deadline_elapsed(hal, deadline_us) && tf_net_get_available_tfp_header(net, &header, &packet_id)) {
            uint8_t pid = (uint8_t)packet_id;

            // We should never get callback packets from the network side of things. Drop them.
            if (header.seq_num == 0) {
                tf_net_drop_packet(net, pid);

                continue;
            }

            // Handle enumerate requests
            if (header.fid == 254 && header.uid == 0 && header.length == 8) {
                for (uint16_t i = 0; i < hal_common->tfps_used; ++i) {
                    hal_common->tfps[i].send_enumerate_request = true;
                }

                tf_net_drop_packet(net, pid);

                continue;
            }

            bool device_found = false;
            bool dispatched = false;

            for (uint16_t i = 0; i < hal_common->tfps_used; ++i) {
                if (header.uid != hal_common->tfps[i].uid) {
                    continue;
                }

                device_found = true;

                // Intentionally don't use get_payload_buffer here: the payload buffer is at send_buf + SPITFP_HEADER_SIZE
                // But the "is the buffer filled" marker is just the SPITFP packet length, i.e. before the SPITFP payload.
                if (hal_common->tfps[i].spitfp->send_buf[0] != 0) {
                    // Send buffer is not empty.
                    continue;
                }

                uint8_t buf[TF_TFP_MAX_MESSAGE_LENGTH] = {0};
                tf_net_get_packet(net, pid, buf);
                tf_tfp_inject_packet(&hal_common->tfps[i], &header, buf);

                // TODO: What timeout to use here? If decided, use return value to check for the timeout, maybe increase an error count
                result = tf_tfp_send_packet(&hal_common->tfps[i], false, deadline_us, &ignored);
                (void)! tf_tfp_finish_send(&hal_common->tfps[i], result, deadline_us); // ignore result for now: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66425#c34
                dispatched = true;
            }

            if (!device_found || (device_found && dispatched)) {
                tf_net_drop_packet(net, pid);
            }
        }
    }
#endif

    tf_hal_callback_tick(hal, timeout_us);

    return TF_E_OK;
}

int tf_hal_callback_tick(TF_HAL *hal, uint32_t timeout_us) {
    uint32_t deadline_us = tf_hal_current_time_us(hal) + timeout_us;
    TF_TFP *tfp = NULL;

    do {
        tfp = next_callback_tick_tfp(hal);

        if (tfp == NULL) {
            return TF_E_OK;
        }

        int result = tf_tfp_callback_tick(tfp, tf_hal_current_time_us(hal));

        if (result != TF_E_OK) {
            return result;
        }
    } while (!tf_hal_deadline_elapsed(hal, deadline_us));

    return TF_E_OK;
}

bool tf_hal_deadline_elapsed(TF_HAL *hal, uint32_t deadline_us) {
    uint32_t now = tf_hal_current_time_us(hal);

    return ((uint32_t)(now - deadline_us)) < (UINT32_MAX / 2);
}

int tf_hal_get_error_counters(TF_HAL *hal,
                              char port_name,
                              uint32_t *ret_spitfp_error_count_checksum,
                              uint32_t *ret_spitfp_error_count_frame,
                              uint32_t *ret_tfp_error_count_frame,
                              uint32_t *ret_tfp_error_count_unexpected) {
    TF_HALCommon *hal_common = tf_hal_get_common(hal);
    TF_TFP *tfp = NULL;
    uint32_t spitfp_error_count_checksum = 0;
    uint32_t spitfp_error_count_frame = 0;
    uint32_t tfp_error_count_frame = 0;
    uint32_t tfp_error_count_unexpected = 0;
    bool port_found = false;

    for (uint16_t i = 0; i < hal_common->tfps_used; ++i) {
        if (tf_hal_get_port_name(hal, hal_common->tfps[i].spitfp->port_id) != port_name) {
            continue;
        }

        port_found = true;
        tfp = &hal_common->tfps[i];

        if (tfp == NULL) {
            continue;
        }

        spitfp_error_count_checksum += tfp->spitfp->error_count_checksum;
        spitfp_error_count_frame += tfp->spitfp->error_count_frame;

        tfp_error_count_frame += tfp->error_count_frame;
        tfp_error_count_unexpected += tfp->error_count_unexpected;
    }

    if (ret_spitfp_error_count_checksum != NULL) {
        *ret_spitfp_error_count_checksum = spitfp_error_count_checksum;
    }

    if (ret_spitfp_error_count_frame != NULL) {
        *ret_spitfp_error_count_frame = spitfp_error_count_frame;
    }

    if (ret_tfp_error_count_frame != NULL) {
        *ret_tfp_error_count_frame = tfp_error_count_frame;
    }

    if (ret_tfp_error_count_unexpected != NULL) {
        *ret_tfp_error_count_unexpected = tfp_error_count_unexpected;
    }

    return port_found ? TF_E_OK : TF_E_PORT_NOT_FOUND;
}

void tf_hal_set_net(TF_HAL *hal, TF_Net *net) {
    TF_HALCommon *hal_common = tf_hal_get_common(hal);

    hal_common->net = net;
}

// FIXME: For a device_id-only lookup to produce stable results the TFP order has to be stable.
//        The order has to be predictable as wall for the user. This means having the TFPs
//        sorted by port_id order port_name. This is currently not the case, I think.
TF_TFP *tf_hal_get_tfp(TF_HAL *hal, uint16_t *index_ptr, const uint32_t *uid, const uint8_t *port_id, const uint16_t *device_id) {
    TF_HALCommon *hal_common = tf_hal_get_common(hal);
    uint16_t index = 0;

    if (index_ptr != NULL) {
        index = *index_ptr;
    }

    if (index >= hal_common->tfps_used) {
        index -= hal_common->tfps_used;
    }

    for (uint16_t i = index; i < index + hal_common->tfps_used; ++i) {
        uint16_t k = i;

        if (k >= hal_common->tfps_used) {
            k -= hal_common->tfps_used;
        }

        TF_TFP *tfp = &hal_common->tfps[k];

        if (uid != NULL && tfp->uid != *uid) {
            continue;
        }

        if (port_id != NULL && tfp->spitfp->port_id != *port_id) {
            continue;
        }

        if (device_id != NULL && tfp->device_id != *device_id) {
            continue;
        }

        if (index_ptr != NULL) {
            *index_ptr = k + 1;
        }

        return tfp;
    }

    return NULL;
}
