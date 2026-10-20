/* func_L02_003DB158, 164 bytes, 1 placement (level 2).  Closest candidate 160 bytes - ONE
   instruction short, and it is the same wall as its twin func_L02_003DAF90: the 16-byte
   lq/sq copy's destination stays folded on the frame pointer here
   (`lq $v1,0x0($sp); sq $v1,0x10($sp)`) where retail materialises it first
   (`addiu $v1,$sp,0x10; lq $v0,0x0($sp); sq $v0,0x0($v1)`).  Five spellings of that copy
   in func_L02_003DAF90 (TImode cast, named destination pointer, union assignment, explicit
   +0 through a char base, pointer-to-source) all fold, so it is the scheduler/allocator and
   not the writing.  Everything else - the 0xA0 frame, the four scratch blocks at
   sp+0x00/0x10/0x20/0x60 with the 0x40-byte one in the middle, all five calls and their
   delay slots - is exact.
   It needs the -G8 unit of src/levels/02_maktar_nebula/L02_003D26B0.c: the resident block
   base and the two floats are `$gp + 0x1490/0x1488/0x1484`, i.e. 0x001B0480 / 0x001B0478 /
   0x001B0474 with $gp pinned at 0x001AEFF0.  Body measured in that unit: */

typedef int s32;
typedef float f32;
typedef int TQ __attribute__((mode(TI)));

extern f32 D_L02_001B0474 __attribute__((sda));
extern f32 D_L02_001B0478 __attribute__((sda));
extern s32 D_L02_001B0480 __attribute__((sda));

extern void *func_0029FCC0(char *, int, void *);
extern void func_0029FC20(char *, int, void *);
extern void func_002830C0(void *, void *, void *);
extern void func_00282CC0(void *, void *, void *);
extern void func_L00_00325AE8(void *, int, float, float);

typedef struct { f32 x, y, z, w; } V4;
typedef struct { f32 v[16]; } M4;

void func_L02_003DB158(char *a0, s32 a1) {
    char *s3 = *(char **)(*(char **)(a0 + 0xB8) + 0x68);
    V4 src, copy;
    M4 block;
    V4 out;

    func_0029FCC0(a0, a1, &src);
    *(TQ *)&copy = *(TQ *)&src;
    func_0029FC20(a0, a1, &block);
    func_002830C0(&out, &D_L02_001B0480, &block);
    func_00282CC0(&src, &src, &out);
    func_L00_00325AE8(&src, *(s32 *)(s3 + 0x250), D_L02_001B0478, D_L02_001B0474);
}
