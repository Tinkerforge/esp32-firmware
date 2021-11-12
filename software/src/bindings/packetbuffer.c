/*
 * Copyright (C) 2020 Erik Fleckstein <erik@tinkerforge.com>
 *
 * Redistribution and use in source and binary forms of this file,
 * with or without modification, are permitted. See the Creative
 * Commons Zero (CC0 1.0) License for more details.
 */

#include "packetbuffer.h"

#include <stdio.h>
#include <string.h>

#include "macros.h"
#include "endian_convert.h"


uint8_t tf_packetbuffer_get_size(TF_Packetbuffer *pb) {
    (void)pb;
    return TF_PACKET_BUFFER_SIZE - 1;
}

uint8_t tf_packetbuffer_get_used(TF_Packetbuffer *pb) {
    if(pb->end < pb->start) {
        return TF_PACKET_BUFFER_SIZE + pb->end - pb->start;
    }

    return pb->end - pb->start;
}

uint8_t tf_packetbuffer_get_free(TF_Packetbuffer *pb) {
    const uint8_t free = tf_packetbuffer_get_size(pb) - tf_packetbuffer_get_used(pb);

    return free;
}

bool tf_packetbuffer_is_empty(TF_Packetbuffer *pb) {
    return tf_packetbuffer_get_used(pb) == 0;
}

bool tf_packetbuffer_is_full(TF_Packetbuffer *pb) {
    return tf_packetbuffer_get_free(pb) == 0;
}

bool tf_packetbuffer_push(TF_Packetbuffer *pb, const uint8_t data) {
    uint8_t write_idx = pb->end;

    pb->end++;
    if(pb->end >= TF_PACKET_BUFFER_SIZE) {
        pb->end = 0;
    }

    if(pb->end == pb->start) {
        if(pb->end == 0) {
            pb->end = TF_PACKET_BUFFER_SIZE-1;
        } else {
            pb->end--;
        }
        return false;
    }

    pb->buffer[write_idx] = data;
    return true;
}

void tf_packetbuffer_remove(TF_Packetbuffer *pb, const uint8_t num) {
    // Make sure that we don't remove more then is available in the buffer
    uint8_t incr = MIN(tf_packetbuffer_get_used(pb), num);

    pb->start += incr;
    if(pb->start >= TF_PACKET_BUFFER_SIZE) {
        pb->start -= TF_PACKET_BUFFER_SIZE;
    }
}

bool tf_packetbuffer_pop(TF_Packetbuffer *pb, uint8_t *data) {
    //Silence Wmaybe-uninitialized in the _read_[type] functions.
    *data = 0;
    if (!tf_packetbuffer_peek(pb, data))
        return false;

    tf_packetbuffer_remove(pb, 1);
    return true;
}

bool tf_packetbuffer_peek(TF_Packetbuffer *pb, uint8_t *data) {
    if(tf_packetbuffer_is_empty(pb)) {
        return false;
    }

    *data = pb->buffer[pb->start];
    return true;
}

bool tf_packetbuffer_peek_offset(TF_Packetbuffer *pb, uint8_t *data, uint8_t offset) {
    if(tf_packetbuffer_get_used(pb) <= offset) {
        return false;
    }

    if (pb->start + offset >= TF_PACKET_BUFFER_SIZE) {
        *data = pb->buffer[pb->start + offset - TF_PACKET_BUFFER_SIZE];
    } else {
        *data = pb->buffer[pb->start + offset];
    }

    return true;
}

void tf_packetbuffer_create(TF_Packetbuffer *pb) {
    memset(pb->buffer, 0, TF_PACKET_BUFFER_SIZE);
    pb->start         = 0;
    pb->end           = 0;
}

void tf_packetbuffer_print(TF_Packetbuffer *pb) {
    int32_t end = pb->end - pb->start;
    uint8_t i;

    if(end < 0) {
        end += TF_PACKET_BUFFER_SIZE;
    }

    printf("TF_Packetbuffer (start %u, end %u, size %u): [\n",
           pb->start, pb->end, TF_PACKET_BUFFER_SIZE);

    for(i = 0; i < end; i++) {
        if((i % 16) == 0) {
            printf("    ");
        }

        printf("%x, ", pb->buffer[(pb->start + i) % TF_PACKET_BUFFER_SIZE]);

        if((i % 16) == 15) {
            printf("\n");
        }
    }

    printf("]\n");
}

