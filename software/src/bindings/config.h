/*
 * Copyright (C) 2020 Erik Fleckstein <erik@tinkerforge.com>
 *
 * Redistribution and use in source and binary forms of this file,
 * with or without modification, are permitted. See the Creative
 * Commons Zero (CC0 1.0) License for more details.
 */

#ifndef TF_CONFIG_H
#define TF_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

// This is the size of the inventory i.e. the mapping from
// UIDs to the ports under which they are reachable.
// Increase this if the HAL initialization returns TF_E_TOO_MANY_DEVICES.
// Decrease this if you don't need as much devices and want decrease the
// program size.
#ifndef TF_INVENTORY_SIZE
#define TF_INVENTORY_SIZE 10
#endif

// Set this to 0 to disable the callback implementation.
// This will reduce the program size.
#ifndef TF_IMPLEMENT_CALLBACKS
#define TF_IMPLEMENT_CALLBACKS 1
#endif

// Set this to a higher log level to reduce debug output and program size.
#ifndef TF_LOG_LEVEL
#define TF_LOG_LEVEL TF_LOG_LEVEL_INFO
#endif

// Set this to 0 to remove the tf_hal_strerror implementation.
// This will reduce the program size.
#ifndef TF_IMPLEMENT_STRERROR
#define TF_IMPLEMENT_STRERROR 1
#endif

// Set this to 0 to remove the tf_get_device_display_name implementation.
// This will reduce the program size.
#ifndef TF_IMPLEMENT_GET_DEVICE_DISPLAY_NAME
#define TF_IMPLEMENT_GET_DEVICE_DISPLAY_NAME 1
#endif

// Set this to 1 to enable network support.
// If a net context is then registered to the HAL,
// attached Bricklets are controllable via network.
#ifndef TF_NET_ENABLE
#define TF_NET_ENABLE 0
#endif

// This is the maximum number of open requests to keep track of in parallel.
// If this many requests are not fulfilled and another one is created by sending
// a packet with response_expected = true, the oldest request will be dropped.
// Note that this does not mean, that a later response to the dropped request will be
// dropped too: As only the mapping to where the request came from is missing, the response
// will be broadcast to all connected clients.
// NEVER set this to a value larger than 255, as an uint8_t is used to index into the
// open request array.
#ifndef TF_NET_MAX_OPEN_REQUEST_COUNT
#define TF_NET_MAX_OPEN_REQUEST_COUNT 10
#endif

// This is the maximum number of clients connected in parallel.
// If this number is reached, no new clients can connect until
// one of the connected client closes their connection.
#ifndef TF_NET_MAX_CLIENT_COUNT
#define TF_NET_MAX_CLIENT_COUNT 4
#endif

// Set this to 1 to enable local device support.
// If a local context is then registered to the HAL,
// the local device (representing the MCU this program
// is running on) is exposed as Brick.
#ifndef TF_LOCAL_ENABLE
#define TF_LOCAL_ENABLE 0
#endif

#ifdef __cplusplus
}
#endif

#endif
