#include "include_asm.h"

/* Measured partial layout; original type and game roles are unknown. */
typedef struct Rac2Native_87e90c235703f6b4_LeafSelect9State15Layout {
    unsigned char unknown_00[0x15c];
    int mode_15c;
    int unknown_160;
    int selected_164;
    int value_168;
} Rac2Native_87e90c235703f6b4_LeafSelect9State15Layout;

extern Rac2Native_87e90c235703f6b4_LeafSelect9State15Layout D_001393E0_1;
extern int D_001A7420_5;

/* When mode at data offset 0x15C equals 2 and the signed selection at offset 0x164 is negative, write selection 9, clear the word at offset 0x168 and write absolute state value 15. */
void func_00298780(void)
{
    if (D_001393E0_1.mode_15c == 2 &&
        D_001393E0_1.selected_164 < 0) {
        D_001393E0_1.selected_164 = 9;
        D_001393E0_1.value_168 = 0;
        D_001A7420_5 = 15;
    }
}

extern int D_001A7424 __attribute__((sda));
extern int D_001A7424_1[] __attribute__((sda));

void func_002987C0(void)
{
    if (D_001393E0_1.mode_15c == 2 && D_001393E0_1.selected_164 < 0) {
        if (*(int *)((char *)&D_001393E0_1 + 0x16C) != 0) {
            D_001A7420_5 = 18;
            D_001A7424 = D_001A7424_1[0] | 0x40;
        } else {
            D_001393E0_1.selected_164 = 7;
            *(int *)((char *)&D_001393E0_1 + 0x148) = 0;
            *(short *)((char *)&D_001393E0_1 + 0x18) = 0;
            D_001393E0_1.value_168 = 0;
            D_001A7420_5 = 22;
        }
    }
}
