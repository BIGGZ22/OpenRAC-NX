/* NON_MATCHING func_L06_002EA920 -- src/overlays/shared/vendor_002D9548.c
 * Best so far: SIZE ours 1200 / retail 1192, checked 2026-10-12.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Not a fair attempt: p0 to p3 were written from the assembly of func_L07_0031E6B0, which the packet printed by 
 *   - func_L06_002EA920 (1192 B): per-frame moby state update, switch on a state from func_L00_0025B4D0 (cases 1/2
 *   - Remaining differences (run 10 budget spent): `lw $6,0x204` read before the `addiu $2,-1` in retail (ours aft
 */
extern char *func_L00_0025B478(void *, int, int);
extern void func_L00_002584A8(void *, int, int);
extern int func_L00_0025B4D0(void *, void *, void *, int, int *, float *, int, int);
extern void func_L06_002EB8C8(char *m);
extern int func_0022ED80(int, int, int);
extern void func_0020D678(void *);
extern float func_001FA748(float, float);
extern float func_001F9FA8(float);
extern int func_001FA8A8(int, int, float);
extern float func_002140F8(float, float);
extern float func_001F9878(float);
extern int func_001FA898(float);
extern int func_001F9938(void *);
extern int func_L00_0025A778(void *, void *, int);
extern float func_001F9D48(void *, void *);
extern int func_L00_00260FB0(float, char *, void *, int, int, void *, int);
extern float func_001F9B88(float);
extern float D_L06_0015F660[] MACRO_ADDR;
extern float D_0015EE6C MACRO_ADDR;
extern int D_L06_0015F6B0 MACRO_ADDR;
extern int *D_L06_001B0FB0[];
extern char *D_L06_00160058_c __asm__("D_L06_00160058") MACRO_ADDR;
extern short D_L06_00161C94;
extern short D_L06_00161C98;
extern short D_L06_00161C9C;
extern char D_0013E633_c __asm__("D_0013E633") MACRO_ADDR;

