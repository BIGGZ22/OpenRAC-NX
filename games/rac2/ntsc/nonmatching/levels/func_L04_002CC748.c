/* func_L04_002CC748, 144 bytes, 2 places, src/levels/shared/L04_002CC748.c (-O2 -G0).
   ABANDON - the address of D_00189E20 is materialised differently.
   Read in full: `if (a0 != 0 && D_00189E20.b22BA != 0 && D_00189E20.h24EC == 0)` returns the
   weapon-record id at `*(int *)(0x27B470 + D_00139568[func_L00_002ADEA0(0)] * 0xE0 + 0x28)`,
   else `0x51` when `D_00189E20.w24CC == 1` and `0`/`0x67` from `D_00189E20.b1492`.
   The guard, the block order, the `movz` tail and the record lookup all match (best candidate
   152 bytes against 144, this file is the 148-byte variant of the same body). What is left is
   one compiler choice: retail keeps the `$hi` of D_00189E20 in `$a1` from the entry block into
   the shared tail, so the tail is a bare `addiu $a0,$a1,-0x61E0` (three instructions for two
   sites); this cc1 gives the full `lui`/`addiu` to one register and re-materialises a fresh
   `lui` in the tail (four instructions) - or, with the guard and the tail on two different C
   spellings of the same address, emits two independent `lui`/`addiu` pairs. Measured: one
   pointer local for both halves, one local each half, the global object twice, the two-name
   `__asm__` alias (see alternatives), `-mno-split-addresses`, goto/if-else/early-return tail
   shapes. What would unblock it: a source shape that keeps the symbol's `high` part live
   across the guard's two branches while the `lo_sum` is re-issued per site. */

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
typedef float f32;

typedef struct {
    u8 pad00[0x7A8];
    f32 field7A8;
    f32 field7AC;
    f32 field7B0;
    f32 field7B4;
    u8 pad7B8[0x1492 - 0x7B8];
    u8 b1492;
    u8 pad1493[0x2290 - 0x1493];
    u32 field2290;
    u32 field2294;
    u8 pad2298[0x22BA - 0x2298];
    u8 b22BA;
    u8 pad22BB[0x24CC - 0x22BB];
    s32 w24CC;
    u8 pad24D0[0x24EC - 0x24D0];
    s16 h24EC;
} D00189E20F;

extern D00189E20F D_00189E20F;

extern char D_00139568[];
extern char D_L04_0027B470[];
extern int func_L00_002ADEA0(int);

int func_L04_002CC748(int a0) {
    int r;

    if (a0 != 0 && D_00189E20F.b22BA != 0 && D_00189E20F.h24EC == 0)
        return *(int *)((int)D_L04_0027B470 +
                        *(u8 *)(func_L00_002ADEA0(0) + (int)D_00139568) * 0xE0 + 0x28);
    else {
        r = 0x67;
        if (D_00189E20F.w24CC == 1)
            r = 0x51;
        else if (D_00189E20F.b1492 == 0)
            r = 0;
    }
    return r;
}
