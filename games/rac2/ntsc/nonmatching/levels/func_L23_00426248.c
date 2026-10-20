/* func_L23_00426248 (900 bytes, 1 place: 23:00426248) -- NEAR MISS, not integrated.
   This C compiles to 880 bytes against retail's 900 (-g8 unit, -O2 -G8): twenty bytes short;
   148 of retail's 225 words match in place, and everything after the local quad copy (the
   probe call, the 0.04f / 48.0f / 0.075f branches, the facing-angle test, the shared
   epilogue) is retail's code instruction for instruction (registers apart).
   What it is: a spawner with a free-slot table. `D_L23_001B19FC` (float) counts the live
   spawns and is reset to 0 when it holds -1.0f (clearing the five slot words of
   D_L23_001B3918); it returns 0 at 5 or more, else claims the first free slot, allocates a
   0x11DF object, copies the position quad (p0) to +0x10 and p1's three vector quads (+0xC0,
   +0xD0, +0xE0), stamps the sub-object from the integer arguments (a2..a7 in registers, a8..a12
   on the stack at 0x90..0xB0) and the float, probes with func_00283DB8(&copy, 128.0f)
   and, on success, tunes the sub-object against D_L23_001BA640 (= D_L23_001BA500 + 0x140).
   What differs:
   1. the five 16-byte copies. Retail: `addiu $a0,$s3,0xC0 ; addiu $v1,$s4,0xC0 ; lq $v0,0($v1) ;
      sq $v0,0($a0)` and the same for +0xD0 and +0xE0, plus `lq $v0,0($s6) ; sq $v0,0($s0)`
      with s0 = obj+0x10 (+0x144) and `lq $v0,0($s0) ; sq $v0,0($sp)` for the local: pointers
      always materialised, the pairs in strict source order. This cc1 folds the
      offsets (`lq $v0,0xC0($s1) ; sq $v0,0xC0($s3)`), so the six `addiu` of the three vector
      pairs are missing and the neighbouring stores are free to interleave. (See func_L23_004280D8 /
      func_L23_00429DC8 for the same shape; an inline-asm qcopy macro explains it, cc1's own
      TImode move does not.)
   2. which incoming arguments are spilled: retail keeps a3, t0, t1 in $s5, $s7, $fp and spills a2,
      t2, t3 to 0x20 / 0x24 / 0x28($sp) (`sw $a2,0x20($sp)` ...); here a3 is spilled and t2 kept
      in $fp. The payload stores that read them are scheduled in a different order.
   3. the order of the stores `swc1 $f2,8($s2)` / `swc1 $f1,0xC($s2)` (2f, 4f) (retail +0x1E8).
   Measured: `0.45378566f` is retail's 0x3EE85698 (0.45378f gives 0x3EE855DA); the three stores
   at +0x160..+0x16C must go through a base pointer (`u8 *b0 = D_L23_001BA500; b0[0x16C]`),
   retail derives it as `addiu $v0,$s4,-0x140` from the +0x140 address, which then stays a
   `0x16C($v0)` store (writing `D_L23_001BA500 + 0x16C` folds it into `0x2C($s1)`); the one
   symbol D_L23_001BA500 with offsets (not two symbols) is what lets cc1 derive one address from
   the other. The statement order is the best of a hill-climb over the header statements. */

typedef unsigned char u8;
typedef unsigned short u16;
typedef short s16;
typedef int s32;
typedef int TI128 __attribute__((mode(TI)));

extern float D_L23_001B19FC __attribute__((sda));
extern s32 D_L23_001B3918[5];
extern u8 D_L23_001BA500[];

extern void *func_0029EFD0(s32);
extern float func_002A78D8(float, float);
extern s32 func_00283DB8(void *, float);
extern float func_00282E48(void *, void *);
extern void func_00282CF0(void *, void *, void *);
extern void func_00282F20(void *, void *, float);
extern float func_00282DB0(void *, void *);
extern float func_002831B0(float);
extern void func_L05_00428AC8(void *);

