#include "include_asm.h"

typedef unsigned char u8;

typedef unsigned short u16;

typedef short s16;

typedef int s32;

/* the selected state object, and the two candidate pointers beside it */
extern u8 *D_L23_001B1484 __attribute__((sda));

/* the same word read through an absolute access */
extern u8 *D_L23_001B1484_2;

extern s32 D_L23_001B1958 __attribute__((sda));

extern s32 D_L23_001B1954 __attribute__((sda));

extern s32 D_0018C0B0 __attribute__((nosda));

extern s32 D_L23_001B1430 __attribute__((sda));

extern s16 D_L23_001B1B94 __attribute__((sda));

extern s16 D_L23_001B1B96 __attribute__((sda));

extern void func_00291450(s32);

extern void func_L00_002E6CF0(void *, void *);

extern void func_L23_0042AD10(void *);

extern void func_L23_0042AD90(void *);

extern void func_L23_0042ADB0(void);

extern void func_L23_00429DC8(void *);

extern void func_L23_00429FF8(void *);

extern void func_L23_0042A8C0(void *);

extern void func_L23_0042A018(void *);

extern void func_L23_0042A438(void *);

extern s32 func_L23_0041B948(void);

extern s32 func_L23_0041C0C0(void);

extern s32 func_L23_0041C0B0(void);

extern float D_L23_001B3930 __attribute__((sda));

/* reports whether the current state object carries byte 0x20 equal to four */
s32 func_L23_0041E548(void) {
    u8 *state = D_L23_001B1484;

    if (state == 0) {
        return 0;
    }
    return state[0x20] == 4;
}

extern s32 D_L23_001B194C __attribute__((sda));

extern s32 D_L23_001B1950 __attribute__((sda));

/* two absolute level words: sized past the $gp limit so each store is a bare symbol through $at */
extern s32 D_L23_001A8AB0[4] __attribute__((sda));

extern s32 D_L23_001A8AC0[4] __attribute__((sda));

extern void func_002E59F8(s32 handle);

extern void func_0029F338(u8 *object);

/* releases the handle of the slot chosen by the key, then the object the slot holds */
void func_L23_00423588(s32 key, s32 flag) {
    s32 *slot;
    s32 handle;
    u8 *obj;
    u8 *v;

    if (key == D_0018C0B0) {
        handle = D_L23_001B194C;
        slot = &D_L23_001B1954;
        if (handle != -1) {
            func_002E59F8(handle);
            D_L23_001B194C = -1;
        }
    } else {
        handle = D_L23_001B1950;
        slot = &D_L23_001B1958;
        if (handle != -1) {
            func_002E59F8(handle);
            D_L23_001B1950 = -1;
        }
    }
    obj = (u8 *)*slot;
    if (obj != 0) {
        if (*(s16 *)(obj + 0xAA) != 0x1190) {
            *slot = 0;
        } else {
            if (flag == 0 && obj[0x20] != 0) {
                return;
            }
            v = *(u8 **)(obj + 0x68);
            D_L23_001A8AB0[0] = 0;
            D_L23_001A8AC0[0] = 0;
            if (*(s16 *)(v + 0x2C) != -1) {
                func_00291450(*(s16 *)(v + 0x2C));
            }
            func_0029F338((u8 *)*slot);
            *slot = 0;
        }
    }
}

/* returns one of the two pinned words, chosen by the measured key */
s32 func_L23_00423698(s32 arg0) {
    s32 *word = &D_L23_001B1958;

    if (arg0 == D_0018C0B0) {
        word = &D_L23_001B1954;
    }
    return *word;
}

/* releases both measured handles and clears them */
void func_L23_0042AC70(void) {
    s16 handle = D_L23_001B1B94;

    if (handle != -1) {
        func_00291450(handle);
        D_L23_001B1B94 = -1;
    }
    handle = D_L23_001B1B96;
    if (handle != -1) {
        func_00291450(handle);
        D_L23_001B1B96 = -1;
    }
}

/* releases the measured handles and seeds the frame */
void func_L23_00429CC0(u8 *arg0) {
    s32 *payload = *(s32 **)(arg0 + 0x68);

    func_L23_00429DC8(arg0);
    if (D_L23_001B1484_2 != 0) {
        if (func_L23_0041B948() != 0) {
            func_L00_002E6CF0((void *)func_L23_0042A018, arg0);
        } else if (func_L23_0041C0C0() != 0) {
            func_L00_002E6CF0((void *)func_L23_0042A438, arg0);
        } else if (func_L23_0041C0B0() != 0) {
            func_L23_0042A8C0(arg0);
        } else {
            payload[1] = 0;
        }
        if (func_L23_0041C0C0() == 0) {
            s16 handle = D_L23_001B1B94;

            if (handle != -1) {
                D_L23_001B3930 = 0.02f;
                func_00291450(handle);
                D_L23_001B1B94 = -1;
            }
        }
        if (func_L23_0041C0B0() == 0) {
            s16 handle = D_L23_001B1B96;

            if (handle != -1) {
                func_00291450(handle);
                D_L23_001B1B96 = -1;
            }
        }
    }
    func_L23_00429FF8(arg0);
}

