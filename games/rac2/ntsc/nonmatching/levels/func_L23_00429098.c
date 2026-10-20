/* func_L23_00429098 (552 bytes, 1 place: level 23) -- NEAR MISS, not integrated.
   state: 552 bytes against retail's 552 (138 instructions); 91 words differ, the first at +0x4.
   Checked in the -G8 unit (it reads three words through `$gp`: D_L23_001B1B8C, D_L23_001B1AE8 and
   D_L23_001B1AEC; `$gp` is 0x001AEFF0); it belongs in L23_0041E418-g8.c.
   What matches: the call sequence with every argument (func_0029EFD0, the func_00283248 /
   func_00283B98 / func_00282E20 angle chain, the two func_002ACEC0 calls, func_002833F0,
   func_002A7878, the two func_002C2210 emissions), all stores and constants (1.5707965f is the
   spelling that encodes retail's 0x3FC90FDC; 1.5707964f gives 0x3FC90FDB), the `ld`/`sd` of the
   parent's eight bytes, the two quad copies, the 0x208 and 0x100 flag updates, the return of the new
   object, the frame (0x50, six saved registers and two float registers, f20 and f21).
   What differs:
   - the quad copy into the object at +0x10. Retail builds `$s2 = obj + 0x10` first (`addiu $s2,$s4,
     0x10` at +0x60), stores through it (`sq $v0,0($s2)`) and keeps it for the two final calls;
     this cc1 folds the +0x10 into the store (`sq $v0,0x10($s3)`) and builds the pointer for the calls
     separately. This is the quad wall of func_L23_00425848.c, and it moves the other allocations:
     retail s0 = a3 then the payload, s1 = a0 then &obj[0xC0], s3 = a2 then &obj[0xD0], s4 = obj,
     s5 = a1; this cc1 s0 = a1, s1 = a3 then the payload, s3 = obj, s4 = a0 then &obj[0xC0],
     s5 = a2 then &obj[0xD0]; and f20/f21 are swapped (retail: the scale and the random value in f20,
     0.0f in f21);
   - the address of D_L23_001A8F20 is built once and kept in a callee-saved register for both final
     calls (`daddu $t1,$s0,$zero` twice), retail rebuilds `lui/addiu $t1` before each call (the
     known cse wall; equal size, a different register);
   - scheduling: retail issues the byte stores and their three constants in another order, loads the
     payload after the quad copies, and reads `0x2B9C($gp)` and `0x2AF8($gp)` earlier.
   What the source had to do for the size: a0 and a2 are reused as the pointers to the +0xC0 and
   +0xD0 records once their last use (the store into the payload, the quad copy) has passed. With
   fresh names the scheduler hoists the two additions to the top of the block (free ALU slots beside
   the byte stores), the parameters stay live beside them, and the function needs eight saved
   registers: 568 bytes. Retail's `$s3` and `$s1` hold a2 and a0 and then the two pointers, which is
   what the reuse gives. 120 orders of the five early stores, 120 orders of the local declarations,
   and 576 orders of the two groups of independent stores give nothing better than 91 words (this
   C is the best of them).
*/

typedef unsigned char u8;

typedef unsigned short u16;

typedef short s16;

typedef int s32;

typedef long long s64;

typedef int TI128 __attribute__((mode(TI)));

extern float D_L23_001B1B8C __attribute__((sda));

extern s32 D_L23_001B1AE8 __attribute__((sda));

extern s32 D_L23_001B1AEC __attribute__((sda));

extern char D_L23_001A8F20[];

extern u8 *func_0029EFD0(s32);

extern float func_00283248(float, float);

extern float func_00283B98(float, float);

extern float func_00282E20(void *);

extern float func_002A7940(void);

extern void func_002ACEC0(void *, void *, void *, float);

extern void func_002833F0(void *, void *);

extern float func_002A7878(float, float);

extern void func_002C2210(void *, s32, s32, s32, s32, void *, s32, float, float, float);

/* spawns object type 0x1237 from a parent: copies the parent's 8 bytes at +0x38, the position a2 and
   the direction a1, derives the yaw and the pitch from the direction, builds two rotated basis
   vectors, and emits two bursts of particles at the new object's position */
u8 *func_L23_00429098(u8 *a0, u8 *a1, u8 *a2, s32 a3, float scale) {
    u8 *p = func_0029EFD0(0x1237);
    u8 *q;
    float angle;
    float len;
    float r;

    if (p != 0) {
        p[0x30] = 0xFF;
        *(s16 *)(p + 0x32) = 0xFF;
        p[0x31] = 1;
        p[0xBC] = a3;
        p[0x20] = 0;
        *(s64 *)(p + 0x38) = *(s64 *)(a0 + 0x38);
        *(TI128 *)(p + 0x10) = *(TI128 *)a2;
        q = *(u8 **)(p + 0x68);
        *(TI128 *)q = *(TI128 *)a1;
        *(u8 **)(q + 0x10) = a0;
        *(float *)(q + 0x14) = scale;
        *(float *)(q + 0x20) = 0.15f;
        a2 = p + 0xD0;
        a0 = p + 0xC0;
        *(u16 *)(p + 0x34) |= 0x208;
        angle = func_00283248(((float *)a1)[0], ((float *)a1)[1]);
        *(float *)(p + 0xF8) = func_00283B98(angle, 1.5707965f);
        len = func_00282E20(a1);
        *(float *)(p + 0xF0) = -func_00283248(len, ((float *)a1)[2]);
        r = func_002A7940();
        func_002ACEC0(p + 0xE0, p + 0xE0, a2, r);
        func_002ACEC0(a0, a0, a2, r);
        func_002833F0(a0, p + 0xF0);
        p[0x23] = 0x80;
        *(float *)(p + 0x2C) = D_L23_001B1B8C;
        *(u16 *)(p + 0x34) |= 0x100;
        *(s32 *)(q + 0x24) = 0;
        r = func_002A7878(8.0f, 10.0f);
        *(s32 *)(q + 0x2C) = 0;
        func_002C2210(p + 0x10, 0xF, D_L23_001B1AE8, 0, 0xA, D_L23_001A8F20, 0, r * 0.3f, r, 0.0f);
        func_002C2210(p + 0x10, 0xF, D_L23_001B1AEC, 1, -0xA, D_L23_001A8F20, 0, r * 0.04f, r * 0.4f, 0.0f);
    }
    return p;
}