void *func_L23_00426248(u8 *p0, u8 *p1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7,
                        s32 a8, s32 a9, s32 a10, s32 a11, s32 a12, float f) {
    TI128 local;
    TI128 local2;
    float g;
    s32 i;
    u8 *obj;
    u8 *payload;
    u8 *pos;
    float d;

    if (D_L23_001B19FC == -1.0f) {
        for (i = 0; i < 5; i++) {
            D_L23_001B3918[i] = 0;
        }
        D_L23_001B19FC = 0.0f;
    }
    g = D_L23_001B19FC;
    if (g >= 5.0f) {
        return 0;
    }
    for (i = 0; i < 5; i++) {
        if (D_L23_001B3918[i] == 0) {
            break;
        }
    }
    D_L23_001B3918[i] = 1;
    D_L23_001B19FC = g + 1.0f;
    obj = func_0029EFD0(0x11DF);
    if (obj != 0) {
        pos = obj + 0x10;
        payload = *(u8 **)(obj + 0x68);
        obj[0x30] = 0xFF;
        *(s16 *)(obj + 0x32) = 0xFF;
        *(s32 *)(payload + 0x3C) = i;
        *(TI128 *)pos = *(TI128 *)p0;
        *(float *)(obj + 0xEC) = 0.0f;
        obj[0x20] = 0;
        *(s32 *)(payload + 0x24) = a4;
        *(float *)(obj + 0xDC) = 0.0f;
        *(float *)(obj + 0xCC) = 0.0f;
        *(float *)(obj + 0x1C) = 1.0f;
        *(float *)(payload + 0x14) = f;
        *(TI128 *)(obj + 0xC0) = *(TI128 *)(p1 + 0xC0);
        *(TI128 *)(obj + 0xE0) = *(TI128 *)(p1 + 0xE0);
        *(TI128 *)(obj + 0xD0) = *(TI128 *)(p1 + 0xD0);
        *(s32 *)(payload + 0x28) = a5;
        *(s32 *)(payload + 0x2C) = a6;
        *(s32 *)(payload + 0x34) = a9;
        *(s32 *)(payload + 0x30) = a7;
        *(s32 *)(payload + 0x48) = a12;
        *(s16 *)payload = 0x3C;
        *(s32 *)(payload + 0x20) = a3;
        *(float *)(payload + 0xC) = f * 4.0f;
        *(s16 *)(payload + 2) = 0x3C;
        *(float *)(payload + 8) = f + f;
        *(s32 *)(payload + 0x1C) = a10;
        *(s32 *)(payload + 0x38) = a11;
        *(s32 *)(payload + 0x18) = a8;
        *(float *)(payload + 0x44) = func_002A78D8(65.0f, 80.0f);
        local = *(TI128 *)pos;
        ((float *)&local)[3] = f * 3.0f;
        if (func_00283DB8(&local, 128.0f) == 1) {
            *(s16 *)(payload + 4) = 0x19;
            *(float *)(payload + 0x10) = 0.04f;
            if (func_00282E48(p0, D_L23_001BA500 + 0x140) < 48.0f && a2 != 0) {
                u8 *b0 = D_L23_001BA500;
                *(s32 *)(b0 + 0x16C) = 0;
                *(s32 *)(b0 + 0x168) = 0x28;
                *(float *)(b0 + 0x160) = 0.075f;
            }
            if (*(s32 *)(payload + 0x18) != 0) {
                func_00282CF0(&local2, pos, D_L23_001BA500 + 0x140);
                func_00282F20(&local2, &local2, 1.0f);
                d = func_002831B0(func_00282DB0(&local2, D_L23_001BA500 + 0x370));
                if (1.5707964f - d < 0.45378566f) {
                    *(s32 *)(payload + 0x18) = 1;
                } else {
                    *(s32 *)(payload + 0x18) = 0;
                }
            }
        } else {
            *(s16 *)(payload + 4) = 0;
            *(s32 *)(payload + 0x18) = 0;
        }
        func_L05_00428AC8(obj);
    }
    return obj;
}
