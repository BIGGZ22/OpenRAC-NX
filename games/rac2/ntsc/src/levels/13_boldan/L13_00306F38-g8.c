#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef short s16;

typedef struct Node {
    u8 pad0[0x28];
    struct Node *next;
    u8 pad2[0x34 - 0x2C];
    u16 flags;
    u8 pad3[0x98 - 0x36];
    u32 f98;
    u8 pad4[0xAA - 0x9C];
    s16 fAA;
} Node;

extern int D_L13_001A8F00 __attribute__((sda));
extern int D_001BD5F0 __attribute__((nosda));
extern Node *D_L13_001B2928[1] __attribute__((sda));

/* state 0 starts the effect, state 1 flags every live node of four ids */
void func_L13_0038C328(u8 *a0) {
    Node *p;

    switch (a0[0x20]) {
    case 0:
        a0[0x30] = 0xFF;
        a0[0x20] = 1;
        break;
    case 1:
        if (D_L13_001A8F00 == 2 && D_001BD5F0 == 3) {
            for (p = D_L13_001B2928[0]; p != 0; p = p->next) {
                s16 k = p->fAA;

                if (k == 0x1F4 || k == 0xEA3 || k == 0xEA4 || k == 0x11EB) {
                    p->f98 = 0;
                    p->flags = (p->flags | 0x41) & 0xEFFF;
                }
            }
        }
        break;
    }
}