bool tf_packetbuffer_free_array_view(TF_Packetbuffer *pb, uint8_t length, uint8_t **first_chunk, uint8_t *first_len, uint8_t **second_chunk, uint8_t *second_len) {
    if (length > tf_packetbuffer_get_free(pb))
        return false;

    bool wraps = (pb->end + length) >= TF_PACKET_BUFFER_SIZE;

    *first_chunk = pb->buffer + pb->end;
    if (wraps) {
        *first_len = TF_PACKET_BUFFER_SIZE - pb->end;
        *second_chunk = pb->buffer;
        *second_len = length - *first_len;
        pb->end = *second_len;
    } else {
        *first_len = length;
        *second_chunk = NULL;
        *second_len = 0;
        pb->end = pb->end + length;
    }

    return true;
}

void tf_packetbuffer_pop_n(TF_Packetbuffer *pb, uint8_t* dest, uint8_t count) {
    for(int i = 0; i < count; ++i) {
        tf_packetbuffer_pop(pb, dest + i);
    }
}

void tf_packetbuffer_peek_offset_n(TF_Packetbuffer *pb, uint8_t* dest, uint8_t count, uint8_t offset) {
    for(int i = 0; i < count; ++i) {
        tf_packetbuffer_peek_offset(pb, dest + i, (uint8_t)(offset + i));
    }
}

uint8_t tf_packetbuffer_read_uint8_t(TF_Packetbuffer *pb) {
    uint8_t result;
    tf_packetbuffer_pop(pb, &result);
    return result;
}

int8_t tf_packetbuffer_read_int8_t(TF_Packetbuffer *pb) {
    int8_t result;
    tf_packetbuffer_pop(pb, (uint8_t*)(&result));
    return result;
}

uint16_t tf_packetbuffer_read_uint16_t(TF_Packetbuffer *pb) {
    uint16_t result;
    tf_packetbuffer_pop_n(pb, (uint8_t*)(&result), sizeof(result));
    return tf_leconvert_uint16_from(result);
}

uint32_t tf_packetbuffer_read_uint32_t(TF_Packetbuffer *pb) {
    uint32_t result;
    tf_packetbuffer_pop_n(pb, (uint8_t*)(&result), sizeof(result));
    return tf_leconvert_uint32_from(result);
}

uint64_t tf_packetbuffer_read_uint64_t(TF_Packetbuffer *pb) {
    uint64_t result;
    tf_packetbuffer_pop_n(pb, (uint8_t*)(&result), sizeof(result));
    return tf_leconvert_uint64_from(result);
}

int16_t tf_packetbuffer_read_int16_t(TF_Packetbuffer *pb) {
    int16_t result;
    tf_packetbuffer_pop_n(pb, (uint8_t*)(&result), sizeof(result));
    return tf_leconvert_int16_from(result);
}

int32_t tf_packetbuffer_read_int32_t(TF_Packetbuffer *pb) {
    int32_t result;
    tf_packetbuffer_pop_n(pb, (uint8_t*)(&result), sizeof(result));
    return tf_leconvert_int32_from(result);
}

int64_t tf_packetbuffer_read_int64_t(TF_Packetbuffer *pb) {
    int64_t result;
    tf_packetbuffer_pop_n(pb, (uint8_t*)(&result), sizeof(result));
    return tf_leconvert_int64_from(result);
}

char tf_packetbuffer_read_char(TF_Packetbuffer *pb) {
    char result;
    tf_packetbuffer_pop_n(pb, (uint8_t*)(&result), sizeof(result));
    return result;
}

float tf_packetbuffer_read_float(TF_Packetbuffer *pb){
    float result;
    tf_packetbuffer_pop_n(pb, (uint8_t*)(&result), sizeof(result));
    return tf_leconvert_float_from(result);
}