/* refreshes the two counters around the measured callback */
void func_L23_0042ACC0(void *arg0) {
    func_L23_0042AD10(arg0);
    if (D_L23_001B1484_2 != 0) {
        if (D_L23_001B1430 != 0) {
            func_L00_002E6CF0((void *)func_L23_0042ADB0, arg0);
        }
    }
    func_L23_0042AD90(arg0);
}


extern s32 D_L23_001B3714;

extern s32 D_L23_001B13E8 __attribute__((sda));

extern void func_002E5CF8(int, int, u8 *);

/* publishes the boot stamp once, seeding the shared state first */
void func_L23_0041E570(void) {
    if (D_L23_001B3714 == 0) {
        u8 *state = D_L23_001B1484;

        if (state != 0) {
            func_002E5CF8(0x10, 0, state);
        }
    }
    D_L23_001B3714 = D_L23_001B13E8;
}

/* the three words at 0x1B17B0 are copied to the stack; the entries picked by the timing tests are chosen by a helper */
typedef struct {
    s32 v[3];
} L23Table3;

extern L23Table3 D_L23_001B17B0;

typedef struct {
    u8 pad[0x18];
    s32 f18;
    s32 f1C;
} L23Block1F90;

extern L23Block1F90 D_L23_001B1F90;

typedef struct {
    u8 pad[0x1874];
    s32 f1874;
} D00189E20_View;

extern D00189E20_View D_00189E20;

extern s32 func_002A77E0(s32);

/* collects the timing entries that apply and returns one of them at random (-1 when none does) */
s32 func_L23_0041E418(void) {
    u8 *obj = *(u8 **)(D_L23_001B1484 + 0x68);
    L23Table3 table = D_L23_001B17B0;
    s32 count = 0;

    if (D_00189E20.f1874 < *(s16 *)(obj + 0x2F4) - 5) {
        table.v[0] = 0;
        count = 1;
    }
    if (count == 0 || !(D_00189E20.f1874 < *(s16 *)(obj + 0x2F4) / 2)) {
        if (D_L23_001B1F90.f18 < *(s16 *)(obj + 0x2FA)) {
            table.v[count++] = 1;
        }
        if (D_L23_001B1F90.f1C < *(s16 *)(obj + 0x2FE)) {
            table.v[count++] = 2;
        }
    }
    if (count == 0) {
        return -1;
    }
    switch (func_002A77E0(count)) {
    case 0:
        return table.v[0];
    case 1:
        return table.v[1];
    case 2:
    default:
        return table.v[2];
    }
}

typedef int TI16 __attribute__((mode(TI)));

/* level words written as bare symbols through $at: each sized past the $gp limit */
extern s32 D_L23_001A8AB4[3] __attribute__((sda));

extern s32 D_L23_001A8AC4[3] __attribute__((sda));

extern long long D_L23_001A8AC8[2] __attribute__((sda));

extern long long D_L23_001A8AB8[2] __attribute__((sda));

/* a plain small extern: $gp in the delay slot reorg copies it into, absolute where it stays */
extern s32 D_L23_001A8FF0;

extern s16 func_00291308(TI16 *, s32, float, float);

extern u8 *func_0029EFD0(s32);

/* allocates the object, records it in the word chosen by the key and fills its fields and the level block */
u8 *func_L23_00423380(u8 *arg0, TI16 *arg1) {
    u8 *obj;
    u8 *payload;
    s32 *word;

    obj = func_0029EFD0(0x1190);
    word = &D_L23_001B1958;
    if (arg0 == (u8 *)D_0018C0B0) {
        word = &D_L23_001B1954;
    }
    if (obj != 0 && *word == 0) {
        *(u8 *)(obj + 0x30) = 0xFF;
        *(s16 *)(obj + 0x32) = 0xFF;
        *(u8 *)(obj + 0x31) = 1;
        *(u8 **)(obj + 0xB8) = arg0;
        *(u8 *)(obj + 0x20) = 0;
        payload = *(u8 **)(obj + 0x68);
        *(TI16 *)(obj + 0x10) = *arg1;
        *word = (s32)obj;
        D_L23_001A8AB4[0] = 0xFF;
        D_L23_001A8AC4[0] = 0x00FFFFFF;
        D_L23_001A8AC8[0] = 0x1F00000042LL;
        D_L23_001A8AB8[0] = 0;
        D_L23_001A8AB0[0] = 0;
        D_L23_001A8AC0[0] = 0;
        *(s32 *)(payload + 0x10) = 0;
        *(s16 *)(payload + 0x2C) = func_00291308(arg1, 0, 0.0f, 0.0f);
    }
    if (D_L23_001A8FF0 != 0) {
        *(u16 *)(obj + 0x34) = *(u16 *)(obj + 0x34) | 0x100;
        *(TI16 *)(obj + 0xC0) = *(TI16 *)(arg0 + 0xC0);
        *(TI16 *)(obj + 0xD0) = *(TI16 *)(arg0 + 0xD0);
        *(TI16 *)(obj + 0xE0) = *(TI16 *)(arg0 + 0xE0);
    }
    return obj;
}

