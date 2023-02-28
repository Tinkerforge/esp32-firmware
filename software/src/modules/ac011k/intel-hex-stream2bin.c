/*
 * compile:
 * gcc -g -fsanitize=address intel-hex-stream2bin.c -o intel-hex-stream2bin
 * && codesign -s - -f --entitlements tmp.entitlements intel-hex-stream2bin
 *
 * run:
 * ./intel-hex-stream2bin < AC011K-AE-25_V1.2.460.hex > AC011K-AE-25_V1.2.460.bin
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#define MAX_RECORD_LEN 256
#define BUF_SIZE 2048

#include <stdio.h>

void hexdump(char *buffer, size_t length) {
#define WIDTH 16
    if(buffer == NULL) {
        return;
    }
    int i, j;
    //fprintf(stderr, "Hexdump of %zu bytes:\n", length);
    //fprintf(stderr, "------------------------\n");
    for (i = 0; i < length; i += WIDTH) {
        fprintf(stderr, "%04x: ", i);
        for (j = 0; j < WIDTH && i + j < length; j++) {
            fprintf(stderr, "%02x ", (unsigned char)buffer[i+j]);
        }
        for (; j < WIDTH; j++) {
            fprintf(stderr, "   ");
        }
        fprintf(stderr, "  ");
        for (j = 0; j < WIDTH && i + j < length; j++) {
            char c = buffer[i+j];
            fprintf(stderr, "%c", (c >= ' ' && c <= '~') ? c : '.');
        }
        fprintf(stderr, "\n");
    }
}


typedef struct {
    char *binOutput;            // Buffer for binary output data
    char *inputGlueBuf;         // Buffer for incomplete input records
    uint32_t inputReminderLen;  // length of not processed hex input in the glue buffer
    uint32_t binOutputLen;      // Length of already converted binary data in output buffer
    uint32_t totalBytesOut;     // Length of already converted binary data in total
    uint32_t lastAddr;          // address from the last decoded record (to find where padding is needed)
} parse_state;


int parse_record(parse_state *state, char *record) {
    uint8_t buf[MAX_RECORD_LEN];
    uint8_t checksum = 0;
    int i, count, record_data_len, type, addr;
    if(record[0] != ':') {
        return 0; // Invalid record format, but we just ignore it
    }
    count = sscanf(record, ":%2x%4x%2x", &record_data_len, &addr, &type);
    if(count != 3) {
        fprintf(stderr, "Invalid record format\n");
        return -1; // Invalid record format
    }
    checksum = record_data_len + (addr & 0xff) + (addr >> 8) + type;
    for(i = 0; i < record_data_len+1; i++) { // +1 because of the checksum field itself
        count = sscanf(record + 9 + i * 2, "%2hhx", &buf[i]);
        if(count != 1) {
            fprintf(stderr, "Invalid data format\n");
            return -1; // Invalid data format
        }
        checksum += buf[i];
    }
    if(checksum != 0) {
        fprintf(stderr, "Checksum (%x) error in: %s", checksum, record);
        return -1; // Checksum error
    }
    switch (type) {
        case 0: // Data record
            if(state->binOutput == NULL) {
                state->binOutput = malloc(BUF_SIZE / 2);
                if(state->binOutput == NULL) {
                    fprintf(stderr, "Error: Out of memory\n");
                    return -1;
                }
            }
            if(state->lastAddr != addr && addr != 0) {
                fprintf(stderr, "Padding needed from %x to %x\n", state->lastAddr, addr);
                // first flush the buffer
                if(state->binOutputLen > 0) {
                    fwrite(state->binOutput, 1, state->binOutputLen, stdout);
                    state->binOutputLen = 0;
                }
                // than the padding
                for(i = state->lastAddr; i < addr; i++) {
                    printf("%c", 0xff);
                    state->totalBytesOut++;
                }
            }
            for(i = 0; i < record_data_len; i++) {
                state->binOutput[state->binOutputLen++] = buf[i];
            }
            state->totalBytesOut += record_data_len;
            state->lastAddr = addr + record_data_len;
            break;
        case 1: // End-of-file record
            break;
        case 4: // Extended Linear Address Record
            /* count = sscanf(record + 9, "%4x", &addr); */
            /* if(count != 1) { */
            /*     fprintf(stderr, "Invalid Extended Linear Address format\n"); */
            /*     return -1; // Invalid data format */
            /* } */
            /* fprintf(stderr, "Extended Linear Address Record %c%c%c%c / total bytes: %d   addr: %d, %d\n", record[9], record[10], record[11], record[12], state->totalBytesOut, addr, state->lastAddr); */
            break;
        case 5: // Start address record
            // it's always :04000005080081313D in our case
            break;
        default:
            // Ignore all other record types
            fprintf(stderr, "WARN: Unsupported record type: %02x\n%s\n\n", type, record);
            return 0;
    }
    return 0;
}

int parse_input(parse_state *state, char *input, int len) {
    int ret = 0;
    char *start, *end;
    if(state->inputGlueBuf == NULL) {
        state->inputGlueBuf = malloc(MAX_RECORD_LEN);
        if(state->inputGlueBuf == NULL) {
            fprintf(stderr, "Error: Out of memory\n");
            return -1;
        }
    }
    if(state->inputReminderLen > 0) {
        // process the glue buffer (filled with the start of the newly arrived input)
        memcpy(state->inputGlueBuf + state->inputReminderLen, input, MAX_RECORD_LEN - state->inputReminderLen);
        state->inputGlueBuf[MAX_RECORD_LEN - 1] = 0;
        ret = parse_record(state, state->inputGlueBuf);
        if(ret != 0) {
            return ret;
        }
        state->inputReminderLen = 0;
        // move on to the rest of the input buffer
        start = (char*)memmem(input, len, "\n", 1); // look for first newline in input buffer
        if(start == NULL) {
            fprintf(stderr, "Error: no newline to end the last record\n");
            return -1;
        }
        start += 1; // move to first char after newline
    } else {
        // start processing all complete records in the buffer
        start = input;
    }
    end = input + len;
    while(start < end) {
        char *newline = (char*)memmem(start, end - start, "\n", 1);
        if(newline == NULL) {
            // we don't have a newline in the input buffer before it ends
            // copy input reminder to glue buffer, to have it when are called next time
            int reminder = input + len - start;
            if (reminder > MAX_RECORD_LEN) {
                fprintf(stderr, "Error: reminder of hex input does not fit into MAX_RECORD_LEN\n");
                return -1;
            }
            state->inputReminderLen = end - start;
            memcpy(state->inputGlueBuf, start, state->inputReminderLen);
            break; // Incomplete record, parse more input when called again
        }
        ret = parse_record(state, start);
        if(ret != 0) {
            return ret;
        }
        start = newline + 1;
    }
    // Output converted data to stdout in chunks
    if(state->binOutputLen > 0) {
        fwrite(state->binOutput, 1, state->binOutputLen, stdout);
        state->binOutputLen = 0;
    }
    return 0;
}

int main() {
    char input[BUF_SIZE] = {0};
    parse_state state = {0};

    // Parse input data from standard input
    while(1) {
        int len = read(STDIN_FILENO, input, sizeof(input)-1); // -1 to be sure the whole buffer is 0 terminated
        if(len == 0) {
            break; // End of input
        }
        int ret = parse_input(&state, input, len);
        if(ret < 0) {
            fprintf(stderr, "Error: Failed to parse input\n");
            return 1;
        }
    }

    fprintf(stderr, "Success: reached the end (%d bytes).\n", state.totalBytesOut);

    // Clean up
    free(state.inputGlueBuf);
    free(state.binOutput);

    return 0;
}

