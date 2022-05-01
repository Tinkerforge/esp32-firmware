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
    char uid_str[8]; tf_packet_buffer_pop_n(payload, (uint8_t *)uid_str, 8);
    char connected_uid_str[8]; tf_packet_buffer_pop_n(payload, (uint8_t *)connected_uid_str, 8);
    tf_packet_buffer_remove(payload, 7); // Drop position, hardware version and firmware version
    uint16_t device_id = tf_packet_buffer_read_uint16_t(payload);
    tf_packet_buffer_remove(payload, 1); // Drop enumeration type

    if (hal_common->tfps_used >= sizeof(hal_common->tfps) / sizeof(hal_common->tfps[0])) {
        ++hal_common->device_overflow_count;
        return;
    }

    uint32_t uid_num;

    if (tf_base58_decode(uid_str, &uid_num) != TF_E_OK) {
        return;
    }

    if (tf_hal_get_tfp(hal, &uid_num, NULL, NULL, false) != NULL) {
        return; // Device already known
    }

    tf_hal_log_info("Found device %s of type %d at port %c\n", uid_str, device_id, tf_hal_get_port_name(hal, port_id));

    TF_PortCommon *port_common = tf_hal_get_port_common(hal, port_id);
    uint16_t index = hal_common->tfps_used;

    tf_tfp_create(&hal_common->tfps[hal_common->tfps_used], uid_num, device_id, &port_common->spitfp);

    // Sort by port ID first and by UID second
    for (uint16_t i = 0; i < hal_common->tfps_used; ++i) {
        TF_TFP *other_tfp = &hal_common->tfps[hal_common->tfps_order[i]];

        if (other_tfp->spitfp->port_id > port_id || (other_tfp->spitfp->port_id == port_id && other_tfp->uid_num > uid_num)) {
            index = i;
            break;
        }
    }

    if (index < hal_common->tfps_used) {
        memmove(&hal_common->tfps_order[index + 1],
                &hal_common->tfps_order[index],
                sizeof(hal_common->tfps_order[0]) * (hal_common->tfps_used - index));
    }

    hal_common->tfps_order[index] = hal_common->tfps_used;
    ++hal_common->tfps_used;
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

