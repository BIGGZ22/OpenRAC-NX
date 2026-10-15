/* NON_MATCHING func_L12_003078C8 -- src/overlays/l12_hoven/vendor_002EDAA0.c
 * Best so far: BYTES 22/832 (97.4% of the bytes match), checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   What it does: loops over 18 slots (D_L12_001FB948, 256-byte objects at D_L12_00160058); slot 17 is the moby it
 *   Still different from retail (p5):
 *   - 0x500 branch: retail sets the int arg ($6 = 0) before loading the float constants for func_L00_0025CCF0; our
 *   - 0x4FD branch: retail computes data + 16i first, then adds 0x454 (addu $a1,$s2,$s5 / addiu); ours does the re
 *   - Prologue/saved-register and stack-slot layout differs slightly from retail (retail keeps the counter at 0($s
 *   Would unblock: a way to keep the loop counter spilled while getting the 0x4FD address order, or a form that ma
 *   HQ s20 round (5 runs, p7-p11): best p8.c, now the same size as retail (832) and 22 bytes off. The 0x500 call s
 *   Unblock: a way to raise data's priority over the constant 17 without changing the code, or a form for the 0x4F
 */
extern void func_0020D678(void *);
extern float func_001FA748(float, float);
extern float func_L00_0025CCF0(void *, float, void *, int, float, float, float);
extern void func_L00_00251E30(void *);
extern int D_L12_001FB948[];
extern int D_L12_00160058_m __asm__("D_L12_00160058") MACRO_ADDR;
extern float D_L12_00161FC0 MACRO_ADDR;
extern float D_L12_00161FD0 MACRO_ADDR;
extern float D_L12_00161FE0 MACRO_ADDR;
extern float D_L12_00161FF0 MACRO_ADDR;
extern float D_0015EE6C MACRO_ADDR;

// Level 12 vendor: updates each of the 18 slot objects for one frame.
void func_L12_003078C8(char *moby) {
    int i;
    int off;
    char *data = *(char **)(moby + 0x78);
    for (i = 0, off = 0; i < 18; i++, off += 4) {
        int s = D_L12_001FB948[i];
        char *obj;
        float f1;
        if (s == 0) {
            continue;
        }
        if (i == 17) {
            obj = moby;
        } else {
            if (s == -1) {
                continue;
            }
            obj = (char *)(D_L12_00160058_m + (s << 8));
        }
        if (i == 17) {
            f1 = D_L12_00161FC0;
            if (f1 == -1.0f) {
                qcopy((char *)&D_L12_00161FC0, obj + 0x10);
                qcopy((char *)&D_L12_00161FD0, obj + 0x40);
            }
            qcopy((char *)&D_L12_00161FE0, obj + 0x10);
            qcopy((char *)&D_L12_00161FF0, obj + 0x40);
            qcopy(obj + 0x10, (char *)&D_L12_00161FC0);
            qcopy(obj + 0x40, (char *)&D_L12_00161FD0);
            f1 = *(float *)(obj + 0x18);
        } else {
            f1 = *(float *)(obj + 0x18);
        }
        if (f1 < 20.0f) {
            if (i == 17) {
                return;
            }
            func_0020D678(obj);
            D_L12_001FB948[i] = -1;
        }
        f1 = *(float *)(data + 0x338 + i * 16) - *(float *)(data + 0x570 + off);
        *(float *)(data + 0x338 + i * 16) = f1;
        *(float *)(obj + 0x18) = *(float *)(obj + 0x18) + f1;
        if (*(short *)(obj + 0xA6) == 0x500) {
            *(float *)(obj + 0x40) = func_001FA748(*(float *)(obj + 0x40), D_0015EE6C * 0.34906584f);
            func_L00_0025CCF0(obj + 0x44, 1.5707964f, data + 0x454 + i * 16, 0, 0.00200000009f, 0.3f, 0.0f);
        } else if (*(short *)(obj + 0xA6) == 0x4FD) {
            *(float *)(obj + 0x40) = func_001FA748(*(float *)(obj + 0x40), *(float *)(data + 0x450 + i * 16));
            func_L00_0025CCF0(obj + 0x44, -1.5707964f, data + 0x454 + i * 16, 0, *(float *)(data + 0x45C + i * 16), 0.3f, 0.0f);
        } else if (*(short *)(obj + 0xA6) == 0x4FA) {
            float t;
            *(float *)(obj + 0x40) = func_001FA748(*(float *)(obj + 0x40), D_0015EE6C * -0.87266463f);
            t = func_001FA748(*(float *)(obj + 0x44), *(float *)(data + 0x454 + i * 16));
            *(float *)(obj + 0x14) = *(float *)(obj + 0x14) + D_0015EE6C * -4.0f;
            *(float *)(obj + 0x44) = t;
        } else {
            *(float *)(obj + 0x14) = *(float *)(obj + 0x14) + D_0015EE6C * -4.0f;
            *(float *)(obj + 0x40) = func_001FA748(*(float *)(obj + 0x40), *(float *)(data + 0x450 + i * 16));
            *(float *)(obj + 0x44) = func_001FA748(*(float *)(obj + 0x44), *(float *)(data + 0x454 + i * 16));
        }
        if (i == 17) {
            qcopy((char *)&D_L12_00161FC0, obj + 0x10);
            qcopy((char *)&D_L12_00161FD0, obj + 0x40);
        }
        func_L00_00251E30(obj);
    }
}
