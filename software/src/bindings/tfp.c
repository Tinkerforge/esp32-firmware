/*
 * Copyright (C) 2020 Erik Fleckstein <erik@tinkerforge.com>
 *
 * Redistribution and use in source and binary forms of this file,
 * with or without modification, are permitted. See the Creative
 * Commons Zero (CC0 1.0) License for more details.
 */

#include "tfp.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#include "endian_convert.h"
#include "packet_buffer.h"
#include "hal_common.h"
#include "errors.h"

// Time in µs to sleep if there is no data available
#define TF_TFP_SLEEP_TIME_US 250

static uint8_t tf_tfp_build_header(TF_TFP *tfp, uint8_t *header_buf, uint8_t length, uint8_t function_id, bool response_expected) {
    TF_TFPHeader header;

    memset(&header, 0, sizeof(TF_TFPHeader));

    uint8_t sequence_number = tfp->next_sequence_number & 0x0F;

    if (sequence_number == 0) {
        sequence_number = 1;
    }

    tfp->next_sequence_number = sequence_number + 1;

    header.uid_num = tfp->uid_num;
    header.length = length;
    header.fid = function_id;
    header.seq_num = sequence_number;
    header.response_expected = response_expected;

    tf_tfp_header_write(&header, header_buf);

    return sequence_number;
}

static bool tf_tfp_dispatch_packet(TF_TFP *tfp, TF_TFPHeader *header, TF_PacketBuffer *packet) {
    TF_HALCommon *hal_common = tf_hal_get_common(tfp->spitfp->hal);

#if TF_NET_ENABLE != 0
    if (hal_common->net != NULL) {
        // The network layer expects a complete copy of the TFP packet (i.e. with header),
        // however we've already removed the header. Write it back here.
        uint8_t net_buf[TF_TFP_MAX_MESSAGE_LENGTH] = {0};

        tf_tfp_header_write(header, net_buf);
        tf_packet_buffer_peek_offset_n(packet, net_buf + 8, header->length - 8, 0);
        tf_net_send_packet(hal_common->net, header, net_buf);
    }
#endif

    // We received a non-callback packet that was not expected from the source TFP.
    // As all getter and setter calls of the uC bindings (i.e. of other TFPs) are blocking,
    // we know immediately that the packet is only for the network layer, or is a late response to
    // a timed out request.
    if (header->seq_num != 0) {
        tf_packet_buffer_remove(packet, header->length - 8);
        return true;
    }

    // Handle enumerate callbacks
    if (header->fid == 253) {
        tf_hal_enumerate_handler(tfp->spitfp->hal, tfp->spitfp->port_id, packet);
        return true;
    }

    // Search TFP for the received callback
    bool result = false;
    TF_TFP *other_tfp = tf_hal_get_tfp(tfp->spitfp->hal, &header->uid_num, NULL, NULL, false);

    if (other_tfp != NULL) {
        result = other_tfp->cb_handler(other_tfp->device, header->fid, packet);
    }

    if (!result && hal_common->net != NULL) {
        // We didn't find another TFP or it had no handler registered to this callback.
        // However we dispatched the callback to the network.
        // Remove the packet and report success.
        tf_packet_buffer_remove(packet, header->length - 8);
        return true;
    }

    return result;
}

