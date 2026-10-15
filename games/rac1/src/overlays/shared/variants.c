/* Functions of common level code added after the first split; stubs added by tools/overlay_variants.py, replaced by C as functions are matched. */
#include "common.h"
#include "include_asm.h"

typedef struct {
    char pad[0x24];
    int v;
    char pad2[0x24];
} Rec4C;
extern unsigned char D_0013E633[];
extern int func_L00_0020DB30(int);
extern Rec4C D_L15_0017A140[];

/* Returns 0 or 0x54 in the early-out cases below, otherwise the v field of D_L15_0017A140[func_L00_0020DB30(0)]. */
int func_L15_001FED10(int a) {
    char *g = (char *)D_0013E633 + 0xE1D;
    switch (*(unsigned char *)(g + 0x20A4)) {
    case 2:
        return 0;
    case 3:
        return 0;
    }
    {
        char *g2 = (char *)D_0013E633 + 0xE1D;
        if (*(int *)(g2 + 0x22A8) == 1) {
            return 0x54;
        }
        if (a == 0) {
            return 0;
        }
        if (*(unsigned char *)(g2 + 0x20A8) == 0) {
            return 0;
        }
        if (*(unsigned char *)(g2 + 0x20AA) == 0) {
            return 0;
        }
        if (*(short *)(g2 + 0x22C8) != 0) {
            return 0;
        }
    }
    {
        int i = func_L00_0020DB30(0);
        return D_L15_0017A140[i].v;
    }
}
