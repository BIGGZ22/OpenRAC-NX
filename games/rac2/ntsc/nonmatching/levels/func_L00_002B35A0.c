/* First plain-C attempt from worker w08. Last check: 244 bytes vs 320; 0/27. */
typedef unsigned char u8;
typedef int CandidateV4 __attribute__((mode(TI)));
extern u8 D_00189E20_1[];
extern void func_00282978(char *);
extern char D_L00_001B8C80[];
extern void func_L00_002B3EF8(int, float, float, float);

void func_L00_002B35A0(void)
{
    char *p = (char *)D_00189E20_1;
    short a;
    short b;
    float f;

    *(float *)(p + 0x1458) = *(float *)(p + 0x1A0);
    *(CandidateV4 *)(p + 0x190) = *(CandidateV4 *)(p + 0x1440);
    func_00282978(p + 0x146C);
    func_00282978(p + 0x146E);
    if (*(unsigned char *)(p + 0x22B4) != 0)
        return;
    a = *(short *)(p + 0x146C);
    b = *(short *)(p + 0x146E);
    if (*(unsigned char *)(p + 0x1457) != 0)
        b = 1;
    else if (a == 0)
        b = 5;
    *(short *)(p + 0x146E) = b;
    if (b == 0)
        *(short *)(p + 0x146C) = 5;
    if (b != 0) {
        f = (4.0f - (float)b) * 0.25f;
        *(float *)(p + 0x1A0) = *(float *)(D_L00_001B8C80 + 0x158);
        *(float *)(p + 0x1B4) = *(float *)(p + 0x1B4) * f;
        func_L00_002B3EF8(0, 0.037f, 0.25f, 0.203f);
        *(float *)(p + 0x145C) = *(float *)(p + 0x1A4);
        *(float *)(p + 0x1A4) = 0.0f;
        *(float *)(p + 0x1A8) = 0.0f;
    }
}