static bool tf_tfp_filter_received_packet(TF_TFP *tfp, bool remove_interesting, uint8_t *error_code, uint8_t *length) {
    TF_PacketBuffer *buf = &tfp->spitfp->recv_buf;
    uint8_t used = tf_packet_buffer_get_used(buf);

    if (used < 8) {
        tf_hal_log_debug("Too short!\n");
        tf_packet_buffer_remove(buf, used);
        ++tfp->error_count_frame;
        return false;
    }

    TF_TFPHeader header;
    tf_tfp_header_read(&header, buf);

    // Compare with <= as behind the tfp packet there has to be the SPITFP checksum
    if (used <= header.length) {
        tf_hal_log_debug("Too short! Used (%d) < header.length (%d)\n", used, header.length);
        tf_packet_buffer_remove(buf, used);
        ++tfp->error_count_frame;
        return false;
    }

    // Patch "position" of enumerate and get_identity packets if "connected_uid" is
    // just a null-terminator I.e. the device is attached to us directly
    if ((header.fid == 253 && header.length == 34) || (header.fid == 255 && header.length == 33)) {
        char connected_uid_first_char;

        tf_packet_buffer_peek_offset(buf, (uint8_t *)&connected_uid_first_char, 8);

        if (connected_uid_first_char == '\0') {
#if TF_LOCAL_ENABLE != 0
            TF_Local *local = tf_hal_get_local(tfp->spitfp->hal, NULL, NULL, NULL, false);

            if (local != NULL) {
                for (uint8_t i = 0; i < sizeof(local->uid_str); ++i) {
                    tf_packet_buffer_poke_offset(buf, local->uid_str[i], 8 + i);
                }

                tf_packet_buffer_poke_offset(buf, '\0', 8 + sizeof(local->uid_str));
            } else {
                tf_packet_buffer_poke_offset(buf, '0', 8);
                tf_packet_buffer_poke_offset(buf, '\0', 9);
            }
#else
            tf_packet_buffer_poke_offset(buf, '0', 8);
            tf_packet_buffer_poke_offset(buf, '\0', 9);
#endif
            tf_packet_buffer_poke_offset(buf, (uint8_t)tf_hal_get_port_name(tfp->spitfp->hal, tfp->spitfp->port_id), 16);
        }
    }

    // We could do this before parsing the header, but in this order it's possible to remove the unwanted packet from the buffer.
    bool packet_uninteresting = (tfp->waiting_for_fid == 0)
                             || (tfp->uid_num != 0 && header.uid_num != tfp->uid_num)
                             || (header.fid != tfp->waiting_for_fid)
                             || (header.seq_num != tfp->waiting_for_seq_num);

    if (packet_uninteresting) {
        if (!tf_tfp_dispatch_packet(tfp, &header, buf)) {
            tf_hal_log_debug("Remove unexpected\n");

            if (tfp->waiting_for_fid == 0) {
                tf_hal_log_debug("tfp->waiting_for_fid == 0\n");
            }

            if (tfp->uid_num != 0 && header.uid_num != tfp->uid_num) {
                tf_hal_log_debug("tfp->uid_num != 0 && header.uid_num (%d) != tfp->uid_num (%d)\n", header.uid_num, tfp->uid_num);
            }

            if (header.fid != tfp->waiting_for_fid) {
                tf_hal_log_debug("header.fid (%d) != tfp->waiting_for_fid (%d)\n", header.fid, tfp->waiting_for_fid);
            }


            if (header.seq_num != tfp->waiting_for_seq_num) {
                tf_hal_log_debug("header.seq_num (%d) != tfp->waiting_for_seq_num (%d)\n", header.seq_num, tfp->waiting_for_seq_num);
            }

            tf_packet_buffer_remove(buf, header.length);

            if (header.seq_num != 0) {
                ++tfp->error_count_unexpected;
            }
        }

        tf_tfp_packet_processed(tfp);

        return false;
    }

    if (remove_interesting) {
        tf_hal_log_debug("Remove interesting\n");
        tf_packet_buffer_remove(buf, header.length);
        tf_tfp_packet_processed(tfp);
        ++tfp->error_count_unexpected;
        return false;
    }

    *error_code = header.error_code;
    *length = header.length - 8; // - 8 as we've already removed the header

    return true;
}

void tf_tfp_packet_processed(TF_TFP *tfp) {
    tf_spitfp_packet_processed(tfp->spitfp);
}

static bool empty_cb_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    (void)device;
    (void)fid;
    (void)payload;

    return false;
}

void tf_tfp_create(TF_TFP *tfp, uint32_t uid_num, uint16_t device_id, TF_SPITFP *spitfp) {
    memset(tfp, 0, sizeof(TF_TFP));

    tfp->spitfp = spitfp;
    tfp->uid_num = uid_num;
    tfp->device_id = device_id;
    tfp->next_sequence_number = 1;
    tfp->cb_handler = empty_cb_handler;
}

