#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef int s32;
typedef unsigned int u32;

/* the level's object-handle table: a pointer global holding the base of 0x100-byte entries */
typedef struct {
    u8 pad00[0x20];
    u8 f20;
    u8 f21;
    u8 pad22[0x68 - 0x22];
    void *f68;
    u8 pad6C[0x95 - 0x6C];
    u8 f95;
    u16 f96;
    u8 pad98[0xBC - 0x98];
    u8 fBC;
    u8 padBD[0x100 - 0xBD];
} Ent;

extern Ent *D_L08_001B29DC __attribute__((sda));

/* handle is valid and the entry's class byte is a live one */
s32 func_L08_003D8918(s32 a0)
{
    Ent *t;
    s32 i;

    if (a0 != -1) {
        t = D_L08_001B29DC;
        i = a0 << 8;
        return *(u8 *)((u8 *)t + i + 0x20) >= 3;
    }
    return 1;
}

/* handle is valid and the entry's class byte is a live one */
s32 func_L08_004229C8(s32 a0)
{
    Ent *t;
    s32 i;

    if (a0 != -1) {
        t = D_L08_001B29DC;
        i = a0 << 8;
        return *(u8 *)((u8 *)t + i + 0x20) >= 3;
    }
    return 1;
}

/* the entry's two faces are either unset or already gone */
s32 func_L08_003D8948(s32 a0)
{
    Ent *p;
    s32 r;

    if (a0 != -1) {
        p = D_L08_001B29DC + a0;
        r = 0;
        if (p == 0 || p->f20 == 0xFE || p->f20 == 0xFD) r = 1;
        return r;
    }
    return 1;
}

/* the entry's two faces are either unset or already gone */
s32 func_L08_003D8990(s32 a0)
{
    Ent *p;
    s32 r;

    if (a0 != -1) {
        p = D_L08_001B29DC + a0;
        r = 0;
        if (p == 0 || p->f20 == 0xFE || p->f20 == 0xFD) r = 1;
        return r;
    }
    return 1;
}

/* the entry's two faces are either unset or already gone */
s32 func_L08_004229F8(s32 a0)
{
    Ent *p;
    s32 r;

    if (a0 != -1) {
        p = D_L08_001B29DC + a0;
        r = 0;
        if (p == 0 || p->f20 == 0xFE || p->f20 == 0xFD) r = 1;
        return r;
    }
    return 1;
}

/* the entry's two faces are either unset or already gone */
s32 func_L08_00422A40(s32 a0)
{
    Ent *p;
    s32 r;

    if (a0 != -1) {
        p = D_L08_001B29DC + a0;
        r = 0;
        if (p == 0 || p->f20 == 0xFE || p->f20 == 0xFD) r = 1;
        return r;
    }
    return 1;
}

/* the entry is a live class or an unset one */
s32 func_L08_003D89D8(s32 a0)
{
    Ent *p;
    u32 b;
    s32 r;

    if (a0 != -1) {
        p = D_L08_001B29DC + a0;
        b = p->f20;
        r = 0;
        if (b >= 3u || p == 0 || b == 0xFE || b == 0xFD) r = 1;
        return r;
    }
    return 1;
}

/* the entry is a live class or an unset one */
s32 func_L08_00422A88(s32 a0)
{
    Ent *p;
    u32 b;
    s32 r;

    if (a0 != -1) {
        p = D_L08_001B29DC + a0;
        b = p->f20;
        r = 0;
        if (b >= 3u || p == 0 || b == 0xFE || b == 0xFD) r = 1;
        return r;
    }
    return 1;
}
