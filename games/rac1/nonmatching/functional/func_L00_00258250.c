/* functional: medium. The NTSC decomp's pending C for the same instructions (a draft it has not matched
   yet), carried to PAL's names by OpenRAC's tools/port.py. Never compared with retail's bytes here. */
extern short D_0015EE84;
extern s32 D_0013D530[37];
extern u8 D_0013D5C8[35];
extern u8 D_0015EED0[];
extern s32 D_0014C290[][64];
typedef struct {
    u8 pad[0xE];
    u16 count;
    u8 tail[8];
} ItemCount_257218_58250;
extern ItemCount_257218_58250 D_L00_001C43B0[];
extern s32 func_002140B0(s32);

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/overlays/shared/rendering/00250df8.c, FUN_L00_00257218. */
int func_L00_00258250(u8 *m, s32 *item) {
    s32 shortfall = 0;
    s32 available = 0;
    s32 stocked = 0;
    s32 i;
    s32 choice;
    s32 scan_index;
    u32 id;
    u32 selected_item;
    u8 *entry;

    if (*item <= 0 || (m != 0 &&
        ((D_0014C290[(*(s32 *)&D_0015EE84)][(s16)*(u16 *)(m + 0xB2) >> 5] >>
          (*(u16 *)(m + 0xB2) & 31)) & 1))) {
        {
            scan_index = 0;
            if (D_0015EED0[0] != 0xFF) {
                entry = D_0015EED0;
                while (1) {
                    id = *entry & 0x3F;
                    if (D_0013D5C8[id]) {
                        ItemCount_257218_58250 *count = &D_L00_001C43B0[id];
                        available++;
                        if (count->count != 0) {
                            stocked++;
                        }
                        shortfall += D_0013D530[id] < count->count;
                    }
                    entry++;
                    if (++scan_index >= 12 || *entry == 0xFF) break;
                }
            }
        }
        /* Retail also uses list index -1 when the random selection is negative. */
        i = -1;
        if (shortfall != 0) {
            scan_index = func_002140B0(shortfall);
            if (scan_index >= 0) {
                i = -1;
                while (1) {
                    ++i;
                    id = D_0015EED0[i] & 0x3F;
                    if (D_0013D5C8[id]) {
                        ItemCount_257218_58250 *count = &D_L00_001C43B0[id];
                        scan_index -= D_0013D530[id] < count->count;
                    }
                    if (scan_index < 0) break;
                }
            }
            selected_item = D_0015EED0[i] & 0x3F;
        } else {
            selected_item = 10;
            if (available != 0) {
                scan_index = func_002140B0(stocked);
                if (scan_index >= 0) {
                    i = -1;
                    while (1) {
                        i++;
                        id = D_0015EED0[i] & 0x3F;
                        if (D_0013D5C8[id]) {
                            ItemCount_257218_58250 *count = &D_L00_001C43B0[id];
                            if (count->count != 0) scan_index--;
                        }
                        if (scan_index < 0) break;
                    }
                }
                selected_item = D_0015EED0[i] & 0x3F;
            }
        }
        *item = selected_item;
    }
    choice = 2;
    if (func_002140B0(5) != 0) choice = 1;
    return choice;
}