extern s32 func_L00_0035F5A0(s32, s32, s32);

/* makes sure the word chosen by the flag holds an object, then moves it to the given position */
void func_L23_004234C0(u8 *arg0, TI16 *arg1, s32 arg2) {
    s32 *word = &D_L23_001B1958;
    TI16 local;
    TI16 *pos = &local;
    u8 *obj;
    u8 *payload;
    u8 *found;

    if (arg2 != 0) {
        word = &D_L23_001B1954;
    }
    local = *arg1;
    if (*word == 0) {
        func_L23_00423380(arg0, &local);
        if (arg2 != 0) {
            D_L23_001B194C = func_L00_0035F5A0(0x14, 0, 0);
        } else {
            D_L23_001B1950 = func_L00_0035F5A0(0x14, 0, 0);
        }
        found = (u8 *)*word;
        if (found == 0) {
            return;
        }
        obj = found;
        payload = *(u8 **)(obj + 0x68);
        if (*(s16 *)(obj + 0xAA) != 0x1190) {
            return;
        }
        *(s16 *)(payload + 0x40) = arg2;
        *(s32 *)(payload + 0x48) = 0;
        *(s32 *)(payload + 0x44) = 0;
    }
    obj = (u8 *)*word;
    *(TI16 *)(obj + 0x10) = *pos;
    payload = *(u8 **)(obj + 0x68);
    *(s16 *)(payload + 0x2E) = 1;
}

extern void func_002A7750(void *, s32, s32);

extern void func_L23_004278D8(u8 *);

/* the timer the second state copies into the sub-object */
extern float D_L23_001B36FC __attribute__((sda));

/* state 0 arms the draw distances and a 2000-tick timer and moves to state 1; state 1 copies the
   shared timer when it is set and moves to state 2; both then run the per-frame handler */
void func_L23_004277E8(u8 *moby) {
    u8 *vars = *(u8 **)(moby + 0x68);
    u8 state = moby[0x20];

    if (state == 0) {
        *(u16 *)(moby + 0x32) = 0xFF;
        moby[0x30] = 0xFF;
        *(u16 *)(moby + 0x34) = *(u16 *)(moby + 0x34) | 0x100;
        *(float *)(vars + 0x20) = 2000.0f;
        *(s16 *)(vars + 0x24) = 2000;
        *(u16 *)(moby + 0x7C) = 0x3FF;
        func_002A7750(moby, 1, 0);
        *(u16 *)(moby + 0x34) = *(u16 *)(moby + 0x34) | 0x4000;
    } else if (state == 1) {
        float t = D_L23_001B36FC;

        if (t != 0.0f) {
            *(float *)(vars + 0x20) = t;
            *(s16 *)(vars + 0x24) = (s16)t;
        }
        func_002A7750(moby, 2, 0);
    }
    func_L23_004278D8(moby);
}

extern s32 D_L23_001B19C4 __attribute__((sda));

extern s32 D_L23_001B19C8 __attribute__((sda));

extern float D_L23_001B19CC __attribute__((sda));

extern float D_L23_001B19D0 __attribute__((sda));

extern float D_L23_001B19D4 __attribute__((sda));

extern s32 D_L23_001B19D8 __attribute__((sda));

/* the three floats come before the two trailing words: that argument order is what keeps the
   two arms of the second call from sharing more than the jal */
extern void func_L23_00425AF8(void *obj, s32 a, s32 b, s32 c, float x, float y, float z, s32 d, s32 e);

/* draws the record twice: once plain (or with the override word when byte 0x2B is set), once scaled */
void func_L23_004259E0(u8 *a0) {
    u8 *obj = *(u8 **)(a0 + 0x68);

    if (obj[0x2A] == 0) {
        func_L23_00425AF8(obj + 0x30, obj[0x28], obj[0x29], 5,
                          *(float *)(obj + 0x10), *(float *)(obj + 0x14), *(float *)(obj + 0x18),
                          *(s32 *)obj, *(s32 *)(obj + 4));
        return;
    }
    if (obj[0x2B] != 0) {
        func_L23_00425AF8(obj + 0x30, obj[0x28], obj[0x29], 5,
                          *(float *)(obj + 0x10), *(float *)(obj + 0x14), *(float *)(obj + 0x18),
                          D_L23_001B19D8, D_L23_001B19D8);
    } else {
        func_L23_00425AF8(obj + 0x30, obj[0x28], obj[0x29], 5,
                          *(float *)(obj + 0x10), *(float *)(obj + 0x14), *(float *)(obj + 0x18),
                          *(s32 *)obj, *(s32 *)(obj + 4));
    }
    func_L23_00425AF8(obj + 0x30, obj[0x28] * D_L23_001B19C4, obj[0x29] * D_L23_001B19C8, 5,
                      *(float *)(obj + 0x10) * D_L23_001B19CC, *(float *)(obj + 0x14) * D_L23_001B19D0,
                      *(float *)(obj + 0x18) * D_L23_001B19D4, *(s32 *)obj, *(s32 *)(obj + 4));
}
