/* functional: high. Same reads, writes, calls and arguments in the same order as retail; differs only in compiler choices (registers, branch layout, which loads of the slot index are shared), and the gp-relative slot counter is named by its address (_gp = 0x166D00, -0x4D0C = 0x161FF4). */
/* func_L11_0030F4F8 -- src/overlays/l11_pokitaru/vendor_002CC828.c (functional C for the port, not a match)
 * Moby update of the level 11 pieces that share the 20-entry, 0x1190-byte VU chain table at
 * D_L11_001DAB40 (the level 11 twin of func_L12_002C0310). First, when D_L11_00167848 is over 180,
 * sets the seven bytes at D_L11_00161280..86.
 * State 0: claims the next slot (gp counter 0x161FF4, warns past 0x80), sets +0x30/+0x34/+0x2C; the
 * first slot also clears the 20 entries' +0x1E, builds the chain (func_L01_002B8C00/002B90A8), fills
 * the light block at D_L11_001CB500 and the chain globals at D_L11_00161288..358; then writes the
 * moby's position and fixed fields into its entry, runs func_L01_002B8E20 and goes to state 1.
 * State 1: links the entry's neighbours (func_L11_0030FBF8), resets the counter, goes to state 2.
 * States 2 and 3: on hit flag 8 / 4 (+0xBC) start falling (data+8 = 1, func_L01_0026F090 with 3 / 2),
 * set +0xBC to 2 / 1, step the fall with func_00214D88 (target 137 / 132.36) and func_L11_0030FB58,
 * and one frame in 200 spawn a particle near the entry (func_L00_002A5158).
 * Then, for a claimed slot: the entry's +0x1E is 0xFFFF while func_L01_00276680 finds something within
 * 64, else 0; slot 0 also refreshes the chain (func_L01_002B9198) and queues func_L11_0030FBD0.
 * equiv: DIFFERENT only in compiler choices, each read by hand: branch polarity (bc1t/bc1f), 0x1190 kept
 * in a register, the +0x1E offset folded into the base, operand order of the base adds, and the
 * +0xBC stores (retail stores 1 / 2 from s1 / s0, which equiv's linear register tracking misreads). */

extern unsigned char D_L11_00161280_30F4F8 __asm__("D_L11_00161280") MACRO_ADDR;
extern unsigned char D_L11_00161281_30F4F8 __asm__("D_L11_00161281") MACRO_ADDR;
extern unsigned char D_L11_00161282_30F4F8 __asm__("D_L11_00161282") MACRO_ADDR;
extern unsigned char D_L11_00161283_30F4F8 __asm__("D_L11_00161283") MACRO_ADDR;
extern unsigned char D_L11_00161284_30F4F8 __asm__("D_L11_00161284") MACRO_ADDR;
extern unsigned char D_L11_00161285_30F4F8 __asm__("D_L11_00161285") MACRO_ADDR;
extern unsigned char D_L11_00161286_30F4F8 __asm__("D_L11_00161286") MACRO_ADDR;
extern int D_L11_00161288_30F4F8 __asm__("D_L11_00161288") MACRO_ADDR;
extern float D_L11_0016128C_30F4F8 __asm__("D_L11_0016128C") MACRO_ADDR;
extern float D_L11_00161290_30F4F8 __asm__("D_L11_00161290") MACRO_ADDR;
extern float D_L11_00161294_30F4F8 __asm__("D_L11_00161294") MACRO_ADDR;
extern int D_L11_00161350_30F4F8 __asm__("D_L11_00161350") MACRO_ADDR;
extern int D_L11_00161354_30F4F8 __asm__("D_L11_00161354") MACRO_ADDR;
extern int D_L11_00161358_30F4F8 __asm__("D_L11_00161358") MACRO_ADDR;
extern float D_L11_00167848_30F4F8 __asm__("D_L11_00167848") MACRO_ADDR;
extern int D_L11_00161FF4_30F4F8 SDATA(D_L11_00161FF4);
extern float D_0015EE70_30F4F8 __asm__("D_0015EE70") MACRO_ADDR;
extern float D_0015EE6C_30F4F8 __asm__("D_0015EE6C") MACRO_ADDR;
extern char D_L11_001DAB40_30F4F8[] __asm__("D_L11_001DAB40");
extern char D_L11_001DAA00_30F4F8[] __asm__("D_L11_001DAA00");
extern char D_L11_001CB500_30F4F8[] __asm__("D_L11_001CB500");
extern char D_L11_00180940_30F4F8[] __asm__("D_L11_00180940");
extern char D_L11_0021B7B0_30F4F8[] __asm__("D_L11_0021B7B0");

