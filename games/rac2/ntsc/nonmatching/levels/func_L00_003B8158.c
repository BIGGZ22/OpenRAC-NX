/* 88 bytes vs retail 116 (22 instructions vs 29): the control flow and the first call match; missing the else arm, which retail builds as an 8-byte local copied from 0x1AFBC8 with ldl/ldr/sdl/sdr (unaligned doubleword pairs, the struct's type unknown) and a 0x20-byte frame. The frame here is 0x10. */
extern void func_37CB80(float scale, void *dst, void *src);

typedef struct OBJ5 {
    char pad0[0x10];
    unsigned char f24_pad[0x14];
} OBJ5;

void func_L00_003B8158(OBJ5 *a0)
{
    OBJ5 *o = *(OBJ5 **)((char *)a0 + 0x68);
    int a = *(int *)((char *)o + 0x24);
    int b;

    if (a == 0)
        return;
    b = *(int *)((char *)o + 0x2C);
    if (b != 0)
        func_37CB80(1.0f, (char *)o + 0x10, 0);
    else
        func_37CB80(1.0f, (char *)o + 0x10, 0);
}
