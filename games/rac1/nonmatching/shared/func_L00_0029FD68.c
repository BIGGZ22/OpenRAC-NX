/* NON_MATCHING func_L00_0029FD68 -- src/overlays/shared/vendor_0029FD68.c
 * Best so far: SIZE ours 1156 / retail 1164, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * Cannot land as written (#define in a candidate): rewrite that in plain C first.
 * What the last attempts found:
 *   func_L00_0029FD68: draws a 3-way stat/price box (clears a 0x18-byte box, then prints text lines via func_001F7
 *   Best candidate p8.c: SIZE 1156/1164 (2 instructions short); structure, saved regs (s0-s4) and first ~130 instr
 *   Difference: retail loads D_L00_00161F30 inside each arm of the `g[0x10] ? hA : h8` select before the shared mu
 *   Round q28: first param must be void * (caller declares it so; int a0 gave COMPILE). p10/p11: ternary for the p
 */
extern void func_001FBAB8(int, int, int, int, int, int, int);
extern void func_001153FC(void *, int, int);
extern void func_0020E180(int, int);
extern void *func_001FE540(int);
extern void func_001F75D0(void *, long, void *, int);
extern int func_00116248(char *str, const char *fmt, ...);
extern void func_001F6968(int, int, long, void *, int);
extern void func_001F6D88(int, int, long, void *, int);
extern int D_L00_001CA7C0[];
typedef struct { int i0; int i4; unsigned short h8; unsigned short hA; unsigned short hC; unsigned short hE; int i10; int i14; } Row;
extern Row D_L00_001C43B0[] NOT_SDA;
extern char D_0013D50F[] NOT_SDA;
extern unsigned char D_0013D5EB NOT_SDA;
extern int D_0015EE98 MACRO_ADDR;
extern char D_L00_00161220[];
extern int D_L00_00161F30 MACRO_ADDR;
extern char D_L00_00161228[];
extern char D_L00_00161230[];

#define ENT ((int *)((char *)D_L00_001CA7C0 + 0xD0 + D_L00_001CA7C0[0x58 / 4] * 0x14))
#define ROW(e) (D_L00_001C43B0[e])

// draws the mission-stat box: title, price and count lines, or a "cannot" message
void func_L00_0029FD68(void *a0, int w, int h) {
    short b[12];
    char s[256];
    int v;
    int id;
    func_001FBAB8(0, 0, 0x200, 0x80, 0x200, 0x80, 0);
    func_001153FC(b, 0, 0x18);
    b[4] = w / 2;
    b[5] = h / 2;
    b[6] = w - 8;
    b[7] = h - 8;
    b[8] = 0x10;
    b[9] = 3;
    b[1] = h;
    b[3] = w;
    if (ENT[1] == 1 && !(((int *)(D_0013D50F + 0x21))[ENT[0]] < ROW(ENT[0]).hE)) {
        id = 0x523B;
        goto tail;
    }
    if (ENT[1] == 1) {
        if (D_L00_001CA7C0[0x10])
            v = ROW(ENT[0]).hA;
        else
            v = ROW(ENT[0]).h8;
        if (!(D_0015EE98 < v)) {
            func_0020E180(D_L00_001CA7C0[8] + 0x500, 1);
            b[9] = 1;
            b[5] = 6;
            func_001F75D0(b, 0x80F0F0F0L, func_001FE540(0x523C), -1);
            func_00116248(s + 0x20, D_L00_00161220, D_L00_00161F30);
            if (D_L00_00161F30 >= 100)
                func_001F6968(0x17, 0x32, 0x80F0F0F0L, s + 0x20, -1);
            else if (D_L00_00161F30 >= 10)
                func_001F6968(0x1E, 0x32, 0x80F0F0F0L, s + 0x20, -1);
            else
                func_001F6968(0x25, 0x32, 0x80F0F0F0L, s + 0x20, -1);
            func_001F6968(0xC, 0x32, 0x80F0F0F0L, D_L00_00161228, -1);
            func_001F6968(0x40, 0x32, 0x80F0F0F0L, D_L00_00161230, -1);
            v = D_L00_00161F30 * (D_L00_001CA7C0[0x10] ? ROW(ENT[0]).hA : ROW(ENT[0]).h8);
            func_00116248(s + 0x20, D_L00_00161220, v);
            func_001F6D88(0x40, 0x4C, 0x80F0F0F0L, s + 0x20, -1);
            return;
        }
        id = 0x523D;
        goto tail;
    }
    goto third;
tail:
    func_001F75D0(b, 0x80F0F0F0L, func_001FE540(id), -1);
    return;
third:
    if (D_0013D5EB)
        v = ROW(ENT[0]).i4;
    else
        v = ROW(ENT[0]).i0;
    if (!(D_0015EE98 < v)) {
        func_001F6968(6, 6, 0x80F0F0F0L, func_001FE540(0x523E), -1);
        func_001F6968(2, 0x20, 0x80F0F0F0L, func_001FE540(0x5239), -1);
        func_001F6968(2, 0x3E, 0x80F0F0F0L, func_001FE540(0x5250), -1);
        return;
    }
    func_001F75D0(b, 0x80F0F0F0L, func_001FE540(0x523D), -1);
}
