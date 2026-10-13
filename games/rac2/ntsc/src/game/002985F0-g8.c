#include "include_asm.h"

/* Measured partial layout; original type and game roles are unknown. */
typedef struct Rac2Native_170b399a2496b87e_LeafSelect7State11Layout {
    unsigned char unknown_00[0x15c];
    int mode_15c;
    int unknown_160;
    int selected_164;
    int value_168;
} Rac2Native_170b399a2496b87e_LeafSelect7State11Layout;

extern Rac2Native_170b399a2496b87e_LeafSelect7State11Layout D_001393E0;
extern int D_001A7420;

/* When mode at data offset 0x15C equals 2 and the signed selection at offset 0x164 is negative, write selection 7, clear the word at offset 0x168 and write absolute state value 11. */
void func_002985F0(void)
{
    if (D_001393E0.mode_15c == 2 &&
        D_001393E0.selected_164 < 0) {
        D_001393E0.selected_164 = 7;
        D_001393E0.value_168 = 0;
        D_001A7420 = 11;
    }
}
