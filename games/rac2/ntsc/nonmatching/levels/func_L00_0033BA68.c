/* 0 of 27 places match - last line: 00_aranos_tutorial: 220 bytes, expected 216 (one word over).
   The same record template as func_L00_00338D28 with the 0x46 class: tag 0x5A404040,
   b9 = func_00283CF0(1.0f) + 0x20, b3 = 0x44, b1 = 1, b2 = *(u8 *)((int *)0x001FEA58 + index),
   f_c = 105000.0f, s_a = 0x78 and b8 = func_00283CF0(value * 40.743664f) - 0x20.
   Same wall as func_L00_00338D28: the 16-byte copy's store address is folded into the sq
   displacement here and materialised in the beqz's delay slot in retail. */

typedef int V4_L00_0033BA68 __attribute__((mode(TI)));

typedef struct {
    unsigned char b0;
    unsigned char b1;
    unsigned char b2;
    unsigned char b3;
    int w4;
    unsigned char b8;
    unsigned char b9;
    short s_a;
    float f_c;
    char v10[16];
    char v20[16];
} Obj_L00_0033BA68;

extern void *func_002B9030(int);
extern int func_002A77E0(int);
extern int func_00283CF0(float);
extern int *D_L00_001FEA58;

Obj_L00_0033BA68 *func_L00_0033BA68(char *src, int index, float value)
{
    Obj_L00_0033BA68 *o = func_002B9030(0x46);

    if (o == 0)
        return 0;
    *(volatile V4_L00_0033BA68 *)((char *)o + 0x10) = *(V4_L00_0033BA68 *)src;
    o->w4 = 0x5A404040;
    o->b9 = func_00283CF0(1.0f) + 0x20;
    o->b3 = 0x44;
    o->b1 = 1;
    o->b2 = *(unsigned char *)((char *)D_L00_001FEA58 + index);
    o->f_c = 105000.0f;
    o->s_a = 0x78;
    o->b8 = func_00283CF0(value * 40.7436637878418f) - 0x20;
    return o;
}