extern int func_001E9730_30F4F8() __asm__("func_001E9730");
extern void func_L01_002B8C00_30F4F8(void *, int) __asm__("func_L01_002B8C00");
extern void func_L01_002B90A8_30F4F8(float) __asm__("func_L01_002B90A8");
extern float func_L00_001FF860_30F4F8(float, float) __asm__("func_L00_001FF860");
extern float func_001F9F90_30F4F8(float) __asm__("func_001F9F90");
extern float func_001F9FA8_30F4F8(float) __asm__("func_001F9FA8");
extern void func_L01_002B8E20_30F4F8(float *, int *) __asm__("func_L01_002B8E20");
extern void func_L11_0030FBF8_30F4F8(int) __asm__("func_L11_0030FBF8");
extern void func_L01_0026F090_30F4F8(int, int) __asm__("func_L01_0026F090");
extern float func_00214D88_30F4F8(float *, float *, float, float, float, float) __asm__("func_00214D88");
extern void func_L11_0030FB58_30F4F8(char *) __asm__("func_L11_0030FB58");
extern int func_002140B0_30F4F8(int) __asm__("func_002140B0");
extern float func_002140F8_30F4F8(float, float) __asm__("func_002140F8");
extern void func_L00_002A5158_30F4F8(void *, int, int, float, float, float, float) __asm__("func_L00_002A5158");
extern int func_L01_00276680_30F4F8(char *, float) __asm__("func_L01_00276680");
extern void func_L01_002B9198_30F4F8(char *, int) __asm__("func_L01_002B9198");
extern void func_001F49B0_30F4F8(void *, void *) __asm__("func_001F49B0");
extern void func_L11_0030FBD0_30F4F8(void) __asm__("func_L11_0030FBD0");

#define ENT_30F4F8(d) (D_L11_001DAB40_30F4F8 + *(int *)((d) + 4) * 0x1190)

