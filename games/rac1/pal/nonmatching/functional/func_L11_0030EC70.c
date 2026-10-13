/* functional: high. Same reads, writes, calls and arguments in the same order as retail (the lq/sq pairs are 16-byte copies through qcopy); differs only in compiler choices (registers, branch layout, state 5 folded into the default, the child pointer reloaded around the qcopy asm, 0x13F450+0x2080 folded into one address), none of which changes a value read or written (_gp = 0x166D00). */
/* func_L11_0030EC70 -- src/overlays/l11_pokitaru/vendor_002CC828.c (functional C for the port, not a match)
 * Update for moby class 1157 (level 11), a state machine on m[0x20]:
 * 0: spawn a 4x6 table of class 0x4B7 children copying this moby's position/rotation, go to 1;
 * 1: wait for the linked moby (D_L11_00160058 + w60*0x100) to report 1 (start cutscene, go to 2) or 2 (go to 5);
 * 2/4: fade with an eased timer at d+0x78 into D_L11_0015F4FC; when it crosses zero call func_L00_00217718
 *      and (2) func_L00_002EBF50 on cuboid w64 or (4) func_L00_002EC0C8(3); at 1.0 go to 3/5;
 * 3: ease d+0x6C toward 1 and blend a position (func_001F9C08) and angles between cuboids w64 and w68
 *    (D_L11_0016016C, 0x80 each), handed to func_L00_002EBE88/func_L00_002EBEE0; at 1.0 go to 4.
 * Always ends with func_L11_0030F188(m).
 * Near miss fixed: undeclared D_L11_0015F4FC, short-typed gp globals, the missing func_L00_002EBE88(va) call
 * in state 3, and extern clashes with the host file (now uniquely suffixed). */

extern char *func_0020D348_30EC70(int) __asm__("func_0020D348");
extern void func_0022ED80_30EC70(int, int, void *) __asm__("func_0022ED80");
extern int func_001F9850_30EC70(int) __asm__("func_001F9850");
extern void func_00214D28_30EC70(float *, float, float) __asm__("func_00214D28");
extern float func_001F9B88_30EC70(float) __asm__("func_001F9B88");
extern void func_L00_00217718_30EC70(void *, void *, int, int) __asm__("func_L00_00217718");
extern void func_L00_002EBF50_30EC70(void *, void *, int, int, int) __asm__("func_L00_002EBF50");
extern float func_001FA790_30EC70(float, float) __asm__("func_001FA790");
extern float func_001FA748_30EC70(float, float) __asm__("func_001FA748");
extern void func_00214D88_30EC70(float *, float *, float, float, float, float) __asm__("func_00214D88");
extern void func_001F9C08_30EC70(void *, void *, void *, float) __asm__("func_001F9C08");
extern void func_001F9BC0_30EC70(void *) __asm__("func_001F9BC0");
extern void func_L00_002EBE88_30EC70(void *) __asm__("func_L00_002EBE88");
extern void func_L00_002EBEE0_30EC70(void *) __asm__("func_L00_002EBEE0");
extern void func_L00_002EC0C8_30EC70(int) __asm__("func_L00_002EC0C8");
extern void func_L11_0030F188_30EC70(void *) __asm__("func_L11_0030F188");

extern int D_L11_00160058_30EC70 __asm__("D_L11_00160058") MACRO_ADDR;
extern char *D_L11_0016016C_30EC70 __asm__("D_L11_0016016C") MACRO_ADDR;
extern float D_0015EE70_30EC70 __asm__("D_0015EE70") MACRO_ADDR;
extern float D_0015EE6C_30EC70 __asm__("D_0015EE6C") MACRO_ADDR;
extern char D_0013E633_30EC70[] __asm__("D_0013E633");
extern int D_L11_00161FE4_30EC70 SDATA(D_L11_00161FE4);
extern float D_L11_00161FE8_30EC70 SDATA(D_L11_00161FE8);
extern float D_L11_00161FEC_30EC70 SDATA(D_L11_00161FEC);
extern float D_L11_00161FF0_30EC70 SDATA(D_L11_00161FF0);
extern float D_L11_0015F4FC_30EC70 SDATA(D_L11_0015F4FC);

typedef struct {
    char *slot[4][6];
    int w60;
    int w64;
    int w68;
    float f6C;
    float f70;
    float f74;
    float f78;
} EcData_30EC70;

