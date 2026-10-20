/* from src/levels/shared/L00_002E5918.c: slot allocator over D_L00_001BCC90 (32 slots
 * of 0x30): index >= 0 is used as-is, index < 0 takes the last slot whose f1C has the
 * 0x5 bits clear, -1 when none is free; D_L00_001B1998 (symbol_addrs:
 * g_rainHeightmapHeightScale) is bumped and the slot filled, index returned.
 * Best of 7 compiles: exact size (160 bytes) and every instruction shape (array-form
 * loop over D[i].f1C, `unsigned short` params keep the four `andi 0xFFFF`), but this
 * cc1 gives D's %hi to $a2 and the field pointer to $a0 where retail keeps $a0 (live
 * across the loop, reused by the second materialisation) and $a1, so the a2/a3
 * params move to $t5/$t6 instead of staying in place and retail's second G store
 * takes $at. Unblocked by the source shape that ties a2/a3 in place. */
typedef struct {
    char pad_00[0x10];
    int f10;
    int f14;
    int f18;
    int f1C;
    unsigned short f20;
    unsigned short f22;
    unsigned short f24;
    unsigned short f26;
    char pad_28[8];
} Slot_r002EA668;

extern Slot_r002EA668 D_L00_001BCC90[];
extern int D_L00_001B1998;

int func_L00_002EA668(int index, unsigned short a1, unsigned short a2, unsigned short a3,
                      unsigned short a4, int a5, int a6) {
    Slot_r002EA668 *p;
    int i;
    if (index < 0) {
        for (i = 0; i < 0x20; i++) {
            if ((D_L00_001BCC90[i].f1C & 5) == 0)
                index = i;
        }
        if (index < 0)
            return -1;
    }
    p = &D_L00_001BCC90[index];
    D_L00_001B1998++;
    p->f10 = a6;
    p->f20 = a1;
    p->f22 = a2;
    p->f24 = a3;
    p->f26 = a4;
    p->f14 = a5;
    p->f1C = 5;
    return index;
}