void tf_tfp_prepare_send(TF_TFP *tfp, uint8_t fid, uint8_t payload_size, bool response_expected) {
    // TODO: theoretically, all bytes should be rewritten when sending a new packet, so this is not necessary.
    uint8_t *buf = tf_spitfp_get_send_payload_buffer(tfp->spitfp);
    memset(buf, 0, TF_TFP_MAX_MESSAGE_LENGTH);

    uint8_t tf_tfp_seq_num = tf_tfp_build_header(tfp, buf, payload_size + TF_TFP_MIN_MESSAGE_LENGTH, fid, response_expected);

    if (response_expected) {
        tfp->waiting_for_fid = fid;
        tfp->waiting_for_seq_num = tf_tfp_seq_num;
    } else {
        tfp->waiting_for_fid = 0;
        tfp->waiting_for_seq_num = 0;
    }
}

uint8_t *tf_tfp_get_send_payload_buffer(TF_TFP *tfp) {
    return tf_spitfp_get_send_payload_buffer(tfp->spitfp) + TF_TFP_MIN_MESSAGE_LENGTH;
}

TF_PacketBuffer *tf_tfp_get_receive_buffer(TF_TFP *tfp) {
    return tf_spitfp_get_receive_buffer(tfp->spitfp);
}

void tf_tfp_inject_packet(TF_TFP *tfp, TF_TFPHeader *header, uint8_t *packet) {
    uint8_t *buf = tf_spitfp_get_send_payload_buffer(tfp->spitfp);

    memset(buf, 0, TF_TFP_MAX_MESSAGE_LENGTH);
    memcpy(buf, packet, header->length);

    tfp->waiting_for_fid = 0;
    tfp->waiting_for_seq_num = 0;
}

static int tf_tfp_send_getter(TF_TFP *tfp, uint32_t deadline_us, uint8_t *error_code, uint8_t *length, int8_t seq_num) {
    tf_spitfp_build_packet(tfp->spitfp, seq_num);

    int result = TF_TICK_AGAIN;
    bool packet_received = false;
    uint32_t last_send = tf_hal_current_time_us(tfp->spitfp->hal);

    while (!tf_hal_deadline_elapsed(tfp->spitfp->hal, deadline_us) && !packet_received) {
        if (result & TF_TICK_TIMEOUT && tf_hal_deadline_elapsed(tfp->spitfp->hal, last_send + 5000)) {
            last_send = tf_hal_current_time_us(tfp->spitfp->hal);
            tf_spitfp_build_packet(tfp->spitfp, TF_RETRANSMISSION);
        }

        result = tf_spitfp_tick(tfp->spitfp, deadline_us);

        if (result < 0) {
            return result;
        }

        if (result & TF_TICK_PACKET_RECEIVED) {
            if (tf_tfp_filter_received_packet(tfp, false, error_code, length)) {
                tfp->waiting_for_fid = 0;
                tfp->waiting_for_seq_num = 0;
                packet_received = true;
            }
        }

        if (result & TF_TICK_SLEEP) {
            tf_hal_sleep_us(tfp->spitfp->hal, TF_TFP_SLEEP_TIME_US);
        }
    }

    return (packet_received ? TF_TICK_PACKET_RECEIVED : TF_TICK_TIMEOUT) | (result & TF_TICK_AGAIN) | (result & TF_TICK_IN_TRANSCEIVE);
}

