/*
 * Copyright (C) 2020 Erik Fleckstein <erik@tinkerforge.com>
 *
 * Redistribution and use in source and binary forms of this file,
 * with or without modification, are permitted. See the Creative
 * Commons Zero (CC0 1.0) License for more details.
 */

#ifndef TF_STREAMING_H
#define TF_STREAMING_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#include "errors.h"
#include "macros.h"

#ifdef __cplusplus
extern "C" {
#endif

//restrict is only available in C. If the files are explicitly compiled with a C++ compiler, we just drop the restrict modifier, possibly resulting in a performance decrease.
#ifdef __cplusplus
    #if defined(__clang__) || defined(__GNUC__)
        #define TF_RESTRICT __restrict__
    #else
        #define TF_RESTRICT
    #endif
#else
    #define TF_RESTRICT restrict
#endif

#define TF_COPY_ITEMS_DECL(type_) void tf_copy_items_##type_(void *TF_RESTRICT dest, size_t dest_offset, const void *TF_RESTRICT src, size_t src_offset, size_t item_count);

TF_COPY_ITEMS_DECL(uint8_t)
TF_COPY_ITEMS_DECL(uint16_t)
TF_COPY_ITEMS_DECL(uint32_t)
TF_COPY_ITEMS_DECL(uint64_t)
TF_COPY_ITEMS_DECL(int8_t)
TF_COPY_ITEMS_DECL(int16_t)
TF_COPY_ITEMS_DECL(int32_t)
TF_COPY_ITEMS_DECL(int64_t)
TF_COPY_ITEMS_DECL(bool)
TF_COPY_ITEMS_DECL(char)
TF_COPY_ITEMS_DECL(float)

typedef int (*TF_LowLevelStreamOut)(void *device, void *wrapper_data, uint32_t *ret_stream_length, uint32_t *ret_chunk_offset, void *ret_chunk_data);
typedef int (*TF_LowLevelStreamIn) (void *device, void *wrapper_data, uint32_t stream_length, uint32_t chunk_offset, void *chunk_data, uint32_t *ret_chunk_written);

typedef void (*TF_CopyItemFunction)(void *TF_RESTRICT dest, size_t dest_offset, const void *TF_RESTRICT src, size_t src_offset, size_t item_count);

typedef struct TF_HighLevelCallback {
    void *data;
    size_t length;
    bool stream_in_progress;
} TF_HighLevelCallback;

int tf_stream_out(void *device,
                 TF_LowLevelStreamOut ll_function, // Generated function wrapper to handle extra parameters etc.
                 void *wrapper_data, // Data passed to the ll_function
                 void *ret_stream, // Target to write the streamed data to
                 uint32_t *ret_stream_length, // How many _items_ where written
                 void *TF_RESTRICT chunk_data, // Scratch buffer used for one chunk of data. Has to be moved outside because we would have to use malloc etc. here. The calling function can pass a stack allocated array of fixed size.
                 uint32_t max_chunk_length, // Maximum chunk length in items
                 TF_CopyItemFunction copy_fn) TF_ATTRIBUTE_NONNULL(1,5,6) TF_ATTRIBUTE_WARN_UNUSED_RESULT;

int tf_stream_in(void *device,
                 TF_LowLevelStreamIn ll_function, // Generated function wrapper to handle extra parameters etc.
                 void *wrapper_data, // Data passed to the ll_function
                 const void *TF_RESTRICT stream, // Source to readthe streamed data from. May only be null iff stream_length is 0.
                 uint32_t stream_length, // How many _items_ shall be read from stream.
                 void *TF_RESTRICT chunk_data, // Scratch buffer used for one chunk of data. Has to be moved outside because we would have to use malloc etc. here. The calling function can pass a stack allocated array of fixed size.
                 uint32_t *ret_stream_written, // How many items where written. Only relevant for streams that can write short.
                 uint32_t max_chunk_length, // Maximum chunk length in items
                 TF_CopyItemFunction copy_fn) TF_ATTRIBUTE_NONNULL(1,6) TF_ATTRIBUTE_WARN_UNUSED_RESULT;

// Returns true to the wrapper if the stream is complete or out of sync. else false. Clean TF_HighLevelCallback struct when stream is complete!
bool tf_stream_out_callback(TF_HighLevelCallback *hlc,
                            uint32_t stream_length,
                            uint32_t chunk_offset,
                            void *chunk_data,
                            uint32_t max_chunk_length,
                            TF_CopyItemFunction copy_fn) TF_ATTRIBUTE_NONNULL_ALL TF_ATTRIBUTE_WARN_UNUSED_RESULT;
#ifdef __cplusplus
}
#endif

#endif
