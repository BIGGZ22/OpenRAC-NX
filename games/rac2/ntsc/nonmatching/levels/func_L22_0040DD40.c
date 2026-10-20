/* func_L22_0040DD40 (192 bytes, 2 places: 22_dobbo_orbit, 23_damosel_orbit) -- NEAR MISS.
   state: exactly 192 bytes and every instruction present; the first difference is the schedule of
   the 16-byte copy. check.py (22_dobbo_orbit) with the source below:
     0040DD90  lq    $v0, 0x10($s0)   |  addiu $a0, $s0, 0x10
     0040DD94  addiu $a0, $s0, 0x10   |  lq    $v0, 0x0($a0)
     0040DD98  daddu $a2, $a0, $zero  |  sq    $v0, 0x0($sp)
     0040DD9C  daddu $a1, $s1, $zero  |  daddu $a2, $a0, $zero
     0040DDA0  jal   func_2F3A80      |  jal   func_2F3A80
     0040DDA4  sq    $v0, 0x0($sp)    |  daddu $a1, $s1, $zero
   Retail materialises the source pointer first (`addiu $a0,$s0,0x10`) and keeps the `lq`/`sq`
   pair adjacent; this cc1 folds the +0x10 into the `lq` offset and sinks the `sq` into the `jal`
   delay slot. This is the TI128 destination wall the lead reconfirmed three times - the file is
   deposited rather than retried.

   What is measured and right:
   - the three-label switch is required: `case 0` stamps state 1 and falls into the case-1 body,
     `case 2` destroys and returns, and `default` shares the callback tail. Two cases alone make
     this cc1 build a comparison chain instead of the tree (`beq` on 1, then an `slti` guard on the
     range, then `beq` on 2) that the bytes show, and an if/else rewrite of the state machine
     loses 28 bytes;
   - the level's own callback address needs `ref_L22_0040E098_00` (0x0040E098 in 22_dobbo_orbit),
     not a literal;
   - the frame and the register set (s0, s1, ra, 0x30 frame) are exact with `char local[16]`
     declared after the handle load.

   Six spellings measured for the copy, all landing on the same wall: a named source pointer, the
   pointer with an explicit `+ 0`, two names for the same pointer, a `TI128`-typed local, a
   volatile source pointer, a volatile destination, and the copy as a comma expression inside the
   call's argument list. */

typedef unsigned char u8;
typedef float f32;

typedef int TI128_L22 __attribute__((mode(TI)));

extern char ref_L22_0040E098_00[];

extern void func_00282CC0_L22(char *out, char *first, char *second);

extern void func_L22_0040DF28(char *a0, char *a1);

extern int func_00282948(char *a0);

extern void func_0029F338_L22(char *a0);

extern void func_002E6EF8_L22(char *callback, char *object);

/* Three-state gate on the record's byte 0x20. State 0 stamps itself state 1 and falls into the
 * state-1 body (a 16-byte copy of the record's vector, the pair update and the shared sub-object
 * test that promotes the record to state 2); state 2 destroys the record and returns; every other
 * state, and the end of the state-1 body, re-arms the record through the level callback. */
void func_L22_0040DD40(char *a0)
{
    char *handle = *(char **)(a0 + 0x68);
    char local[16];

    switch (*(unsigned char *)(a0 + 0x20)) {
    case 0:
        *(unsigned char *)(a0 + 0x20) = 1;
    case 1: {
        char *p = a0 + 0x10;

        *(TI128_L22 *)local = *(TI128_L22 *)p;
        func_00282CC0_L22(p, handle, p);
        func_L22_0040DF28(a0, local);
        if (func_00282948(handle + 0x10) != 0)
            *(unsigned char *)(a0 + 0x20) = 2;
        break;
    }
    case 2:
        func_0029F338_L22(a0);
        return;
    default:
        break;
    }
    func_002E6EF8_L22(ref_L22_0040E098_00, a0);
}
