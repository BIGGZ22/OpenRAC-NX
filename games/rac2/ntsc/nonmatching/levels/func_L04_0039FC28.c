/* func_L04_0039FC28, 176 bytes, 5 places, now in the src/levels/shared/L04_002CC748-g8.c
   sibling (-O2 -G8 -ffunction-sections -mno-split-addresses).
   ABANDON - one instruction pair sits two slots later than retail's.
   Read in full: the three position components and the two angles are packed into the record at
   `dst`, each offset by its own constant (0x001AD848, 0x001AD84C, -1.0f, and the level's
   small-data angle D_L04_001AD840 twice) and scaled to 1/1024 units through
   `func_00283CF0(float)`.
   The candidate is 176 bytes = retail's size with every instruction present and 34 of 44 words
   identical, single-precision `sda` float and the two absolute constants included (the recipe
   that worked here: the `-g8` sibling, the UNIT's `-mno-split-addresses` line, and the two
   absolute constants spelled as literals so `lui $at,0x1B` folds them into the load:
   `lwc1 $f0,-0x27B8($at)`). What is left is the placement of the first pair: retail issues
   `lui $at,0x1B ; lwc1 $f0,-0x27B8($at)` BEFORE the four prologue stores, this cc1 always issues
   it after them (and after the 1024.0f materialisation), shifting the next ten words by one
   pair. Measured: the constant read into a local first, both constants into locals (that one
   also costs an `$f21` save, 184), the first sum in a local, the scale in a local, the constant
   read through a pointer local, and `-fno-schedule-insns2` (which moves the pair after the
   stores but also sinks `$s1 = $a1`) - every shape puts the pair after the stores. Below: the
   nearest candidate (176 bytes, ten words shifted). */

typedef float f32;

extern f32 D_L04_001AD840 __attribute__((sda));
extern int func_00283CF0(float);

void func_L04_0039FC28(char *src, char *dst) {
    f32 k0 = *(f32 *)0x001AD848;

    *(int *)(dst + 0) = func_00283CF0((*(f32 *)(src + 0) + k0) * 1024.0f);
    *(int *)(dst + 4) = func_00283CF0((*(f32 *)(src + 4) + *(f32 *)0x001AD84C) * 1024.0f);
    *(int *)(dst + 8) = func_00283CF0((*(f32 *)(src + 8) - 1.0f) * 1024.0f);
    *(short *)(dst + 0xC) = func_00283CF0(D_L04_001AD840 * 1024.0f);
    *(short *)(dst + 0xE) = func_00283CF0(D_L04_001AD840 * 1024.0f);
}