bool tf_packetbuffer_read_bool(TF_Packetbuffer *pb) {
    bool result;
    tf_packetbuffer_pop_n(pb, (uint8_t*)(&result), sizeof(result));
    return result;
}

void tf_packetbuffer_read_bool_array(TF_Packetbuffer *pb, bool* dest, uint16_t count) {
    uint8_t byte = 0;
    for(int i = 0; i < count; ++i) {
        if (i % 8 == 0) {
            tf_packetbuffer_pop(pb, &byte);
        }
        dest[i] = (byte & (1 << (i % 8))) != 0;
    }
}

uint8_t tf_packetbuffer_peek_uint8_t(TF_Packetbuffer *pb, uint8_t offset) {
    uint8_t result = 0;
    tf_packetbuffer_peek_offset(pb, &result, offset);
    return result;
}

int8_t tf_packetbuffer_peek_int8_t(TF_Packetbuffer *pb, uint8_t offset) {
    int8_t result = 0;
    tf_packetbuffer_peek_offset(pb, (uint8_t*)(&result), offset);
    return result;
}

uint16_t tf_packetbuffer_peek_uint16_t(TF_Packetbuffer *pb, uint8_t offset) {
    uint16_t result = 0;
    tf_packetbuffer_peek_offset_n(pb, (uint8_t*)(&result), sizeof(result), offset);
    return tf_leconvert_uint16_from(result);
}

uint32_t tf_packetbuffer_peek_uint32_t(TF_Packetbuffer *pb, uint8_t offset) {
    uint32_t result = 0;
    tf_packetbuffer_peek_offset_n(pb, (uint8_t*)(&result), sizeof(result), offset);
    return tf_leconvert_uint32_from(result);
}

uint64_t tf_packetbuffer_peek_uint64_t(TF_Packetbuffer *pb, uint8_t offset) {
    uint64_t result = 0;
    tf_packetbuffer_peek_offset_n(pb, (uint8_t*)(&result), sizeof(result), offset);
    return tf_leconvert_uint64_from(result);
}

int16_t tf_packetbuffer_peek_int16_t(TF_Packetbuffer *pb, uint8_t offset) {
    int16_t result = 0;
    tf_packetbuffer_peek_offset_n(pb, (uint8_t*)(&result), sizeof(result), offset);
    return tf_leconvert_int16_from(result);
}

int32_t tf_packetbuffer_peek_int32_t(TF_Packetbuffer *pb, uint8_t offset) {
    int32_t result = 0;
    tf_packetbuffer_peek_offset_n(pb, (uint8_t*)(&result), sizeof(result), offset);
    return tf_leconvert_int32_from(result);
}

int64_t tf_packetbuffer_peek_int64_t(TF_Packetbuffer *pb, uint8_t offset) {
    int64_t result = 0;
    tf_packetbuffer_peek_offset_n(pb, (uint8_t*)(&result), sizeof(result), offset);
    return tf_leconvert_int64_from(result);
}

char tf_packetbuffer_peek_char(TF_Packetbuffer *pb, uint8_t offset) {
    char result = 0;
    tf_packetbuffer_peek_offset_n(pb, (uint8_t*)(&result), sizeof(result), offset);
    return result;
}

float tf_packetbuffer_peek_float(TF_Packetbuffer *pb, uint8_t offset){
    float result = 0;
    tf_packetbuffer_peek_offset_n(pb, (uint8_t*)(&result), sizeof(result), offset);
    return tf_leconvert_float_from(result);
}

bool tf_packetbuffer_peek_bool(TF_Packetbuffer *pb, uint8_t offset) {
    bool result = false;
    tf_packetbuffer_peek_offset_n(pb, (uint8_t*)(&result), sizeof(result), offset);
    return result;
}

void tf_packetbuffer_peek_bool_array(TF_Packetbuffer *pb, bool* dest, uint16_t count, uint8_t offset) {
    uint8_t byte = 0;
    for(int i = 0; i < count; ++i) {
        if (i % 8 == 0) {
            tf_packetbuffer_peek_offset(pb, &byte, offset);
        }
        dest[i] = (byte & (1 << (i % 8))) != 0;
    }
}