static int tf_tfp_send_setter(TF_TFP *tfp, uint32_t deadline_us, int8_t seq_num) {
    tf_spitfp_build_packet(tfp->spitfp, seq_num);

    int result = TF_TICK_AGAIN;
    bool packet_sent = false;

    while (!tf_hal_deadline_elapsed(tfp->spitfp->hal, deadline_us) && !packet_sent) {
        if (result & TF_TICK_TIMEOUT) {
            tf_spitfp_build_packet(tfp->spitfp, TF_RETRANSMISSION);
        }

        result = tf_spitfp_tick(tfp->spitfp, deadline_us);

        if (result < 0) {
            return result;
        }

        if (result & TF_TICK_PACKET_RECEIVED) {
            uint8_t error_code, length;
            tf_tfp_filter_received_packet(tfp, true, &error_code, &length);
        }

        if (result & TF_TICK_PACKET_SENT) {
            packet_sent = true;
        }

        if (result & TF_TICK_SLEEP) {
            tf_hal_sleep_us(tfp->spitfp->hal, TF_TFP_SLEEP_TIME_US);
        }
    }

    return (packet_sent ? TF_TICK_PACKET_SENT : TF_TICK_TIMEOUT) | (result & TF_TICK_AGAIN) | (result & TF_TICK_IN_TRANSCEIVE);
}

int tf_tfp_send_packet(TF_TFP *tfp, bool response_expected, uint32_t deadline_us, uint8_t *error_code, uint8_t *length, int8_t seq_num) {
    return response_expected ? tf_tfp_send_getter(tfp, deadline_us, error_code, length, seq_num) : tf_tfp_send_setter(tfp, deadline_us, seq_num);
}

int tf_tfp_finish_send(TF_TFP *tfp, int previous_result, uint32_t deadline_us) {
    int result = previous_result;

    while ((!tf_hal_deadline_elapsed(tfp->spitfp->hal, deadline_us) || result & TF_TICK_IN_TRANSCEIVE) && (result & TF_TICK_AGAIN)) {
        result = tf_spitfp_tick(tfp->spitfp, deadline_us);

        if (result < 0) {
            return result;
        }
    }

    // Prevent sending the packet again for example in the callback_tick.
    // Note that we don't have to do this before the while loop, as we only
    // tick SPITFP iff previous_result has TF_TICK_AGAIN set, i.e. we are
    // in the middle of a transmission.
    // SPITFP would only resend the packet after returning once without
    // the TF_TICK_AGAIN flag set.
    tfp->spitfp->send_buf[0] = 0;

    // Also make sure we are not waiting for a packet anymore (in case of timeout).
    tfp->waiting_for_fid = 0;
    tfp->waiting_for_seq_num = 0;

    return (result & TF_TICK_AGAIN) ? TF_E_TIMEOUT : 0;
}

int tf_tfp_get_error(uint8_t error_code) {
    switch (error_code) {
        case 1:
            return TF_E_INVALID_PARAMETER;

        case 2:
            return TF_E_NOT_SUPPORTED;

        case 3:
            return TF_E_UNKNOWN_ERROR_CODE;

        case 0:
        default:
            return TF_E_OK;
    }
}

int tf_tfp_callback_tick(TF_TFP *tfp, uint32_t deadline_us) {
    int result = TF_TICK_AGAIN;

    if (tfp->spitfp->send_buf[0] != 0) {
        tf_spitfp_build_packet(tfp->spitfp, TF_RETRANSMISSION);
    }

    do {
        if (result & TF_TICK_SLEEP) {
            tf_hal_sleep_us(tfp->spitfp->hal, TF_TFP_SLEEP_TIME_US);
        }

        result = tf_spitfp_tick(tfp->spitfp, deadline_us);

        if (result < 0) {
            return result;
        }

        if (result & TF_TICK_PACKET_RECEIVED) {
            // handle possible callback packet
            uint8_t error_code, length;
            tf_tfp_filter_received_packet(tfp, true, &error_code, &length);
        }

        if (result & TF_TICK_PACKET_SENT) {
            tfp->spitfp->send_buf[0] = 0;
        }

        // Allow the state machine to run a bit over the deadline:
        // Result will in the worst case not contain TICK_AGAIN when
        // the state machine has just sent an ACK. The then
        // received 3 bytes should not contain a complete packet.
        // (Except an ACK that has not be acked again).
    } while (!tf_hal_deadline_elapsed(tfp->spitfp->hal, deadline_us) || (result & TF_TICK_AGAIN));

    return TF_E_OK;
}
