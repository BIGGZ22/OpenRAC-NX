/* 0 of 27 exact: 144 bytes, retail 164 (5 instructions short: retail keeps base, offset and record pointer live across the calls and saves s0-s4). The calls and the 96.0f scale match; the compiler folds base+off and loses the saved registers. */
extern char D_L00_0024EFC0[];

extern int func_00283CF0(float value);

extern int func_00283D38(int a, int b, float value);

extern void func_002A0488(char *object, int r, int g, int b);

void func_L00_003E80A8(int index, int param)
{
    char *base;
    char *field;
    int off;
    int color;

    base = D_L00_0024EFC0;
    off = index << 5;
    field = base + 0xC;
    field = field + off;
    *(char *)(*(char **)(base + off) + 0x23) = func_00283CF0(*(float *)field * 96.0f);
    color = func_00283D38(param, *(int *)(base + off + 4), *(float *)field);
    func_002A0488(*(char **)(base + off), color & 0xFF, (color >> 8) & 0xFF, (color >> 16) & 0xFF);
}