// Per-frame update of a moby's state machine: steps its timers and picks the next state.
void func_L06_002EA920(char *m) {
    char *p = *(char **)(m + 0x78);
    char *r;
    int ev;
    int st;
    float zf;
    *(float *)(p + 0x30) = *(float *)(m + 0xE8) * 0.25f;
    r = 0;
    if (*(int *)(p + 0x20C) != 0) {
        r = func_L00_0025B478(*(void **)(p + 0x20C), 0x330000, 0);
    }
    if (r != 0 && (*(char **)(r + 0x20) == 0 || *(short *)(*(char **)(r + 0x20) + 0xA6) != 0xAF)) {
        func_L06_002EB5C8(m, D_L06_0015F660);
        func_L06_002EB360(m);
        func_L00_002584A8(m, 0, -1);
        func_0020D678(m);
        return;
    }
    zf = 0.0f;
    r = func_L00_0025B478(m, 0x330000, 0);
    st = func_L00_0025B4D0(m, r, p + 0x20, 0, &ev, &zf, 0, 4);
    if (ev != 1 && *(unsigned char *)(m + 0x20) != 0xE) {
        *(float *)(p + 0x20) = *(float *)(p + 0x20) - zf;
        if (*(float *)(p + 0x20) <= 0.0f) {
            st = 1;
        }
        switch (st) {
        case 0:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
            break;
        case 1:
        case 2:
            func_L00_002584A8(m, 0, -1);
            func_L06_002EB5C8(m, r + 0x10);
            if (*(unsigned char *)(p + 0x14E) == 1) {
                func_L06_002EB8C8(m);
                *(unsigned char *)(p + 0x14E) = 0;
            }
            func_0022ED80(3, 0, (int)m);
            if (*(int *)(p + 0x20C) != 0) {
                func_L06_002EB360(m);
            }
            if (*(short *)(p + 0x240) == 0) {
                func_0020D678(m);
                return;
            }
            {
                int v24 = *(short *)(p + 0x24);
                *(short *)(p + 0x240) = *(unsigned short *)(p + 0x240) - 1;
                *(float *)(p + 0x20) = (float)v24;
            }
            qcopy(m + 0x10, p + 0x210);
            m[0x20] = 0xF;
            *(int *)(m + 0x94) = 0;
            *(unsigned short *)(m + 0x34) = (*(unsigned short *)(m + 0x34) & 0xEFFF) | 0x41;
            break;
        }
    }
    *(unsigned char *)(m + 0xA4) = 0xFF;
    {
        float a = func_001FA748(*(float *)(p + 0x248), *(float *)&D_L06_00161C94 * 0.017453292f * D_0015EE6C);
        float b;
        *(float *)(p + 0x248) = a;
        b = func_001F9FA8(a);
        *(int *)(m + 0x90) = func_001FA8A8(*(int *)&D_L06_00161C98, *(int *)&D_L06_00161C9C, (b + 1.0f) * 0.5f);
    }
    if (*(int *)(p + 0x38) != 0) {
        float c = func_002140F8(180.0f, 240.0f);
        *(short *)(p + 0x1E8) = func_001FA898(func_001F9878(c));
        *(int *)(p + 0x38) = 0;
    }
    func_001F9938(p + 0x1E8);
    {
        float e = *(float *)(p + 0x1DC);
        if (*(short *)(p + 0x1E8) != 0) {
            e = e + 6.0f;
        }
        *(float *)(p + 0x1E0) = e;
    }
    {
        int id = *(int *)(p + 0x204);
        if (id != -1) {
            char *pool = D_L06_00160058_c + (id << 8);
            *(int *)(p + 0x1C4) = 1;
            *(char **)(p + 0x1C0) = pool;
            qcopy(p + 0x180, pool + 0x10);
        } else if (*(unsigned char *)((char *)&D_0013E633_c + 0x2EC1) == 1) {
            int t3 = D_L06_0015F6B0;
            int x = t3 + 7;
            if (id < t3) {
                x = t3;
            }
            if (t3 - ((x >> 3) << 3) == *(short *)(p + 0x20A)) {
                func_L06_002F4908(*(float *)(p + 0x1E0), m, p + 0x1C0);
                if (*(int *)(p + 0x1C0) != 0) {
                    int idx = *(int *)(p + 0x1F4);
                    char *e = (char *)D_L06_001B0FB0[idx];
                    int ok = func_L00_0025A778(*(char **)(p + 0x1C0) + 0x10, e + 0x10, *(int *)e);
                    if (ok == 0) {
                        *(int *)(p + 0x1C0) = 0;
                        *(int *)(p + 0x1C4) = 2;
                    } else if (*(short *)(*(char **)(p + 0x1C0) + 0xA6) != 0x359) {
                        *(int *)(p + 0x1C4) = 0;
                    } else {
                        float f = func_001F9D48(m + 0x10, *(char **)(p + 0x1C0) + 0x10);
                        if (2.0f < f) {
                            *(int *)(p + 0x1C0) = 0;
                            *(int *)(p + 0x1C4) = 2;
                        } else {
                            *(int *)(p + 0x1C4) = 0;
                        }
                    }
                } else {
                    *(int *)(p + 0x1C4) = 2;
                }
            }
        } else {
            int idx = *(int *)(p + 0x1F4);
            char *e = (char *)D_L06_001B0FB0[idx];
            int res = func_L00_00260FB0(*(float *)(p + 0x1E0), m, p + 0x180, 0, 0, e + 0x10, *(int *)e);
            if (res != 2) {
                float f = func_001F9D48(p + 0x210, p + 0x180);
                if (*(float *)(p + 0x1E0) < f) {
                    *(int *)(p + 0x1C4) = 2;
                } else {
                    float g = func_001F9B88(*(float *)(m + 0x18) - *(float *)(p + 0x188));
                    if (3.0f < g) {
                        *(int *)(p + 0x1C4) = 2;
                    }
                }
            }
        }
    }
    if (*(int *)(p + 0x1C0) == 0) {
        *(int *)(p + 0x1C0) = *(int *)((char *)&D_0013E633_c + 0x2E9D);
    }
}
