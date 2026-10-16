#include "common.h"
#include "structs.h"

/* Three metadata words following the 0x50000-byte data ring. */
typedef struct ReadBufState {
    int wr;
    int fill;
    int capacity;
} ReadBufState;

/* Maintains the data ring and its three trailing metadata words. */
void func_0023CD10(char *buffer) {
    ReadBufState *state = (ReadBufState *)(buffer + 0x50000);
    state->capacity = 0x50000;
    state->wr = 0;
    state->fill = 0;
}
/* readBufDelete(ReadBuf *) -- nothing to free. */
void func_0023CD28(void *rb) {
}
/* Maintains the data ring and its three trailing metadata words. */
int func_0023CD30(char *buffer, char **out) {
    ReadBufState *state = (ReadBufState *)(buffer + 0x50000);
    int free = state->capacity - state->fill;
    if (free) *out = buffer + state->wr;
    return free;
}
/* Maintains the data ring and its three trailing metadata words. */
int func_0023CD60(char *buffer, int count) {
    ReadBufState *state = (ReadBufState *)(buffer + 0x50000);
    int capacity = state->capacity;
    int fill = state->fill;
    int write = state->wr;
    int used = capacity - fill;
    if (count < used) used = count;
    state->wr = (write + used) % capacity;
    state->fill = fill + used;
    return used;
}
/* Maintains the data ring and its three trailing metadata words. */
int func_0023CDA8(char *buffer, char **out) {
    ReadBufState *state = (ReadBufState *)(buffer + 0x50000);
    if (state->fill) *out = buffer + ((state->wr - state->fill + state->capacity) % state->capacity);
    return state->fill;
}
/* Consumes at most the available bytes from the data ring. */
int func_0023CDF0(char *buffer, int count) {
    ReadBufState *state = (ReadBufState *)(buffer + 0x50000);
    int used = state->fill;
    int fill = used;
    if (used > count) used = count;
    state->fill = fill - used;
    return used;
}
