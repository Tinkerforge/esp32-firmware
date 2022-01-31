/*
 * Copyright (C) 2020 Erik Fleckstein <erik@tinkerforge.com>
 *
 * Redistribution and use in source and binary forms of this file,
 * with or without modification, are permitted. See the Creative
 * Commons Zero (CC0 1.0) License for more details.
 */

#ifndef TF_HAL_COMMON_H
#define TF_HAL_COMMON_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#include "config.h"
#include "packet_buffer.h"
#include "macros.h"

#include "tfp_header.h"
#include "tfp.h"

#include "net_common.h"

#if TF_LOCAL_ENABLE != 0
#include "local.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

// This is a union to allow easy initialization in the TF_PORT macros.
// The spitfp will be initialized per port in tf_hal_common_prepare
typedef union TF_PortCommon {
    TF_SPITFP spitfp;
    uint8_t _to_init;
} TF_PortCommon;

typedef struct TF_HALCommon {
    TF_TFP tfps[TF_INVENTORY_SIZE];
    uint16_t tfps_order[TF_INVENTORY_SIZE];
    uint16_t tfps_used;

    uint16_t device_overflow_count;
    uint32_t timeout;

    bool locked;
    uint8_t port_count;
    uint16_t callback_tick_index;

    uint8_t empty_buf[TF_SPITFP_MAX_MESSAGE_LENGTH];

    TF_Net *net;

#if TF_LOCAL_ENABLE != 0
    TF_Local *local;
#endif
} TF_HALCommon;

typedef struct TF_HAL TF_HAL;

void tf_hal_set_timeout(TF_HAL *hal, uint32_t timeout_us) TF_ATTRIBUTE_NONNULL_ALL;
uint32_t tf_hal_get_timeout(TF_HAL *hal) TF_ATTRIBUTE_NONNULL_ALL;
int tf_hal_get_device_info(TF_HAL *hal, uint16_t index, char ret_uid_str[7], char *ret_port_name, uint16_t *ret_device_id) TF_ATTRIBUTE_NONNULL(1);
int tf_hal_callback_tick(TF_HAL *hal, uint32_t timeout_us) TF_ATTRIBUTE_NONNULL_ALL;
int tf_hal_tick(TF_HAL *hal, uint32_t timeout_us) TF_ATTRIBUTE_NONNULL_ALL;

bool tf_hal_deadline_elapsed(TF_HAL *hal, uint32_t deadline_us) TF_ATTRIBUTE_NONNULL_ALL;
int tf_hal_get_error_counters(TF_HAL *hal, char port_name, uint32_t *ret_spitfp_error_count_checksum, uint32_t *ret_spitfp_error_count_frame, uint32_t *ret_tfp_error_count_frame, uint32_t *ret_tfp_error_count_unexpected) TF_ATTRIBUTE_NONNULL(1);
TF_TFP *tf_hal_get_tfp(TF_HAL *hal, const uint32_t *uid_num, const uint8_t *port_id, const uint16_t *device_id, bool skip_already_in_use);
int tf_hal_get_attachable_tfp(TF_HAL *hal, TF_TFP **tfp_ptr, const char *uid_str_or_port_name, uint16_t device_id);

#if TF_LOCAL_ENABLE != 0
TF_Local *tf_hal_get_local(TF_HAL *hal, const uint32_t *uid_num, const char *position, const uint16_t *device_id, bool skip_already_in_use);
int tf_hal_get_attachable_local(TF_HAL *hal, TF_Local **local_ptr, const char *uid_str_or_position, uint16_t device_id);
#endif

#define TF_LOG_LEVEL_NONE 0
#define TF_LOG_LEVEL_ERROR 1
#define TF_LOG_LEVEL_INFO 2
#define TF_LOG_LEVEL_DEBUG 3

#if TF_LOG_LEVEL >= TF_LOG_LEVEL_ERROR
#define tf_hal_log_error(...) tf_hal_printf(__VA_ARGS__)
#else
#define tf_hal_log_error(...)
#endif

#if TF_LOG_LEVEL >= TF_LOG_LEVEL_INFO
#define tf_hal_log_info(...) tf_hal_printf(__VA_ARGS__)
#else
#define tf_hal_log_info(...)
#endif

#if TF_LOG_LEVEL >= TF_LOG_LEVEL_DEBUG
#define tf_hal_log_debug(...) tf_hal_printf(__VA_ARGS__)
#else
#define tf_hal_log_debug(...)
#endif

// Very minimalistic printf: no zero-padding, grouping, l-modifier or similar and no float.
// Newlines (\n) are translated to the platform specific newline character(s).
// To print fixed width integer types use a I[width] prefix, for example %I16x to print
// an uint16_t as hex. Supported are I8, I16, I32 and I64.
// Note that the prefix only specifies the width of the integer, not it's signedness
// (i.e. use %I.. even if you want to print an unsigned integer).
// The signedness is communicated as usual with %[prefix]d or %[prefix]u.
void tf_hal_printf(const char *format, ...) TF_ATTRIBUTE_NONNULL_ALL;

// To be used by HAL implementations
int tf_hal_common_create(TF_HAL *hal) TF_ATTRIBUTE_NONNULL_ALL TF_ATTRIBUTE_WARN_UNUSED_RESULT;
int tf_hal_common_prepare(TF_HAL *hal, uint8_t port_count, uint32_t port_discovery_timeout_us) TF_ATTRIBUTE_NONNULL_ALL TF_ATTRIBUTE_WARN_UNUSED_RESULT;
void tf_hal_enumerate_handler(TF_HAL *hal, uint8_t port_id, TF_PacketBuffer *payload) TF_ATTRIBUTE_NONNULL_ALL;
void tf_hal_set_net(TF_HAL *hal, TF_Net *net);
#if TF_LOCAL_ENABLE != 0
void tf_hal_set_local(TF_HAL *hal, TF_Local *local);
#endif

// BEGIN - To be implemented by the specific HAL

int tf_hal_chip_select(TF_HAL *hal, uint8_t port_id, bool enable) TF_ATTRIBUTE_NONNULL_ALL TF_ATTRIBUTE_WARN_UNUSED_RESULT;
int tf_hal_transceive(TF_HAL *hal, uint8_t port_id, const uint8_t *write_buffer, uint8_t *read_buffer, uint32_t length) TF_ATTRIBUTE_NONNULL_ALL TF_ATTRIBUTE_WARN_UNUSED_RESULT;
uint32_t tf_hal_current_time_us(TF_HAL *hal) TF_ATTRIBUTE_NONNULL_ALL;
void tf_hal_sleep_us(TF_HAL *hal, uint32_t us) TF_ATTRIBUTE_NONNULL_ALL;
TF_HALCommon *tf_hal_get_common(TF_HAL *hal) TF_ATTRIBUTE_NONNULL_ALL;
char tf_hal_get_port_name(TF_HAL *hal, uint8_t port_id) TF_ATTRIBUTE_NONNULL_ALL;
TF_PortCommon *tf_hal_get_port_common(TF_HAL *hal, uint8_t port_id) TF_ATTRIBUTE_NONNULL_ALL;

// These functions have to work without an initialized HAL to be able to report HAL initialization info/errors, so don't pass the handle here.
void tf_hal_log_message(const char *msg, size_t len) TF_ATTRIBUTE_NONNULL_ALL;
void tf_hal_log_newline(void);

#if TF_IMPLEMENT_STRERROR != 0
const char *tf_hal_strerror(int e_code);
#endif

// END - To be implemented by the specific HAL

#ifdef __cplusplus
}
#endif

#endif
