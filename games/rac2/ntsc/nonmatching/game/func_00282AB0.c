/* func_00282AB0 -- the game's own memcpy, 160 bytes in retail.

   It picks the stride from the alignment of the destination, the source and
   the end address: 16 bytes when all three are 16-byte aligned, 4 bytes when
   they are 4-byte aligned, one byte otherwise.

   Measured with tools/check.py in src/game/0027F128.c (the -O2 -G0 profile):
   "func_00282AB0: 0 of 28 places match", 156 bytes against retail's 160.

   What differs:
   - the retail body is laid out byte-loop, 4-byte-loop, 16-byte-loop, with
     plain `beqz` branches jumping forward to the 4-byte and the 16-byte
     loops; this cc1 lays the blocks out in the source order and turns the two
     branches into their branch-likely forms, with the first instruction of
     the target block in the delay slot;
   - retail computes the end address with the trapping `add` and the three
     strides with `addi`, this cc1 uses `addu` and `addiu` for the same
     expressions;
   - retail loads the byte with `lb`, this cc1 loads it with `lbu`: the sign
     of the copied byte is irrelevant here, so this cc1 widens the load;
   - retail keeps the alignment word in `$v1` and the test result in `$v0`,
     this cc1 swaps the two.
*/
typedef unsigned int u32;
typedef int s32;
typedef signed char s8;
typedef int TI128 __attribute__((mode(TI)));

void func_00282AB0(signed char *dst, signed char *src, s32 len) {
    signed char *end;
    if (len == 0) {
        return;
    }
    end = dst + len;
    if (!((((u32)dst | (u32)src | (u32)end) & 0xF) == 0)) {
        if (!((((u32)dst | (u32)src | (u32)end) & 3) == 0)) {
            do {
                *dst = *src;
                dst++;
                src++;
            } while (dst != end);
        } else {
            do {
                *(s32 *)dst = *(s32 *)src;
                dst += 4;
                src += 4;
            } while (dst != end);
        }
    } else {
        do {
            *(TI128 *)dst = *(TI128 *)src;
            dst += 16;
            src += 16;
        } while (dst != end);
    }
}