void func_L11_0030F4F8_m(char *moby) __asm__("func_L11_0030F4F8");
void func_L11_0030F4F8_m(char *moby) {
    char *data;
    float *P;
    float a;
    char *p;
    int k;
    int n;
    float f;
    float r;

    if (!(D_L11_00167848_30F4F8 <= 180.0f)) {
        D_L11_00161285_30F4F8 = 0x28;
        D_L11_00161283_30F4F8 = 0x40;
        D_L11_00161286_30F4F8 = 0x30;
        D_L11_00161280_30F4F8 = 0;
        D_L11_00161281_30F4F8 = 0x28;
        D_L11_00161282_30F4F8 = 0x30;
        D_L11_00161284_30F4F8 = 0;
    }
    data = *(char **)(moby + 0x78);

    switch (*(unsigned char *)(moby + 0x20)) {
    case 0:
        *(unsigned char *)(moby + 0x30) = 0xFF;
        n = D_L11_00161FF4_30F4F8;
        *(int *)(data + 4) = n;
        D_L11_00161FF4_30F4F8 = n + 1;
        if (n + 1 >= 0x80) {
            func_001E9730_30F4F8(D_L11_0021B7B0_30F4F8);
        }
        *(unsigned char *)(moby + 0x30) = 0xFF;
        *(unsigned short *)(moby + 0x34) = *(unsigned short *)(moby + 0x34) | 1;
        f = *(float *)(*(char **)(moby + 0x24) + 0x24);
        *(float *)(moby + 0x2C) = f + f;
        if (*(int *)(data + 4) == 0) {
            p = D_L11_001DAB40_30F4F8 + 0x14DCE;
            k = 0x13;
            do {
                *(unsigned short *)p = 0;
                k = k - 1;
                p = p - 0x1190;
            } while (k >= 0);
            func_L01_002B8C00_30F4F8(D_L11_001DAB40_30F4F8, 0x14);
            func_L01_002B90A8_30F4F8(1.0f);
            P = (float *)D_L11_001CB500_30F4F8;
            a = *(float *)(D_L11_00180940_30F4F8 + 0x10);
            f = *(float *)(D_L11_00180940_30F4F8 + 0x14);
            P[0] = 16.0f;
            P[3] = -8.0f;
            P[6] = 0.9f;
            P[8] = 0.1f;
            P[2] = -8.0f;
            P[5] = 1.0f;
            P[4] = 1.0f;
            P[1] = 16.0f;
            a = func_L00_001FF860_30F4F8(a, f);
            P[12] = func_001F9F90_30F4F8(a) * 0.57735f;
            P[13] = func_001F9FA8_30F4F8(a) * 0.57735f;
            *((unsigned char *)P + 0x3D) = 0x40;
            P[14] = -0.57735f;
            *(int *)(P + 10) = 0x38;
            *(int *)(P + 11) = 0x39;
            *((unsigned char *)P + 0x3C) = 0x1C;
            D_L11_00161350_30F4F8 = (int)D_L11_001DAB40_30F4F8;
            D_L11_00161354_30F4F8 = (int)D_L11_001DAA00_30F4F8;
            D_L11_00161358_30F4F8 = 0x14;
            D_L11_0016128C_30F4F8 = 32768.0f;
            D_L11_00161290_30F4F8 = 255.0f;
            D_L11_00161294_30F4F8 = 48.0f;
            D_L11_00161288_30F4F8 = 0;
        }
        *(float *)(ENT_30F4F8(data) + 0x0) = *(float *)(moby + 0x10);
        *(float *)(ENT_30F4F8(data) + 0x4) = *(float *)(moby + 0x14);
        *(float *)(ENT_30F4F8(data) + 0x8) = *(float *)(moby + 0x18);
        *(int *)(ENT_30F4F8(data) + 0x14) = 0x38;
        *(int *)(ENT_30F4F8(data) + 0x18) = 0x39;
        *(unsigned char *)(ENT_30F4F8(data) + 0x1C) = 0x1C;
        *(unsigned char *)(ENT_30F4F8(data) + 0x1D) = 0x44;
        *(unsigned short *)(ENT_30F4F8(data) + 0x1E) = 0xFFFF;
        func_L01_002B8E20_30F4F8((float *)ENT_30F4F8(data),
                                 (int *)(D_L11_001DAA00_30F4F8 + *(int *)(data + 4) * 16));
        *(unsigned char *)(moby + 0x20) = 1;
        break;
    case 1:
        n = *(int *)(data + 4);
        D_L11_00161FF4_30F4F8 = 0;
        func_L11_0030FBF8_30F4F8(n);
        *(unsigned char *)(moby + 0x20) = 2;
        break;
    case 2:
        if (*(unsigned char *)(moby + 0xBC) & 8) {
            *(int *)(data + 8) = 1;
            func_L01_0026F090_30F4F8(*(unsigned char *)(moby + 0x21), 3);
        }
        *(unsigned char *)(moby + 0xBC) = 2;
        if (*(int *)(data + 8) != 0) {
            func_00214D88_30F4F8((float *)(moby + 0x18), (float *)data, 137.0f,
                                 D_0015EE70_30F4F8 * 4.0f, D_0015EE70_30F4F8 * 4.0f, D_0015EE6C_30F4F8 * 4.0f);
            func_L11_0030FB58_30F4F8(moby);
        }
        if (func_002140B0_30F4F8(200) == 0) {
            r = func_002140F8_30F4F8(-4.0f, 4.0f);
            a = *(float *)ENT_30F4F8(data) + r;
            r = func_002140F8_30F4F8(-4.0f, 4.0f);
            func_L00_002A5158_30F4F8(ENT_30F4F8(data), 1, 1, a, *(float *)(ENT_30F4F8(data) + 4) + r, 1.0f, -0.03f);
        }
        break;
    case 3:
        if (*(unsigned char *)(moby + 0xBC) & 4) {
            *(int *)(data + 8) = 1;
            func_L01_0026F090_30F4F8(*(unsigned char *)(moby + 0x21), 2);
        }
        *(unsigned char *)(moby + 0xBC) = 1;
        if (*(int *)(data + 8) != 0) {
            func_00214D88_30F4F8((float *)(moby + 0x18), (float *)data, 132.36f,
                                 D_0015EE70_30F4F8 * 4.0f, D_0015EE70_30F4F8 * 4.0f, D_0015EE6C_30F4F8 * 4.0f);
            func_L11_0030FB58_30F4F8(moby);
        }
        if (func_002140B0_30F4F8(200) == 0) {
            r = func_002140F8_30F4F8(-4.0f, 4.0f);
            a = *(float *)ENT_30F4F8(data) + r;
            r = func_002140F8_30F4F8(-4.0f, 4.0f);
            func_L00_002A5158_30F4F8(ENT_30F4F8(data), 1, 1, a, *(float *)(ENT_30F4F8(data) + 4) + r, 1.0f, -0.03f);
        }
        break;
    }

    if (*(int *)(data + 4) >= 0) {
        f = 0.0f;
        if (func_L01_00276680_30F4F8(moby, 64.0f) != -1) {
            f = 1.0f;
        }
        if (f == 0.0f) {
            *(unsigned short *)(ENT_30F4F8(data) + 0x1E) = 0;
        } else {
            *(unsigned short *)(ENT_30F4F8(data) + 0x1E) = 0xFFFF;
        }
        if (*(int *)(data + 4) == 0) {
            func_L01_002B9198_30F4F8(D_L11_001DAB40_30F4F8, 0x14);
            func_001F49B0_30F4F8((void *)func_L11_0030FBD0_30F4F8, moby);
        }
    }
}
