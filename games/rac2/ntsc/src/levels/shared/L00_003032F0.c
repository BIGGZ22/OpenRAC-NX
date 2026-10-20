#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef signed int s32;

typedef struct {
    u8 pad0[0x14];
    u16 *f14;
    u8 pad1[4];
    s32 f1C;
} TutorialState;

typedef struct {
    u8 pad0[0x3A];
    s16 f3A;
    u8 pad1[4];
} Rec;

extern TutorialState D_L00_001C9340;
extern Rec *D_L00_001B24D0[1] __attribute__((sda));
extern s32 D_L00_001B24D4[1] __attribute__((sda));

void func_L00_003032F0(void)
{
    Rec *p;
    s16 *q;
    s32 n;
    s32 count;
    TutorialState *st;

    if (D_L00_001C9340.f1C == 0) {
        n = D_L00_001B24D4[0];
        if (n <= 0)
            return;
        q = &D_L00_001B24D0[0]->f3A;
        do {
            *q = 0x7F80;
            q = (s16 *)((u8 *)q + 0x40);
            n--;
        } while (n != 0);
    } else {
        n = D_L00_001B24D4[0];
        if (n <= 0)
            return;
        p = D_L00_001B24D0[0];
        st = &D_L00_001C9340;
        count = n;
        do {
            p->f3A = st->f14[p->f3A];
            p = (Rec *)((u8 *)p + 0x40);
            count--;
        } while (count != 0);
    }
}
