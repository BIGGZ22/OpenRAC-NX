/* func_L06_00457AD0 / func_L06_00457DA0 (300 bytes each, identical bodies).
 * -- ABANDON (304 bytes). Structure is right: every call, every constant and
 * every address matches, including the three-way switch decision tree, the
 * shared 0xA8 = 0xFF tail, the 16-byte stack buffer and the -0.033333335f /
 * 3.0f / 6.0f argument set. Remaining gap is one instruction plus callee-saved
 * allocation: retail keeps the parameter in $s0 and the 0x68 pointer in $s1,
 * gcc swaps them (parameter -> $s1, pointer -> $s0) however the locals are
 * declared (both `char *p = a0; char *s = ...` and the unreferenced-parameter
 * forms were tried), and that also changes the two branch-likely conversions in
 * the dispatch (retail beql + target-first delay slot, gcc beqz + epilogue load).
 */
extern char D_L06_001A9020[];
extern void func_35A550(char *, int, int);
extern int func_35CBA0(char *, int, int);
extern void func_32D650(char *, float, char *);
extern void func_L06_00457C00(char *, int, char *, int, float, float, float, float);
extern void func_3A0948(int, int, char *);
extern void func_3661E0(char *, int, float);
extern void func_350388(char *);

void func_L06_00457AD0(char *a0)
{
    char *p = a0;
    char *s = *(char **)(a0 + 0x68);

    switch (*(unsigned char *)(p + 0x20)) {
    case 0: {
        unsigned short u = *(unsigned short *)(p + 0x34);

        func_35A550(p, 1, -1);
        *(unsigned short *)(p + 0x34) = u | 0x800;
        break;
    }
    case 1:
        if (func_35CBA0(p, 0x04010000, 0) == 0) {
            *(unsigned char *)(p + 0xA8) = 0xFF;
            break;
        }
        if (*(int *)s != 0) {
            *(unsigned char *)(p + 0xA8) = 0xFF;
            break;
        }
        func_35A550(p, 2, -1);
        *(unsigned char *)(p + 0xA8) = 0xFF;
        break;
    case 2: {
        char buf[16];

        func_32D650(buf, -0.033333335f, p + 0xD0);
        func_L06_00457C00(p, 7, buf, 0xFF, 3.0f, 6.0f, 3.0f, 6.0f);
        func_3A0948(0, 0, p);
        func_3661E0(p, 1, 0.0f);
        func_350388(p);
        break;
    }
    }
}
