/* scePadGetState — retail 0x01EA3FD8..0x01EA4050 (120 bytes).
 * libpad; the slot record sits at 0x01FD3650, stride 0x70 per port and
 * 0x1C per slot, `linked` at +0x10.
 */
typedef unsigned char u8;

typedef struct {
    void *data;     /* 0x00 */
    int unk04[3];   /* 0x04 */
    int linked;     /* 0x10 */
    int unk14[2];   /* 0x14 */
} PadSlot;          /* 0x1C */

extern PadSlot padInfo[][4]; /* 0x01FD3650 */

typedef struct {
    u8 pad[0x70];
    u8 state; /* 0x70 */
    u8 mode;  /* 0x71 */
    u8 pad2[0x0E];
} PadData; /* 0x80 */

extern void PadGetDataInternal(int port, int slot, PadData *data);

int scePadGetState(int port, int slot) {
    PadData data;

    if (padInfo[port][slot].linked == 0)
        return 0x63;
    PadGetDataInternal(port, slot, &data);
    if (data.state == 6 && data.mode == 2)
        return 5;
    return data.state;
}
