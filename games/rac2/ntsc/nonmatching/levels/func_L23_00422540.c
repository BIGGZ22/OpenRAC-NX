/* func_L23_00422540 (level 23, 292 bytes) -- NEAR MISS, not integrated.
   state: 284 bytes against retail's 292 (71 instructions against 73); check.py: 0 of 1 places match.
   matches: the 0x60 frame with the $f20 save, the two first calls (the first local at sp+0), the field
   stores, the four vector calls with 1.0f kept in $f20, and the mtc1 $zero / nop / swc1 tail.
   what still differs: retail keeps the second local's address in $s2 (`addiu $s2,$sp,0x10` after the
   copies, passed as `daddu $a0,$s2`, read back as `lq $v0,0($s2)` after the call) and copies *arg0
   through a materialised `addiu $a2,$s3,0x10`. This cc1 folds all three (`addiu $a0,$sp,0x10`,
   `lq $v0,0x10($sp)`, `sq $v0,0x10($s2)`); without the $s2 pseudo the object takes $s2 and arg0 $s3,
   and sched1 hoists obj+0xC0, obj+0xD0 and the 1.0f load above the copies.
   measured with -da dumps (2026-10-20): CSE1 rewrites a TImode MEM through a pointer it knows to hold a
   local's address (the ADDRESSOF of a TI local, or fp+16 of a BLKmode one) into the frame address; only
   a read in another extended basic block keeps the pointer (func_L23_004234C0's `pos` matched that way,
   at offset 0). Tried: the pointer initialised at its declaration (the locals swap places and &local1
   ends in a saved register: 296 bytes), the second local as an aligned struct (BLKmode: ld/sd copies,
   304), both locals in one struct or one array with a pointer to the second (284), a float[4] local
   (296). See func_L23_00422668.c for what would unblock this family.
   In place in L23_0041E418.c this needs the file's TI16 and s16 typedefs moved above it.
*/

typedef unsigned char u8;
typedef short s16;
typedef int s32;

typedef int TI16 __attribute__((mode(TI)));

extern void func_002AFFD8(void *, void *, s32);

extern u8 *func_0029EFD0(s32);

extern void func_00282F20(float *, float *, float);

extern void func_00282DD8(float *, float *, float *);

/* builds the object from the two records and the float, then sets its three floats */
u8 *func_L23_00422540(u8 *arg0, TI16 *arg1, float *arg2) {
    TI16 local1;
    TI16 local2;
    u8 *obj;
    u8 *payload;
    float value;
    TI16 *second;

    func_002AFFD8(arg0, &local1, 1);
    obj = func_0029EFD0(0x118E);
    if (obj != 0) {
        value = *arg2;
        payload = *(u8 **)(obj + 0x68);
        *(float *)(payload + 0x1C) = value;
        *(s32 *)(payload + 0x10) = 0x3C;
        *(u8 *)(obj + 0x30) = 0xFF;
        *(s16 *)(obj + 0x32) = 0xFF;
        *(u8 *)(obj + 0x20) = 0;
        *(TI16 *)(obj + 0x10) = *(TI16 *)arg0;
        *(TI16 *)payload = *arg1;
        *(s32 *)(payload + 0x14) = 0;
        *(s32 *)(payload + 0x18) = 0;
        second = &local2;
        func_00282F20((float *)second, (float *)arg1, 1.0f);
        *(TI16 *)(obj + 0xC0) = *second;
        func_00282DD8((float *)(obj + 0xD0), (float *)(obj + 0xC0), (float *)&local1);
        func_00282F20((float *)(obj + 0xD0), (float *)(obj + 0xD0), 1.0f);
        func_00282DD8((float *)(obj + 0xE0), (float *)(obj + 0xD0), (float *)(obj + 0xC0));
        value = 0.0f;
        *(float *)(obj + 0xCC) = value;
        *(float *)(obj + 0xEC) = value;
        *(float *)(obj + 0xDC) = value;
    }
    return obj;
}
