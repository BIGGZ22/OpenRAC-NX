#include "common.h"
#include "structs.h"

typedef struct VoBuf {
    char *frames;        /* 0x0, ring of 0xD0000-byte frames */
    char *data;          /* 0x4, ring of 0x138C0-byte entries */
    volatile int wr;     /* 0x8, shared ring state */
    volatile int count;  /* 0xC, shared ring state */
    int cap;            /* 0x10 */
} VoBuf;

/* Updates or queries the decoded-frame ring. */
void func_0023E560(VoBuf *vb, char *frames, char *entries, int capacity) {
    vb->count = 0;
    vb->frames = frames;
    vb->data = entries;
    vb->cap = capacity;
    vb->wr = 0;
    if (capacity > 0) {
        int offset = 0;
        do {
            *(int *)(offset + (unsigned int)vb->data) = 0;
            capacity--;
            offset += 0x138C0;
        } while (capacity != 0);
    }
}
/* voBufDelete(VoBuf *) -- nothing to free. */
void func_0023E5B0(VoBuf *vb) {
}
/* voBufReset(VoBuf *) */
void func_0023E5B8(volatile int *arg0) {
    arg0[3] = 0;
    arg0[2] = 0;
}
/* voBufIsFull(VoBuf *) */
int func_0023E5C8(VoBuf *vb) {
    return vb->count == vb->cap;
}
/* Updates or queries the decoded-frame ring. */
extern int func_0011D960(void);
extern void func_0011D9A8(void);
/* voBufIncCount(VoBuf *) */
void func_0023E5E0(VoBuf *vb) {
    func_0011D960();
    *(int *)(vb->data + vb->wr * 0x138C0) = 2;
    vb->count++;
    vb->wr = (vb->wr + 1) % vb->cap;
    func_0011D9A8();
}
/* voBufGetData(VoBuf *) */
char *func_0023E658(VoBuf *vb) {
    if (voBufIsFull(vb)) {
        return 0;
    }
    return *(char **)vb + vb->wr * 0xD0000;
}
/* voBufIsEmpty -- true when the entry count is zero. */
int func_0023E698(VoBuf *vb) {
    return vb->count == 0;
}
/* Returns the current ring entry when the corresponding count test permits it. */
char *func_0023E6A8(VoBuf *vb) {
    if (voBufIsEmpty(vb)) return 0;
    return vb->data + ((vb->wr - vb->count + vb->cap) % vb->cap) * 0x138C0;
}
/* voBufDecCount(VoBuf *) */
void func_0023E710(volatile int *vb) {
    if (vb[3] > 0) {
        vb[3]--;
    }
}
