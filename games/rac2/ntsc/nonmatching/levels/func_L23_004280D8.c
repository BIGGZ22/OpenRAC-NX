/* func_L23_004280D8 (580 bytes, 1 place: 23:004280D8) -- NEAR MISS, not integrated.
   This C compiles to 584 bytes against retail's 580 (-g8 unit, -O2 -G8): one callee-saved
   register too many, and one word of a quad store that cannot be spelled. The calls, their
   arguments, every constant and every store are retail's; 98 of retail's 145 words match in
   place, the rest is allocation / order.
   What it is: a spawner. It allocates a 0x1230 object, stamps its header bytes, copies the
   parent's +0x38 doubleword, copies the position quad (a2) to +0x10 and the direction quad (a1)
   to the sub-object, then derives the facing angles (atan2 of a1[0],a1[1]; -atan2(|a1|, a1[2]))
   and the two rotation rows at +0xC0/+0xE0 from a random scale, and queues two particle
   requests (func_002C2210) whose float arguments scale one random value in 8..10.
   What differs:
   1. `lq $v0,0($s3) ; sq $v0,0($s2)` (retail +0x78/+0x7C, s2 = obj+0x10 computed at +0x60 and
      reused as `$a0` of both func_002C2210 calls): this cc1 folds the offset into the store,
      `sq $v0,0x10($s4)`. A `u8 *pos = obj + 0x10; *(TI128 *)pos = *a2;` local does not change it.
      Because pos then has no early consumer, sched1 gives the `addiu` of pos a low priority (25) and
      the two `addiu` for obj+0xD0 / obj+0xC0 (priority about 45, ready from the first cycle) fill
      the idle ALU slots at the top of the block, so they are live across the parent / a2
      copies and the allocation needs $s6 (retail reuses $s1 for obj+0xC0 after its last use
      of the parent, and $s3 for obj+0xD0 after the quad copy of a2).
      Retail schedules them after the two quad pairs, which is what two scheduling barriers
      (volatile-asm copies) would force. Every pair of the four L23 bodies in this batch is
      `lq $v0,0($p) ; sq $v0,0($q)` with both pointers materialised by their own `addiu`: the
      shape of an inline `qcopy` macro, not of a cc1 TImode move (func_L00_00395240 shows the cc1
      shape, offsets folded: `lq $v1,0xC0($s1) ; sq $v1,0xC0($s0)`).
   2. $f20 / $f21: retail keeps `f` and then the random in $f20 and the 0.0f argument in $f21
      (`mtc1 $zero,$f21`), this cc1 the opposite.
   3. the four closing `lwc1`/`swc1` pairs (D_L23_001B1A14/2C/38/20 into payload +0x30/34/38/3C):
      retail loads all four, then stores in the order 0x30, 0x3C, 0x34, 0x38; here each store
      follows its load.
   Measured: `1.5707965f` (1.5707964f encodes 0x3FC90FDB, retail has 0x3FC90FDC); the second
   `func_002C2210` call needs its own name for the table address (`D_L23_001A8F20_2`, the same
   address): with one name cse keeps the address in a callee-saved register across the calls
   where retail rebuilds `lui $t1,0x1B ; addiu $t1,$t1,-0x70E0` for each call. The statement
   order below is the best of a hill-climb over the header statements, scored by instruction-
   line differences against retail (tools/check.py's side by side, immediates ignored). */

typedef unsigned char u8;
typedef unsigned short u16;
typedef short s16;
typedef int s32;
typedef long long s64;
typedef int TI128 __attribute__((mode(TI)));

extern float D_L23_001B1AD8 __attribute__((sda));
extern s32 D_L23_001B1A44 __attribute__((sda));
extern s32 D_L23_001B1A48 __attribute__((sda));
extern float D_L23_001B1A14 __attribute__((sda));
extern float D_L23_001B1A2C __attribute__((sda));
extern float D_L23_001B1A38 __attribute__((sda));
extern float D_L23_001B1A20 __attribute__((sda));
extern u8 D_L23_001A8F20[];
extern u8 D_L23_001A8F20_2[]; /* the same address as D_L23_001A8F20 */

extern void *func_0029EFD0(s32);
extern float func_00283248(float, float);
extern float func_00283B98(float, float);
extern float func_00282E20(void *);
extern float func_002A7940(void);
extern float func_002A7878(float, float);
extern void func_002ACEC0(void *, void *, void *, float);
extern void func_002833F0(void *, void *);
extern void func_002C2210(void *, s32, s32, s32, s32, void *, s32, float, float, float);

u8 *func_L23_004280D8(u8 *parent, TI128 *a1, TI128 *a2, s32 b, float f) {
    u8 *obj = func_0029EFD0(0x1230);
    u8 *payload;
    float r;

    if (obj != 0) {
        *(TI128 *)(obj + 0x10) = *a2;
        *(s16 *)(obj + 0x32) = 0xFF;
        obj[0x30] = 0xFF;
        obj[0xBC] = b;
        obj[0x31] = 1;
        *(s64 *)(obj + 0x38) = *(s64 *)(parent + 0x38);
        obj[0x20] = 0;
        payload = *(u8 **)(obj + 0x68);
        *(u8 **)(payload + 0x10) = parent;
        *(TI128 *)payload = *a1;
        *(float *)(payload + 0x14) = f;
        *(float *)(payload + 0x20) = 0.15f;
        *(u16 *)(obj + 0x34) |= 0x208;
        *(float *)(obj + 0xF8) = func_00283B98(func_00283248(((float *)a1)[0], ((float *)a1)[1]), 1.5707965f);
        *(float *)(obj + 0xF0) = -func_00283248(func_00282E20(a1), ((float *)a1)[2]);
        r = func_002A7940();
        func_002ACEC0(obj + 0xE0, obj + 0xE0, obj + 0xD0, r);
        func_002ACEC0(obj + 0xC0, obj + 0xC0, obj + 0xD0, r);
        func_002833F0(obj + 0xC0, obj + 0xF0);
        obj[0x23] = 0x80;
        *(float *)(obj + 0x2C) = D_L23_001B1AD8;
        *(u16 *)(obj + 0x34) |= 0x100;
        *(s32 *)(payload + 0x24) = 0;
        r = func_002A7878(8.0f, 10.0f);
        func_002C2210(obj + 0x10, 0xF, D_L23_001B1A44, 0, 10, D_L23_001A8F20, 0, r * 0.3f, r, 0.0f);
        func_002C2210(obj + 0x10, 0xF, D_L23_001B1A48, 1, -10, D_L23_001A8F20_2, 0, r * 0.04f, r * 0.4f, 0.0f);
        *(float *)(payload + 0x3C) = D_L23_001B1A20;
        *(float *)(payload + 0x38) = D_L23_001B1A38;
        *(float *)(payload + 0x34) = D_L23_001B1A2C;
        *(float *)(payload + 0x30) = D_L23_001B1A14;
    }
    return obj;
}