void func_L11_0030EC70_m(char *m) __asm__("func_L11_0030EC70");
void func_L11_0030EC70_m(char *m) {
    EcData_30EC70 *d = *(EcData_30EC70 **)(m + 0x78);
    float va[4] __attribute__((aligned(16)));
    float vb[4] __attribute__((aligned(16)));
    char *o;
    char *p;
    float prev;
    float e;
    int i;
    int k;

    switch (*(unsigned char *)(m + 0x20)) {
    case 0:
        for (i = 0; i < 4; i++) {
            char **pp = &d->slot[i][0];
            for (k = 5; k >= 0; k--) {
                *pp = func_0020D348_30EC70(0x4B7);
                *(short *)(*pp + 0x32) = 0x40;
                (*pp)[0x31] = 1;
                *(long *)(*pp + 0x38) =
                    *(long *)(*(char **)(D_0013E633_30EC70 + 0xE1D + 0x2080) + 0x38);
                *(unsigned short *)(*pp + 0x34) = *(unsigned short *)(m + 0x34);
                qcopy(*pp + 0x10, m + 0x10);
                qcopy(*pp + 0x40, m + 0x40);
                pp++;
            }
        }
        *(int *)&d->f6C = 0;
        m[0x20] = 1;
        break;
    case 1:
        o = (char *)(D_L11_00160058_30EC70 + (d->w60 << 8));
        if (*(unsigned char *)(o + 0xBC) == 1) {
            m[0x20] = 2;
            d->f78 = -1.0f;
            D_L11_00161FE4_30EC70 = 0;
            func_0022ED80_30EC70(0, 0, m);
        } else if (*(unsigned char *)(o + 0xBC) == 2) {
            d->f6C = 1.0f;
            m[0x20] = 5;
        }
        break;
    case 2:
        prev = d->f78;
        func_00214D28_30EC70(&d->f78, 1.0f, 1.0f / (float)func_001F9850_30EC70(0x14));
        e = func_001F9B88_30EC70(d->f78);
        D_L11_0015F4FC_30EC70 = 1.0f - e;
        if (prev < 0.0f && 0.0f <= d->f78) {
            o = (char *)(D_L11_00160058_30EC70 + (d->w60 << 8));
            func_L00_00217718_30EC70(o + 0x10, o + 0x40, 0x72, 1);
            p = D_L11_0016016C_30EC70 + (d->w64 << 7);
            func_L00_002EBF50_30EC70(p + 0x30, p + 0x70, 1, 0, 0);
            break;
        }
        if (1.0f <= d->f78) {
            m[0x20] = 3;
        }
        break;
    case 3: {
        float s = D_0015EE70_30EC70;
        float a = D_L11_00161FE8_30EC70 * s;
        float b = D_L11_00161FEC_30EC70 * s;
        float c = D_L11_00161FF0_30EC70 * D_0015EE6C_30EC70;
        d->f74 = d->f6C;
        func_00214D88_30EC70(&d->f6C, &d->f70, 1.0f, a, b, c);
        func_001F9C08_30EC70(va, D_L11_0016016C_30EC70 + (d->w64 << 7) + 0x30,
                             D_L11_0016016C_30EC70 + (d->w68 << 7) + 0x30, d->f6C);
        func_001F9BC0_30EC70(vb);
        vb[0] = func_001FA790_30EC70(*(float *)(D_L11_0016016C_30EC70 + (d->w68 << 7) + 0x70),
                                     *(float *)(D_L11_0016016C_30EC70 + (d->w64 << 7) + 0x70)) * d->f6C;
        vb[0] = func_001FA748_30EC70(*(float *)(D_L11_0016016C_30EC70 + (d->w64 << 7) + 0x70), vb[0]);
        vb[1] = func_001FA790_30EC70(*(float *)(D_L11_0016016C_30EC70 + (d->w68 << 7) + 0x74),
                                     *(float *)(D_L11_0016016C_30EC70 + (d->w64 << 7) + 0x74)) * d->f6C;
        vb[1] = func_001FA748_30EC70(*(float *)(D_L11_0016016C_30EC70 + (d->w64 << 7) + 0x74), vb[1]);
        vb[2] = func_001FA790_30EC70(*(float *)(D_L11_0016016C_30EC70 + (d->w68 << 7) + 0x78),
                                     *(float *)(D_L11_0016016C_30EC70 + (d->w64 << 7) + 0x78)) * d->f6C;
        vb[2] = func_001FA748_30EC70(*(float *)(D_L11_0016016C_30EC70 + (d->w64 << 7) + 0x78), vb[2]);
        func_L00_002EBE88_30EC70(va);
        func_L00_002EBEE0_30EC70(vb);
        if (1.0f <= d->f6C) {
            m[0x20] = 4;
            d->f78 = -1.0f;
        }
        break;
    }
    case 4:
        prev = d->f78;
        func_00214D28_30EC70(&d->f78, 1.0f, 1.0f / (float)func_001F9850_30EC70(0x14));
        e = func_001F9B88_30EC70(d->f78);
        D_L11_0015F4FC_30EC70 = 1.0f - e;
        if (prev < 0.0f && 0.0f <= d->f78) {
            func_L00_00217718_30EC70(D_0013E633_30EC70 + 0xE9D, D_0013E633_30EC70 + 0xE9D + 0x10, 0, 1);
            func_L00_002EC0C8_30EC70(3);
            break;
        }
        if (1.0f <= d->f78) {
            m[0x20] = 5;
        }
        break;
    default:
        break;
    }
    func_L11_0030F188_30EC70(m);
}