int tf_hal_get_device_info(TF_HAL *hal, uint16_t index, char ret_uid_str[7], char *ret_port_name, uint16_t *ret_device_id) {
    TF_HALCommon *hal_common = tf_hal_get_common(hal);

    if (index >= hal_common->tfps_used) {
        return TF_E_DEVICE_NOT_FOUND;
    }

    index = hal_common->tfps_order[index];

    if (ret_uid_str != NULL) {
        tf_base58_encode(hal_common->tfps[index].uid_num, ret_uid_str);
    }

    if (ret_port_name != NULL) {
        *ret_port_name = tf_hal_get_port_name(hal, hal_common->tfps[index].spitfp->port_id);
    }

    if (ret_port_name != NULL) {
        *ret_device_id = hal_common->tfps[index].device_id;
    }

    return TF_E_OK;
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
    .uid_num=0,
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
    uint8_t ignored_error_code;
    uint8_t ignored_length;
    int result;

    if (net != NULL) {
        tf_net_tick(net);

        for (uint16_t i = 0; i < hal_common->tfps_used; ++i) {
            if (hal_common->tfps[i].send_enumerate_request
             && hal_common->tfps[i].spitfp->send_buf[0] == 0) {
                tf_tfp_inject_packet(&hal_common->tfps[i], &enumerate_request_header, enumerate_request);
                // TODO: What timeout to use here? If decided, use return value to check for the timeout, maybe increase an error count
                result = tf_tfp_send_packet(&hal_common->tfps[i], false, deadline_us, &ignored_error_code, &ignored_length);

                if (result & TF_TICK_PACKET_SENT) {
                    hal_common->tfps[i].send_enumerate_request = false;
                }

                (void)! tf_tfp_finish_send(&hal_common->tfps[i], result, deadline_us); // ignore result for now: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66425#c34
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
            if (header.fid == 254 && header.uid_num == 0 && header.length == 8) {
                for (uint16_t i = 0; i < hal_common->tfps_used; ++i) {
                    hal_common->tfps[i].send_enumerate_request = true;
                }

#if TF_LOCAL_ENABLE != 0
                if (hal_common->local != NULL) {
                    hal_common->local->trigger_enumerate_callback = true;
                }
#endif

                tf_net_drop_packet(net, pid);

                continue;
            }

            bool device_found = false;
            bool dispatched = false;
            uint8_t buf[TF_TFP_MAX_MESSAGE_LENGTH] = {0};

            for (uint16_t i = 0; i < hal_common->tfps_used; ++i) {
                if (header.uid_num != hal_common->tfps[i].uid_num) {
                    continue;
                }

                device_found = true;

                // Intentionally don't use get_payload_buffer here: the payload buffer is at send_buf + SPITFP_HEADER_SIZE
                // But the "is the buffer filled" marker is just the SPITFP packet length, i.e. before the SPITFP payload.
                if (hal_common->tfps[i].spitfp->send_buf[0] != 0) {
                    // Send buffer is not empty.
                    continue;
                }

                tf_net_get_packet(net, pid, buf);
                tf_tfp_inject_packet(&hal_common->tfps[i], &header, buf);

                // TODO: What timeout to use here? If decided, use return value to check for the timeout, maybe increase an error count
                result = tf_tfp_send_packet(&hal_common->tfps[i], false, deadline_us, &ignored_error_code, &ignored_length);
                (void)! tf_tfp_finish_send(&hal_common->tfps[i], result, deadline_us); // ignore result for now: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66425#c34
                dispatched = true;

                break;
            }

#if TF_LOCAL_ENABLE != 0
            if (!device_found && hal_common->local != NULL && header.uid_num == hal_common->local->uid_num) {
                tf_net_get_packet(net, pid, buf);
                tf_local_inject_packet(hal_common->local, &header, buf);
                tf_local_transceive_packet(hal_common->local);

                uint8_t *recv_buf = tf_local_get_recv_buffer(hal_common->local);
                TF_TFPHeader header_;

                tf_tfp_header_peek_plain(&header_, recv_buf);
                tf_net_send_packet(net, &header_, recv_buf);

                device_found = true;
                dispatched = true;
            }
#endif

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
    TF_HALCommon *hal_common = tf_hal_get_common(hal);

#if TF_LOCAL_ENABLE != 0
    if (hal_common->local != NULL
     && tf_local_callback_tick(hal_common->local)
     && hal_common->net != NULL) {
        uint8_t *recv_buf = tf_local_get_recv_buffer(hal_common->local);
        TF_TFPHeader header;

        tf_tfp_header_peek_plain(&header, recv_buf);
        tf_net_send_packet(hal_common->net, &header, recv_buf);
    }
#endif

    if (hal_common->tfps_used == 0) {
        return TF_E_OK;
    }

    uint32_t deadline_us = tf_hal_current_time_us(hal) + timeout_us;
    uint16_t first_index = hal_common->callback_tick_index;
    TF_TFP *tfp = NULL;

    do {
        ++hal_common->callback_tick_index;

        if (hal_common->callback_tick_index >= hal_common->tfps_used) {
            hal_common->callback_tick_index -= hal_common->tfps_used;
        }

        tfp = &hal_common->tfps[hal_common->callback_tick_index];

        int result = tf_tfp_callback_tick(tfp, tf_hal_current_time_us(hal));

        if (result != TF_E_OK) {
            return result;
        }
    } while (first_index != hal_common->callback_tick_index && !tf_hal_deadline_elapsed(hal, deadline_us));

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

#if TF_LOCAL_ENABLE != 0

void tf_hal_set_local(TF_HAL *hal, TF_Local *local) {
    TF_HALCommon *hal_common = tf_hal_get_common(hal);

    hal_common->local = local;
}

#endif

TF_TFP *tf_hal_get_tfp(TF_HAL *hal, const uint32_t *uid_num, const uint8_t *port_id, const uint16_t *device_id, bool skip_already_in_use) {
    TF_HALCommon *hal_common = tf_hal_get_common(hal);

    for (uint16_t i = 0; i < hal_common->tfps_used; ++i) {
        TF_TFP *tfp = &hal_common->tfps[hal_common->tfps_order[i]];

        if (skip_already_in_use && tfp->device != NULL) {
            continue;
        }

        if (uid_num != NULL && tfp->uid_num != *uid_num) {
            continue;
        }

        if (port_id != NULL && tfp->spitfp->port_id != *port_id) {
            continue;
        }

        if (device_id != NULL && tfp->device_id != *device_id) {
            continue;
        }

        return tfp;
    }

    return NULL;
}

int tf_hal_get_attachable_tfp(TF_HAL *hal, TF_TFP **tfp_ptr, const char *uid_str_or_port_name, uint16_t device_id) {
    TF_HALCommon *hal_common = tf_hal_get_common(hal);

    *tfp_ptr = NULL;

    if (uid_str_or_port_name != NULL && *uid_str_or_port_name != '\0') {
        uint32_t uid_num = 0;
        int base58_rc = tf_base58_decode(uid_str_or_port_name, &uid_num);

        // Could be a UID
        if (base58_rc == TF_E_OK) {
            TF_TFP *tfp = tf_hal_get_tfp(hal, &uid_num, NULL, &device_id, true);

            if (tfp != NULL) {
                *tfp_ptr = tfp;

                return TF_E_OK;
            }

            // Could be wrong-device-type or already-in-use
            tfp = tf_hal_get_tfp(hal, &uid_num, NULL, NULL, false);

            if (tfp != NULL) {
                if (tfp->device_id != device_id) {
                    return TF_E_WRONG_DEVICE_TYPE;
                }

                return TF_E_DEVICE_ALREADY_IN_USE;
            }

            // Intentionally don't exit here and check port names
        }

        // Could be a port name
        if (*(uid_str_or_port_name + 1) == '\0') {
            bool known_port_name = false;

            for (uint8_t port_id = 0; port_id < hal_common->port_count; ++port_id) {
                char port_name = tf_hal_get_port_name(hal, port_id);

                if (*uid_str_or_port_name != port_name) {
                    continue;
                }

                known_port_name = true;

                TF_TFP *tfp = tf_hal_get_tfp(hal, NULL, &port_id, &device_id, true);

                if (tfp != NULL) {
                    *tfp_ptr = tfp;

                    return TF_E_OK;
                }

                // Could be already-in-use
                tfp = tf_hal_get_tfp(hal, NULL, &port_id, &device_id, false);

                if (tfp != NULL) {
                    return TF_E_DEVICE_ALREADY_IN_USE;
                }

                // Intentionally don't exit here, port names are not enforced to be unique
            }

            // Could be known port name, but no device of requested type is connected
            if (known_port_name) {
                return TF_E_DEVICE_NOT_FOUND;
            }

            // Intentionally don't exit here and report potential base58 error
        }

        // Not a port name, report base58 error
        if (base58_rc != TF_E_OK) {
            return base58_rc;
        }

        return TF_E_DEVICE_NOT_FOUND;
    } else {
        TF_TFP *tfp = tf_hal_get_tfp(hal, NULL, NULL, &device_id, true);

        if (tfp != NULL) {
            *tfp_ptr = tfp;

            return TF_E_OK;
        }

        // Could be already-in-use
        tfp = tf_hal_get_tfp(hal, NULL, NULL, &device_id, false);

        if (tfp != NULL) {
            return TF_E_DEVICE_ALREADY_IN_USE;
        }

        return TF_E_DEVICE_NOT_FOUND;
    }
}

#if TF_LOCAL_ENABLE != 0

TF_Local *tf_hal_get_local(TF_HAL *hal, const uint32_t *uid_num, const char *position, const uint16_t *device_id, bool skip_already_in_use) {
    TF_HALCommon *hal_common = tf_hal_get_common(hal);

    if (hal_common->local == NULL) {
        return NULL;
    }

    if (skip_already_in_use && hal_common->local->device != NULL) {
        return NULL;
    }

    if (uid_num != NULL && hal_common->local->uid_num != *uid_num) {
        return NULL;
    }

    if (position != NULL && hal_common->local->position != *position) {
        return NULL;
    }

    if (device_id != NULL && hal_common->local->device_id != *device_id) {
        return NULL;
    }

    return hal_common->local;
}

int tf_hal_get_attachable_local(TF_HAL *hal, TF_Local **local_ptr, const char *uid_str_or_position, uint16_t device_id) {
    *local_ptr = NULL;

    if (uid_str_or_position != NULL && *uid_str_or_position != '\0') {
        uint32_t uid_num = 0;
        int base58_rc = tf_base58_decode(uid_str_or_position, &uid_num);

        // Could be a UID
        if (base58_rc == TF_E_OK) {
            TF_Local *local = tf_hal_get_local(hal, &uid_num, NULL, &device_id, true);

            if (local != NULL) {
                *local_ptr = local;

                return TF_E_OK;
            }

            // Could be wrong-device-type or already-in-use
            local = tf_hal_get_local(hal, &uid_num, NULL, NULL, false);

            if (local != NULL) {
                if (local->device_id != device_id) {
                    return TF_E_WRONG_DEVICE_TYPE;
                }

                return TF_E_DEVICE_ALREADY_IN_USE;
            }

            // Intentionally don't exit here and check position
        }

        // Could be a position
        if (*(uid_str_or_position + 1) == '\0') {
            char position = *uid_str_or_position;
            TF_Local *local = tf_hal_get_local(hal, NULL, &position, &device_id, true);

            if (local != NULL) {
                *local_ptr = local;

                return TF_E_OK;
            }

            // Could be already-in-use
            local = tf_hal_get_local(hal, NULL, &position, &device_id, false);

            if (local != NULL) {
                return TF_E_DEVICE_ALREADY_IN_USE;
            }

            // Could be known position, but no device of requested type is connected
            local = tf_hal_get_local(hal, NULL, &position, NULL, false);

            if (local != NULL) {
                return TF_E_DEVICE_NOT_FOUND;
            }

            // Intentionally don't exit here and report potential base58 error
        }

        // Not a position, report base58 error
        if (base58_rc != TF_E_OK) {
            return base58_rc;
        }

        return TF_E_DEVICE_NOT_FOUND;
    } else {
        TF_Local *local = tf_hal_get_local(hal, NULL, NULL, &device_id, true);

        if (local != NULL) {
            *local_ptr = local;

            return TF_E_OK;
        }

        // Could be already-in-use
        local = tf_hal_get_local(hal, NULL, NULL, &device_id, false);

        if (local != NULL) {
            return TF_E_DEVICE_ALREADY_IN_USE;
        }

        return TF_E_DEVICE_NOT_FOUND;
    }
}

#endif
