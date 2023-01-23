/*
 * Copyright (C) 2020 Erik Fleckstein <erik@tinkerforge.com>
 *
 * Redistribution and use in source and binary forms of this file,
 * with or without modification, are permitted. See the Creative
 * Commons Zero (CC0 1.0) License for more details.
 */

#include "streaming.h"
#include "stdbool.h"

// We want to do the pointer arithmetic on typed pointers, as calculating on void * is a GNU extension.
// Also write the memcpy as a loop, any optimizing compiler should understand this.

#define COPY_ITEMS_IMPL(type_) void tf_copy_items_##type_(void *TF_RESTRICT dest, size_t dest_offset, const void *TF_RESTRICT src, size_t src_offset, size_t item_count) { \
    type_ *d = ((type_ *)dest) + dest_offset; \
    if (src == NULL) { \
        for(size_t i = 0; i < item_count; ++i) { \
            *d = 0; \
            ++d; \
        } \
        return; \
    } \
    const type_ *TF_RESTRICT s = ((const type_ *TF_RESTRICT )src) + src_offset; \
    for(size_t i = 0; i < item_count; ++i) { \
        *d = *s; \
        ++d; \
        ++s; \
    } \
}

COPY_ITEMS_IMPL(uint8_t)
COPY_ITEMS_IMPL(uint16_t)
COPY_ITEMS_IMPL(uint32_t)
COPY_ITEMS_IMPL(uint64_t)
COPY_ITEMS_IMPL(int8_t)
COPY_ITEMS_IMPL(int16_t)
COPY_ITEMS_IMPL(int32_t)
COPY_ITEMS_IMPL(int64_t)
COPY_ITEMS_IMPL(bool)
COPY_ITEMS_IMPL(char)
COPY_ITEMS_IMPL(float)

int tf_stream_out(void *device, TF_LowLevelStreamOut ll_function, void *wrapper_data, void *ret_stream,
                  uint32_t *ret_stream_read, void *TF_RESTRICT chunk_data, uint32_t max_chunk_length, TF_CopyItemFunction copy_fn)
{
    int ret = TF_E_OK;

    uint32_t stream_length = 0;
    bool stream_out_of_sync = false;

    uint32_t chunk_offset = 0;
    uint32_t chunk_length = 0;

    *ret_stream_read = 0;

    ret = ll_function(device, wrapper_data, &stream_length, &chunk_offset, chunk_data);

    if (ret == TF_E_INTERNAL_STREAM_HAS_NO_DATA) {
        return TF_E_OK;
    }

    if (ret != TF_E_OK) {
        return ret;
    }

    stream_out_of_sync = chunk_offset != 0;

    if (!stream_out_of_sync) {
        chunk_length = stream_length - chunk_offset;

        if (chunk_length > max_chunk_length) {
            chunk_length = max_chunk_length;
        }

        if (ret_stream != NULL) {
            copy_fn(ret_stream, 0, chunk_data, 0, chunk_length);
        }

        *ret_stream_read += chunk_length; // This could just be = as we've written 0 beforehand. However += looks more in line with the following code.

        while (*ret_stream_read < stream_length) {
            ret = ll_function(device, wrapper_data, &stream_length, &chunk_offset, chunk_data);

            if (ret != TF_E_OK) {
                return ret;
            }

            stream_out_of_sync = chunk_offset != *ret_stream_read;

            if (stream_out_of_sync) {
                break;
            }

            chunk_length = stream_length - chunk_offset;

            if (chunk_length > max_chunk_length) {
                chunk_length = max_chunk_length;
            }

            if (ret_stream != NULL) {
                copy_fn(ret_stream, *ret_stream_read, chunk_data, 0, chunk_length);
            }

            *ret_stream_read += chunk_length;
        }
    }

    if (stream_out_of_sync) {
        *ret_stream_read = 0; // return empty array

        // discard remaining stream to bring it back in-sync
        while (chunk_offset + max_chunk_length < stream_length) {
            ret = ll_function(device, wrapper_data, &stream_length, &chunk_offset, chunk_data);

            if (ret != TF_E_OK) {
                return ret;
            }
        }

        ret = TF_E_STREAM_OUT_OF_SYNC;
    }

    return ret;
}

int tf_stream_in(void *device, TF_LowLevelStreamIn ll_function, void *wrapper_data, const void *TF_RESTRICT stream, uint32_t stream_length,
                 void *TF_RESTRICT chunk_data, uint32_t *ret_stream_written, uint32_t max_chunk_length, TF_CopyItemFunction copy_fn)
{
    int ret = TF_E_OK;
    uint32_t chunk_offset = 0;
    uint32_t chunk_length = 0;
    uint32_t chunk_written = 0;

    if (ret_stream_written != NULL) {
        *ret_stream_written = 0;
    }

    // If the stream length is 0, we still have to send one chunk (filled with zeroes) to transmit potential extra parameters.
    if (stream_length == 0) {
        copy_fn(chunk_data, 0, NULL, 0, max_chunk_length);

        ret = ll_function(device, wrapper_data, stream_length, chunk_offset, chunk_data, &chunk_written);

        if (ret_stream_written != NULL) {
            *ret_stream_written += chunk_written;
        }
        return ret;
    }

    // Setting stream to null is only allowed if stream_length is 0 (handled above). Return error otherwise.
    if (stream == NULL) {
        return TF_E_NULL;
    }

    while (chunk_offset < stream_length) {
        chunk_length = stream_length - chunk_offset;

        if (chunk_length > max_chunk_length) {
            chunk_length = max_chunk_length;
        }

        copy_fn(chunk_data, 0, stream, chunk_offset, chunk_length);
        copy_fn(chunk_data, chunk_length, NULL, 0, max_chunk_length - chunk_length);

        ret = ll_function(device, wrapper_data, stream_length, chunk_offset, chunk_data, &chunk_written);

        if (ret != TF_E_OK) {
            if (ret_stream_written != NULL) {
                *ret_stream_written = 0;
            }

            break;
        }

        if (ret_stream_written != NULL) {
            *ret_stream_written += chunk_written;
        }

        if (chunk_written < max_chunk_length) {
            break;  // either last chunk or short write
        }

        chunk_offset += max_chunk_length;
    }

    return ret;
}

bool tf_stream_out_callback(TF_HighLevelCallback *hlc, uint32_t stream_length, uint32_t chunk_offset, void *chunk_data, uint32_t max_chunk_length, TF_CopyItemFunction copy_fn)
{
    uint32_t chunk_length = stream_length - chunk_offset;
    if (chunk_length > max_chunk_length)
        chunk_length = max_chunk_length;

    if (!hlc->stream_in_progress) {  // no stream in progress
        if (chunk_offset == 0) {  // stream starts
            hlc->stream_in_progress = true;

            if (hlc->data != NULL) {
                copy_fn(hlc->data, 0, chunk_data, 0, chunk_length);
            }

            hlc->length = chunk_length;

            if (hlc->length >= stream_length) {
                return true;
            }
        } else {
            // ignore tail of current stream, wait for next stream start
        }
    } else { // stream in progress
        if (chunk_offset != hlc->length) { // stream out of sync
            hlc->stream_in_progress = false;
            hlc->length = 0;

            return true;
        } else { //stream in sync
            if (hlc->data != NULL) {
                copy_fn(hlc->data, hlc->length, chunk_data, 0, chunk_length);
            }

            hlc->length += chunk_length;

            if (hlc->length >= stream_length) {
                return true;
            }
        }
    }

    return false;
}
