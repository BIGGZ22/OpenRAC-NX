/* functional: high. Carried from PAL's matched func_L07_00315528 and adapted: the third animation callback is FUN_L06_00249738 itself (US 0x00262078 in level 07) where PAL passes its counterpart + 0x20, and the state-7 exit tests hit == 4 where PAL tests hit & 4; the rest is the same instructions with US addresses. */
/* From the decompilation of Ratchet & Clank (PAL), src/overlays/l07_umbris/vendor_00313D28.c (games/rac1):
 * func_L07_00315528, matched C there; names and addresses carried to this version through the version
 * map (kind "near": the instructions differ), then adapted by hand to the US assembly. Functional C, not matched C. */
/* carry notes, checked against the US assembly:
 *   D_L07_00161BF4 -> D_L07_00161B64 (gp -0x509C), D_L07_0020C8C0 -> D_L07_0020C600: the US asm names these.
 *   The three callbacks for func_001FF308 (US 0x00314894..): 0x0025DBE8, 0x0025DD08 and 0x00262078 are
 *     FUN_L00_00235d80, FUN_L00_00235ea0 and FUN_L06_00249738 at their level-07 places (functions.tsv).
 *   func_L00_00222B80, func_L00_0025F4A8, func_L07_00314ED0 -> their "near" US counterparts: the same
 *     arguments at each call in the US asm.
 */

typedef int s32_c07_314150;
static __inline__ void qcopy_c07_314150(void *dst, void *src) {
    __asm__ __volatile__("lq $2,0x0(%1)\n\tsq $2,0x0(%0)" : : "r"(dst), "r"(src) : "$2", "memory");
}
static __inline__ void qcopy_nc_c07_314150(void *dst, void *src) {
    __asm__ __volatile__("lq $2,0x0(%1)\n\tsq $2,0x0(%0)" : : "r"(dst), "r"(src) : "$2");
}
static __inline__ void qzero_c07_314150(void *p) {
    __asm__ __volatile__("sq $0,0x0(%0)" : : "r"(p));
}
extern void func_00210850(void *, int, int *, void *);

void func_L07_00312DC8(char *moby);
extern s32_c07_314150 D_L07_0015F5CC ;
extern float func_001FA6C0(int);
extern int func_001FA6D0(float);
extern int func_001F9688(int);
extern float func_001F9DC8(float);
extern float func_001F9DE0(float);
extern void func_001F9A10(void *, void *, void *);
extern void func_L00_001FF500(void *, void *, float);
extern void func_001F9A10(void *, void *, void *);
extern void func_001F9A28(void *, void *, void *);
extern float func_002132A8(float, float);

void func_L07_00313920(float *out, float value);
extern void *func_L07_00313350(void *, void *, void *);
extern int func_L01_0028B510(void *, void *, float);
extern char *D_L07_001B0530[];

void func_L07_00313988(char *m, char *a, void *b, void *c);
extern unsigned char D_0013F350[] ;
extern float D_0015ED70 ;
extern float D_0015ED6C ;
extern int D_L07_0015F5CC ;
extern float func_001F9B80(void *, void *);
extern void func_001F9A28(void *, void *, void *);
extern void func_001F9A10(void *, void *, void *);
extern float func_L00_001FF8B0(float, float);
extern void func_L00_00258278(char *moby, float target, float *vel, float k, float d, float max);
extern void func_001F9A68(void *, void *, float);

int func_L07_00313AF0(char *m, char *a, void *p6, float *p7, float thr);
extern float func_L00_001FF8B0(float, float);
extern float func_001FA688(float, float);
extern int func_L00_00257B90(int, int);
extern void func_00212F90(void *, int, int, int);
extern float func_L00_001FF8B0(float, float);
extern float func_001FA5C8(float, float);

void func_L07_00313F60(char *moby, char *d, float *t);
extern int D_L07_0015F5C4 ;

void func_L07_00314058(char *arg);
extern float func_001F9AF0(void *);
extern void func_L00_001FF500(void *, void *, float);
extern void func_L07_00318E98(int arg0);
extern void func_L00_00263AC8(float, int, int, unsigned char *);
extern int func_00214720(void *, int);
extern float func_001F9B48(void *, void *);
extern void func_L00_0025A120(void *);
extern void func_L07_00313420(void *, void *, void *);
extern void func_L00_0025FCB8(void *, void *, float);
extern int func_L00_001FEFC8(void *);
extern int func_001F9770(void *);
extern float func_001F99C0(float);
extern int func_001FA6D0(float);
extern void func_L00_002DEFE0(int);
extern int func_0022DA68(int, int, int);
extern void func_L00_0028D918(int);
extern void func_001F4600(void (*)(), void *);
extern int func_001FF308(int, int, int, int, int, int, int);
extern void func_L05_0028BE78(int *, int, int);
extern void func_L00_002DDCA0(void *);
extern int func_001160D8(void);
extern int func_001F9740(int *);
extern int func_00213260(int);
extern void func_L07_0029B8D8(char *, int, float, float);
extern int func_L07_00313DE8(unsigned char *, unsigned char *, float *, int, int);
extern int func_L07_003122B0(void *, char *, float *);
extern int func_00214CC8(void *, float);
extern unsigned char *func_L07_0030DBB0(int, void *, int, float, float, float, float, float);
extern void func_L00_0024F7C8(void *, int, void *);
extern void func_L07_00313298_f_c07_314150(void *, void *, int, void *, short, unsigned int, int, float) __asm__("func_L07_00311EB8");
extern float func_00213308(void);
extern void func_00214DB0(void *, float, float, float);
extern void func_L07_0029C4E8(void *, void *);
extern void func_L07_003120D8(char *);
extern float func_00213508(void *, int, float);
extern float func_001F9DC8(float);
extern float func_001F9DE0(float);
extern float func_001F9AB0(void *, void *);
extern void func_L07_00313D30(void *, void *, float, float, float);
extern float func_0020C9E0(void *);
extern float func_001FA6C0(int);
extern unsigned char *func_L00_002715E8(void *, int, int, float, float);
extern void func_L00_0025D458(void *, short *);
extern void func_L00_00266858(void *, int);
extern void func_L00_00257470(void *, int, int);
extern void func_L00_002502A0(int);
extern void func_L00_00284E50(void *, void *);
extern void func_00214598(void *, void *);
extern float func_001FA580(float, float);
extern float func_L07_00317E60(void *, void *);
extern void func_L00_002223F8(int, int);
extern void func_L00_00257D78(float *, float, float);
extern int func_L00_0025D808(void *, void *, int *, float *, float, int);
extern float func_L00_0025ABF0(void *, void *, void *, float, float);
extern char *func_L00_002C52A8(void *, int, int, int);
extern void func_001FF570(int, int);
extern void func_L00_0025E450(void *, void *, void *, float, float, int, int, int, float, float, float, int, float, float, int, int, int, int);
extern void func_L06_002E6148(char *);
extern void func_L00_0023E838(int);
extern void func_0020C828(void *);
extern void func_L00_00259FE8(unsigned char *, float);
extern void func_L07_00312150(char *);
extern int func_001F9688(int);
extern void func_L07_00312420();
extern void func_L07_003131B0();
extern void FUN_L00_00235d80();
extern void FUN_L00_00235ea0();
extern void FUN_L06_00249738();
extern char D_L07_00166E40[];
extern float D_0015ED60 ;
extern char D_0013E550[];
extern char *D_L07_0015FFD8 ;
extern char *D_L07_001600EC ;
typedef struct { char pad[0x3C]; short s3C; char pad2[0xE]; } SbRec4C_c07_314150;
extern SbRec4C_c07_314150 D_L07_00179B40[];
extern int D_L07_0020C600[][4];
extern char D_0013D407[];
extern short D_L07_00161B64;
typedef struct { float f[4]; } SbVec_c07_314150 __attribute__((aligned(16)));
typedef float SbPVec_c07_314150[4] __attribute__((aligned(16)));
typedef struct { int n; int pad4[3]; SbPVec_c07_314150 pts[1]; } SbPath_c07_314150;
typedef struct {
    char pad0[0x108];
    float f108;
    char pad10C[0x160 - 0x10C];
    int i160;
    int i164;
    int i168;
    char pad16C[0x1D4 - 0x16C];
    unsigned char b1D4[4];
    int i1D8[6];
    char pad1F0[0x294 - 0x1F0];
    int i294;
} SbData_c07_314150;
typedef struct { SbVec_c07_314150 pos; SbVec_c07_314150 v10; SbVec_c07_314150 v20; SbVec_c07_314150 v30; void *moby; int kind; } SbTarget_c07_314150;
void FUN_L07_00314150(unsigned char *m) {
    SbVec_c07_314150 oldpos;
    SbTarget_c07_314150 tgt;
    SbVec_c07_314150 va;
    SbVec_c07_314150 vb;
    SbVec_c07_314150 vc;
    SbVec_c07_314150 ve;
    SbVec_c07_314150 vf;
    SbVec_c07_314150 vg;
    SbVec_c07_314150 vh;
    unsigned char *d = *(unsigned char **)(m + 0x78);
    int see;
    int t194;
    int t19A;
    int a;
    int b;
    int c;
    int e;
    float dist;
    float ang;
    float hdist;
    float hang;
    int i;
    float *t20;
    func_L00_00263AC8(2.5f, (int)m, 0xB, d + 0x2A0);
    func_L07_00314058((char *)m);
    if (D_L07_0015F5C4 == 2) return;
    if (*(int *)(d + 0x1A4) != -1 && !func_00214720(D_0013F350 + 0x80, *(int *)(d + 0x1A4))) return;
    if (m[0x31] != 0) {
        if (func_001F9B48(m + 0x10, D_L07_00166E40) < 35.0f) {
            func_L00_0025A120(m);
            m[0x7F] = 0x1C;
        }
    }
    qcopy_c07_314150(&oldpos, m + 0x10);
    t20 = (float *)(d + 0x20);
    func_L07_00313420(m, d, t20);
    see = 0;
    func_L00_0025FCB8(m, &tgt, 30.0f);
    dist = func_001F9B80(&tgt, m + 0x10);
    ang = func_L00_001FF8B0(tgt.pos.f[0] - *(float *)(m + 0x10), tgt.pos.f[1] - *(float *)(m + 0x14));
    hdist = func_001F9B80(m + 0x10, D_0013F350 + 0x80);
    {
        char *g = D_0013F350;
        hang = func_L00_001FF8B0(*(float *)(g + 0x80) - *(float *)(m + 0x10), *(float *)(g + 0x84) - *(float *)(m + 0x14));
    }
    if (func_L00_001FEFC8(d + 0x1F7)) {
        if ((func_001F9B80(m + 0x10, D_0013F350 + 0x80) > 8.0f && m[0x20] != 5)
            || (func_001F9B80(m + 0x10, D_0013F350 + 0x80) > 6.0f && (m[0x20] == 5 || m[0xBC] % 3U == 1))) {
            see = 1;
        }
    }
    t19A = 0;
    t194 = func_001F9770(d + 0x194) != 0;
    if (func_001F9770(d + 0x19A) && dist > 9.0f) t19A = 1;
    a = 0;
    if (d[0x1C7] != 0) {
        char *g = D_0013F350;
        if (tgt.moby == *(void **)(g + 0x2080) && dist < 30.0f
            && func_001F99C0(func_001FA5C8(ang, *(float *)(m + 0x48))) < 1.3962634f) {
            a = 1;
        }
    }
    b = 0;
    if (func_001F9770(d + 0x198)) {
        if (((m[0x20] == 5 || m[0xBC] % 3U == 1) && dist < 14.0f) || (m[0x20] != 5 && dist < 10.0f)) b = 1;
    }
    {
        char *g = D_0013F350;
        c = 0;
        e = 0;
        if (*(float *)(m + 0x18) + 1.0f < *(float *)(g + 0x2D8)) {
            if (*(char **)(g + 0x2FC) != 0 && *(short *)(*(char **)(g + 0x2FC) + 0xA6) >= 0x459
                && *(short *)(*(char **)(g + 0x2FC) + 0xA6) < 0x45D) {
                e = 1;
            }
        }
    }
    switch (m[0xBC]) {
    case 0:
        see = 0;
        a = 0;
        break;
    case 3:
        if (*(int *)(d + 0x1A8) != -1) {
            see = 1;
            c = 1;
            t194 = 0;
            b = 0;
        } else {
            see = 0;
        }
        a = 0;
        break;
    case 6:
        if (*(int *)(d + 0x1A8) != -1) {
            see = 1;
            c = 1;
            t194 = 0;
            b = 0;
        } else {
            see = 0;
        }
        break;
    case 1:
    case 4:
    case 7:
        t194 = 0;
        a = 0;
        if (m[0x20] != 1) t19A = 0;
        break;
    case 2:
    case 5:
    case 8:
        b = 0;
        t194 = 0;
        a = 0;
        t19A = 0;
        break;
    }
    *(int *)(d + 0x1B4) = func_001FA6D0(*(float *)(d + 0x20) * 16.0f) & 0xFFF0;
    if (*(int *)(d + 0x1B4) >= func_001FA6D0(((float)*(short *)(d + 0x24) - 0.5f) * 16.0f)) {
        *(int *)(d + 0x1B4) |= 0xF;
    }
    if (m[0xBC] % 3U == 0) {
        if (*(float *)(d + 0x20) > 0.5f) {
            int r = ((func_001FA6D0(*(float *)(d + 0x20)) % (*(short *)(d + 0x24) / 6)) * 16) / (*(short *)(d + 0x24) / 6);
            if (r < 0) r = 0;
            *(int *)(d + 0x1B4) += r & 0xF;
        } else {
            *(int *)(d + 0x1B4) = 0;
        }
    }
    if (m[0xBC] % 3U == 2) *(int *)(d + 0x1B4) += 0xF;
    if (m[0x20] != 8 && m[0x20] != 0) func_L00_002DEFE0((int)m);
    if (d[0x1C5] == 1) {
        if (*(int *)(d + 0x1F8) == -1) *(int *)(d + 0x1F8) = func_0022DA68(0xF, 4, (int)m);
        *(float *)(d + 0x1C8) += D_0015ED60 * 0.01f;
        if (*(float *)(d + 0x1C8) >= 1.0f) d[0x1C5] = 0;
        if (*(float *)(d + 0x1C8) > 1.0f) *(float *)(d + 0x1C8) = 1.0f;
    }
    if (d[0x1C5] == 2) {
        if (*(int *)(d + 0x1F8) == -1) *(int *)(d + 0x1F8) = func_0022DA68(0xF, 4, (int)m);
        *(float *)(d + 0x1C8) -= D_0015ED60 * 0.01f;
        if (*(float *)(d + 0x1C8) <= 0.0f) {
            if (*(int *)(d + 0x1F8) != -1) {
                char *sl = D_0013E550 + *(int *)(d + 0x1F8) * 0x70;
                if (*(void **)(sl + 0x88) == m && *(unsigned char *)(sl + 0x74) != 0) {
                    func_L00_0028D918(*(int *)(d + 0x1F8));
                }
            }
            d[0x1C5] = 0;
            *(int *)(d + 0x1F8) = -1;
        }
        if (*(float *)(d + 0x1C8) < 0.0f) *(float *)(d + 0x1C8) = 0.0f;
    }
    if (*(float *)(d + 0x1C8) > 0.0f) func_001F4600(func_L07_00312DC8, m);
    func_L07_00313F60((char *)m, (char *)d, (float *)(D_0013F350 + 0x80));
    if (m[0x20] != 0 && m[0x20] != 0xA && m[0x20] != 0x10) {
        *(int *)(d + 0x1B0) = func_001FF308(0x16, 0xFFFF, (int)FUN_L00_00235d80, (int)FUN_L00_00235ea0,
                                            (int)FUN_L06_00249738, (int)(d + 0x1B4),
                                            (*(short *)(d + 0x24) << 4) | 0xF);
    }
    switch (m[0x20]) {
    case 0:
        *(int *)&D_L07_00161B64 = 0;
        m[0x20] = 1;
        m[0xBC] = 0;
        *(short *)(d + 0x280) = -1;
        *(float *)(d + 0x190) = 0.0f;
        if (*(int *)(d + 0x18C) != -1) {
            int i;
            for (i = 0; i < *(int *)D_L07_001B0530[*(int *)(d + 0x18C)]; i++) {
                ((SbPath_c07_314150 *)D_L07_001B0530[*(int *)(d + 0x18C)])->pts[i][3] =
                    func_001F9B48(D_L07_001B0530[*(int *)(d + 0x18C)] + (i * 16 + 0x10),
                                  D_L07_001B0530[*(int *)(d + 0x18C)] + (((i + 1) % *(int *)D_L07_001B0530[*(int *)(d + 0x18C)]) * 16 + 0x10));
                *(float *)(d + 0x190) += ((SbPath_c07_314150 *)D_L07_001B0530[*(int *)(d + 0x18C)])->pts[i][3];
            }
        }
        if (*(int *)(d + 0x290) != -1) {
            char *tbl = D_L07_001B0530[*(int *)(d + 0x290)];
            int i;
            for (i = 0; i < *(int *)tbl; i++) {
                ((SbPath_c07_314150 *)tbl)->pts[i][3] = func_001F9B48(tbl + (i * 16 + 0x10), tbl + (((i + 1) % *(int *)tbl) * 16 + 0x10));
            }
        }
        if (*(int *)(d + 0x188) != -1) {
            int i;
            for (i = 0; i < *(int *)D_L07_001B0530[*(int *)(d + 0x188)]; i++) {
                int k = func_001FA6D0(((SbPath_c07_314150 *)D_L07_001B0530[*(int *)(d + 0x188)])->pts[i][3]);
                if ((unsigned int)k < 4) ((SbData_c07_314150 *)d)->b1D4[k] = i;
            }
        }
        *(int *)(d + 0xA8) &= ~8;
        *(unsigned short *)(m + 0x34) |= 0x1000;
        func_L05_0028BE78((int *)(d + 0x170), 3, *(int *)(d + 0x188));
        d[0x19C] = 7;
        m[0x30] = 0xFF;
        *(short *)(m + 0x32) = 0xFF;
        *(int *)(d + 0x1A8) = -1;
        *(int *)(d + 0x1B0) = -1;
        *(int *)(d + 0x1C0) = func_L01_0028B510(m + 0x10, D_L07_001B0530[*(int *)(d + 0x188)], 0.0f);
        func_L00_002DDCA0(m);
        *(float *)(m + 0x2C) *= 1.75f;
        d[0x1C7] = 0;
        *(float *)(d + 0x98) = 1.7453293f;
        d[0x29] = 1;
        d[0x58] = func_001FA6D0(30.0f);
        d[0x5A] = func_001FA6D0(40.0f);
        break;
    case 1:
        if (m[0x70] & 2) {
            if (m[0x53] != (func_001160D8() & 1)) {
                int r1 = func_001160D8() & 1;
                int r2 = func_L00_00257B90(0, 0xF);
                func_00212F90(m, r1, r2, func_L00_00257B90(0x1E, 0x2A));
            }
        }
        if (func_001F9740((int *)(d + 0x160))) {
            if (t19A) {
                *(int *)(d + 0x164) = 0;
                *(int *)(d + 0x160) = 0;
                if (m[0x53] != 5) {
                    int r = func_L00_00257B90(0, 2);
                    func_00212F90(m, 5, r, func_L00_00257B90(0x12, 0x14));
                }
                m[0x20] = 4;
            }
            if (t194) {
                if (m[0x20] != 5 || func_00213260(4) == 0) {
                    *(int *)(d + 0x164) = 0;
                    if (m[0x53] != 4) {
                        int r = func_L00_00257B90(0, 9);
                        func_00212F90(m, 4, r, func_L00_00257B90(0x14, 0x28));
                    }
                    m[0x20] = 3;
                    *(float *)(m + 0x58) = 1.0f;
                }
            }
            if (a) {
                if (m[0x53] != 6) {
                    int r = func_L00_00257B90(0, 2);
                    func_00212F90(m, 6, r, func_L00_00257B90(0xF, 0x19));
                }
                *(int *)(d + 0x164) = 0;
                *(float *)(d + 0x16C) = 0.0f;
                d[0x1C4] = 0;
                m[0x20] = 8;
                *(int *)(d + 0x294) = 0;
                *(int *)(d + 0x1F8) = func_0022DA68(9, 4, (int)m);
            }
            if (b) {
                if (m[0x53] != 9) {
                    int r = func_L00_00257B90(0, 2);
                    func_00212F90(m, 9, r, func_L00_00257B90(0x11, 0x19));
                }
                m[0x20] = 2;
            }
        }
        if (see && m[0x20] != 8) {
            if (m[0x53] != 0xA) {
                int r = func_L00_00257B90(0, 4);
                func_00212F90(m, 0xA, r, func_L00_00257B90(0x14, 0x1E));
            }
            *(float *)(m + 0x58) = 1.5f;
            *(int *)(d + 0x298) = 0;
            m[0x20] = 5;
        }
        break;
    case 2: {
        float rot;
        rot = *(float *)(m + 0x48);
        if (D_L07_0015F5CC % 6 == 0) func_L07_0029B8D8((char *)m, 0, 600000.0f, 5.0f);
        if ((D_L07_0015F5CC - 2) % 6 == 0) func_L07_0029B8D8((char *)m, 1, 600000.0f, 5.0f);
        if (func_L07_00313DE8(m, d, (float *)(D_0013F350 + 0x80), 9, 0)) break;
        func_L00_00258278((char *)m, ang, (float *)(d + 0x168), D_0015ED70 * 2.0943952f, D_0015ED70 * 0.6981317f, D_0015ED6C * 2.0943952f);
        if (func_001FA688(*(float *)(m + 0x48), rot) > D_0015ED6C * 0.17453292f) {
            if (m[0x53] != 0xD) {
                int r = func_L00_00257B90(0, 1);
                func_00212F90(m, 0xD, r, func_L00_00257B90(0xF, 0x16));
            }
        } else {
            if (m[0x53] != 9) {
                int r = func_L00_00257B90(0, 1);
                func_00212F90(m, 9, r, func_L00_00257B90(0xF, 0x16));
            }
        }
        if (m[0x52] == 9 || m[0x52] == 0xD) m[0x20] = func_L07_003122B0(m, (char *)d, (float *)&tgt);
        if ((m[0xBC] % 3U == 0 && func_001F9B80(&tgt, m + 0x10) > 13.0f)
            || (m[0xBC] % 3U == 1 && func_001F9B80(&tgt, m + 0x10) > 20.0f)) {
            *(short *)(d + 0x198) = func_001F9688(0x23);
            *(int *)(d + 0x160) = func_001F9688(0x1E);
            m[0x20] = 1;
        }
        break;
    }
    case 3:
        func_L00_00258278((char *)m, ang, (float *)(d + 0x168), D_0015ED70 * 1.5707964f, D_0015ED70 * 0.6981317f, D_0015ED6C * 1.0471976f);
        if (func_00214CC8(m, 31.0f) || func_00214CC8(m, 67.0f) || func_00214CC8(m, 101.0f)) {
            *(int *)(d + 0x164) += 1;
            qcopy_c07_314150(&va, m + 0x10);
            ((SbData_c07_314150 *)d)->i168 = 0;
            func_L07_0030DBB0((int)m, &va, 0x3A, 1.0f, 25.0f, 2.0f, D_0015ED6C * 8.0f, 1.0f);
            func_L00_0024F7C8(m, (*(int *)(d + 0x164) & 1) + 5, &va);
            func_L07_00313298_f_c07_314150(m, d, 1, &va, func_001F9688(0x1E), 0x800060FF, 0, 10.0f);
            va.f[2] = *(float *)(m + 0x18);
            for (i = 0; i < 55; i++) {
                float r1 = func_00213308();
                float r2 = func_002132A8(0.3926991f, 0.78539819f);
                func_00214DB0(&vb, D_0015ED6C * 5.0f, r1, r2);
                func_L00_001FF500(&vc, &vb, 0.75f);
                vc.f[2] += 0.1f;
                func_001F9A10(&vc, &vc, &va);
                *(int *)&vb.f[3] = 0;
                func_L07_0029C4E8(&vc, &vb);
            }
            func_L07_003120D8((char *)m);
        }
        if (*(int *)(d + 0x164) >= 3 && func_00214CC8(m, 51.0f)) {
            if (t19A) {
                *(short *)(d + 0x194) = 0xC0;
                *(int *)(d + 0x160) = 0xAA;
            } else {
                *(short *)(d + 0x194) = func_001F9688(0x3C);
                *(int *)(d + 0x160) = func_001F9688(0x37);
                if (*(short *)(d + 0x19A) <= func_001F9688(0x37)) *(short *)(d + 0x19A) += func_001F9688(0x3C);
            }
            *(float *)(m + 0x58) = 1.0f;
            if (m[0x53] != (func_001160D8() & 1)) {
                int r1 = func_001160D8() & 1;
                int r2 = func_L00_00257B90(0, 0xF);
                func_00212F90(m, r1, r2, func_L00_00257B90(0x1E, 0x2A));
            }
            m[0x20] = 1;
        }
        break;
    case 4:
        if (func_L07_00313DE8(m, d, (float *)(D_0013F350 + 0x80), 5, 0)) break;
        if (*(int *)(d + 0x160) < func_001F9688(0x3C) / 2) {
            func_L00_0024F7C8(m, 2, &va);
            func_L07_00313298_f_c07_314150(m, d, 2, &va, func_001F9688(0x1E), 0x800060FF, 0, 10.0f);
            if ((D_L07_0015F5CC & 7) == 0) func_L07_0029B8D8((char *)m, 2, 0.0f, 0.2f);
            if (((D_L07_0015F5CC - 4) & 7) == 0) func_L07_0029B8D8((char *)m, 3, 0.0f, 0.2f);
        }
        if (m[0x52] == 5) {
            qcopy_c07_314150(&va, &tgt);
            va.f[2] = func_00213508(&va, 0, 0.5f);
            *(int *)(d + 0x160) += 1;
            if (*(int *)(d + 0x164) == 0) {
                func_001F9A28(&vb, m + 0x10, &va);
                vb.f[2] = 0.0f;
                func_L00_001FF500(&vb, &vb, 1.0f);
                func_L00_001FF500(&vg, &vb, 0.075f);
                vc.f[0] = func_001F9DC8(tgt.v10.f[2]);
                vc.f[1] = func_001F9DE0(tgt.v10.f[2]);
                vc.f[2] = 0.0f;
                {
                    float s = func_001F9AB0(&vc, &vb);
                    func_L00_001FF500(&vh, &vb, s);
                }
                func_001F9A28(&vc, &vc, &vh);
                vc.f[2] = 0.0f;
                func_L00_001FF500(&ve, &vb, 8.0f);
                func_L00_001FF500(&vc, &vc, 4.0f);
                func_001F9A10(&ve, &ve, &va);
                func_001F9A28(&ve, &ve, &vc);
                func_L00_001FF500(&vc, &vc, 8.0f);
                func_001F9A10(&vc, &vc, &va);
                func_L00_001FF500(&vb, &vb, 2.0f);
                func_001F9A28(&vc, &vc, &vb);
                qcopy_c07_314150(&vh, D_0013F350 + 0x100);
                vh.f[2] = 0.0f;
                func_001F9A68(&vf, &vh, (float)func_001F9688(0x28));
                func_001F9A28(&vb, &vc, &ve);
                func_001F9A68(&vb, &vb, 0.5f);
                func_001F9A10(&vb, &vb, &ve);
                func_001F9A28(&vb, &vb, &va);
                func_001F9A28(&vb, &vb, &vg);
                func_001F9A28(&vb, &vf, &vb);
                func_001F9A10(&ve, &ve, &vb);
                func_001F9A10(&vc, &vc, &vb);
                qcopy_c07_314150(d + 0x110, &ve);
                qcopy_c07_314150(d + 0x120, &vc);
                qcopy_c07_314150(d + 0x140, &ve);
                func_001F9A28(d + 0x130, &vc, &ve);
                func_001F9A68(d + 0x130, d + 0x130, 1.0f / (float)func_001F9688(0x3C));
                qcopy_c07_314150(d + 0x150, d + 0x140);
                *(int *)(d + 0x15C) = 0;
                *(int *)(d + 0x164) = 1;
            } else {
                if (*(int *)(d + 0x160) < func_001F9688(0x3C)) {
                    func_001F9A10(d + 0x140, d + 0x140, d + 0x130);
                    func_001F4600(func_L07_00312420, m);
                    *(int *)(d + 0x164) = 2;
                } else {
                    *(int *)(d + 0x164) = 0;
                    *(short *)(d + 0x19A) = func_001F9688(0xF0);
                    *(int *)(d + 0x160) = func_001F9688(0xC8);
                    m[0x20] = 1;
                }
            }
        }
        break;
    case 5:
    case 6: {
        float thr = 2.0f;
        float rot = *(float *)(m + 0x48);
        float dd;
        func_L07_00313350(m, d, &vc);
        if (!c) {
            thr = 6.0f;
            if (*(int *)(d + 0x29C) != -1 && (d[0x19C] & 3) == 0 && func_00214720(&tgt, *(int *)(d + 0x29C))) {
                qcopy_c07_314150(&vb, D_L07_001B0530[*(int *)(d + 0x1A0)] + func_L01_0028B510(&tgt, D_L07_001B0530[*(int *)(d + 0x1A0)], 0.0f) * 16 + 0x10);
                dd = func_001F9B80(m + 0x10, &vb);
            } else {
                qcopy_c07_314150(&vb, &tgt);
                qcopy_c07_314150(&vb, D_L07_001B0530[*(int *)(d + 0x290)] + func_L01_0028B510(&vb, D_L07_001B0530[*(int *)(d + 0x290)], 0.0f) * 16 + 0x10);
                dd = func_001F9B80(m + 0x10, &vb);
                if (dd < *(float *)(d + 0x9C) + 1.0f) qcopy_c07_314150(&vb, &tgt);
            }
        } else {
            qcopy_c07_314150(&vb, D_L07_001B0530[*(int *)(d + 0x188)] + *(int *)(d + 0x1A8) * 16 + 0x10);
            dd = func_001F9B80(m + 0x10, &vb);
        }
        func_L07_00313D30(m, d, dd, thr, 1.5f);
        if ((*(int *)(d + 0x298) & 3) == 0) func_L07_00313988((char *)m, (char *)d, &vb, d + 0x140);
        qcopy_c07_314150(&va, d + 0x140);
        *(int *)(d + 0x298) += 1;
        if (*(float *)(d + 0x98) < func_001FA688(*(float *)(m + 0x48), func_L00_001FF8B0(va.f[0] - *(float *)(m + 0x10), va.f[1] - *(float *)(m + 0x14)))) {
            *(float *)(m + 0x58) = 1.0f;
            d[0x1F4] = m[0x20];
            d[0x1F6] = 0xA;
            d[0x1F5] = 1;
            qcopy_c07_314150(d + 0xC0, &va);
            if (m[0x53] != 3) {
                int r = func_L00_00257B90(0, 1);
                func_00212F90(m, 3, r, func_L00_00257B90(0xF, 0x16));
            }
            m[0x20] = 7;
        } else {
            int hit = func_L07_00313AF0((char *)m, (char *)d, &vb, (float *)&va, thr);
            float spd;
            int near;
            int moving;
            func_001FA688(*(float *)(m + 0x48), rot);
            spd = func_001F9AF0(d + 0x100);
            near = 0;
            if (!c && dist < 20.0f) near = 1;
            moving = D_0015ED6C * 0.01f < spd;
            if (hit == 4 || !moving) {  /* PAL: (hit & 4); US asm 0x00315A94 compares hit with 4 */
                d[0x1F7] = func_001F9688(0x5A);
                if (m[0x53] != (func_001160D8() & 1)) {
                    int r1 = func_001160D8() & 1;
                    int r2 = func_L00_00257B90(0, 9);
                    func_00212F90(m, r1, r2, func_L00_00257B90(0x14, 0x28));
                }
                *(int *)(d + 0x1A8) = -1;
                m[0x20] = 1;
                break;
            }
            if (m[0x53] != 0xA) {
                if (!near && (m[0x70] & 2)) func_00212F90(m, 0xA, 5, func_L00_00257B90(0xA, 0xD));
            } else {
                if (near && (m[0x70] & 2)) func_00212F90(m, 0xC, 5, func_L00_00257B90(0xA, 0xD));
            }
            if (func_001F9740((int *)(d + 0x160))) {
                if (t19A) {
                    *(int *)(d + 0x164) = 0;
                    *(int *)(d + 0x160) = 0;
                    *(float *)(m + 0x58) = 1.0f;
                    if (m[0x53] != 5) {
                        int r = func_L00_00257B90(0, 2);
                        func_00212F90(m, 5, r, func_L00_00257B90(0x14, 0x19));
                    }
                    m[0x20] = 4;
                }
                if (t194) {
                    *(int *)(d + 0x164) = 0;
                    if (m[0x53] != 4) {
                        int r = func_L00_00257B90(0, 5);
                        func_00212F90(m, 4, r, func_L00_00257B90(0x14, 0x28));
                    }
                    m[0x20] = 3;
                    *(float *)(m + 0x58) = 1.0f;
                }
                if (e) {
                    *(int *)(d + 0x164) = 0;
                    { char *g = D_0013F350; qcopy_c07_314150(d + 0x150, *(char **)(g + 0x2FC) + 0x10); }
                    *(float *)(d + 0x158) += 3.0f;
                    m[0x20] = 9;
                }
                if (a) {
                    *(float *)(m + 0x58) = 1.0f;
                    if (m[0x53] != 6) {
                        int r = func_L00_00257B90(0, 2);
                        func_00212F90(m, 6, r, func_L00_00257B90(0x14, 0x19));
                    }
                    *(int *)(d + 0x164) = 0;
                    *(float *)(d + 0x16C) = 0.0f;
                    d[0x1C4] = 0;
                    *(int *)(d + 0x294) = 0;
                    m[0x20] = 8;
                    *(int *)(d + 0x1F8) = func_0022DA68(9, 4, (int)m);
                }
            }
            if (!see) {
                *(float *)(m + 0x58) = 1.0f;
                if (m[0x53] != (func_001160D8() & 1)) {
                    int r1 = func_001160D8() & 1;
                    int r2 = func_L00_00257B90(0, 2);
                    func_00212F90(m, r1, r2, func_L00_00257B90(0x1C, 0x28));
                }
                m[0x20] = 1;
            }
            if (m[0x52] == 0xC) func_L07_003122B0(m, (char *)d, (float *)&tgt);
        }
        if (m[0x52] == 0xA || m[0x52] == 0xC) {
            if (func_00214CC8(m, 27.5f)) func_L07_003120D8((char *)m);
            if (func_00214CC8(m, 73.0f)) func_L07_003120D8((char *)m);
        }
        break;
    }
    case 7: {
        float diff;
        if (d[0x1F5] == 0) qcopy_c07_314150(d + 0xC0, D_0013F350 + 0x80);
        diff = func_001FA5C8(func_L00_001FF8B0(*(float *)(d + 0xC0) - *(float *)(m + 0x10), *(float *)(d + 0xC4) - *(float *)(m + 0x14)), *(float *)(m + 0x48));
        if (!b && (m[0x70] & 2)) {
            if (diff < 0.0001f) {
                if (m[0x53] != 3) func_00212F90(m, 3, 5, func_L00_00257B90(0xA, 0xD));
            } else {
                if (m[0x53] != 0xE) func_00212F90(m, 0xE, 5, func_L00_00257B90(0xA, 0xD));
            }
        }
        if (m[0x52] == 3 || m[0x52] == 0xE || m[0x52] == 0xD) {
            float dz = func_0020C9E0(m);
            if (dz < 14.5f || (dz > 19.0f && dz < 35.5f) || dz > 40.5f) {
                func_L00_00258278((char *)m, func_L00_001FF8B0(*(float *)(d + 0xC0) - *(float *)(m + 0x10), *(float *)(d + 0xC4) - *(float *)(m + 0x14)),
                                  (float *)(d + 0x168), D_0015ED70 * 2.0943952f, D_0015ED70 * 0.6981317f, D_0015ED6C * 2.0943952f);
            }
            if (diff > 0.001f && m[0x53] == 3) {
                func_00212F90(m, 0xE, 5, func_L00_00257B90(0xA, 0xD));
            } else if (diff < 0.001f && m[0x53] == 0xE) {
                func_00212F90(m, 3, 5, func_L00_00257B90(0xA, 0xD));
            }
        } else {
            *(int *)(d + 0x84) = 0;
        }
        if (func_001FA688(*(float *)(m + 0x48), func_L00_001FF8B0(*(float *)(d + 0xC0) - *(float *)(m + 0x10), *(float *)(d + 0xC4) - *(float *)(m + 0x14))) < *(float *)(d + 0x98) * 0.25f) {
            qzero_c07_314150(d + 0x100);
            if (m[0x53] != d[0x1F6]) {
                int anim = d[0x1F6];
                int r = func_L00_00257B90(0, 2);
                func_00212F90(m, anim, r, func_L00_00257B90(0xF, 0x1B));
            }
            m[0x20] = d[0x1F4];
        }
        if (m[0x52] == 3 || m[0x52] == 0xE) {
            if (func_00214CC8(m, 16.0f)) func_L07_003120D8((char *)m);
            if (func_00214CC8(m, 37.0f)) func_L07_003120D8((char *)m);
        } else if (m[0x52] == 0xD) {
            if (func_00214CC8(m, 13.0f)) func_L07_003120D8((char *)m);
            if (func_00214CC8(m, 34.0f)) func_L07_003120D8((char *)m);
        }
        break;
    }
    case 12: {
        int k;
        if (*(float *)(d + 0x20) <= func_001FA6C0(*(short *)(d + 0x24) / 6)) {
            if (m[0x53] != 8) func_00212F90(m, 8, 0x35, func_L00_00257B90(0xA, 0xD));
        } else {
            if (m[0x53] != 0xB) func_00212F90(m, 0xB, 0x3D, 1);
        }
        m[0x20] = 0xD;
        *(float *)(d + 0x108) = 0.0f;
        if (m[0xBC] % 3U < 2) m[0xBC] += 1;
        if (m[0xBC] % 3U < 2) m[0xBC] += 1;
        if (m[0xBC] >= 8) m[0xBC] = 8;
        if (((SbData_c07_314150 *)d)->i1D8[d[0x19D]] != -1) {
            int k2;
            k = d[0x19D];
            k2 = k + 3;
            if (((SbData_c07_314150 *)d)->i1D8[k2] != -1) {
                char *o1 = *(char **)(D_L07_0015FFD8 + (((SbData_c07_314150 *)d)->i1D8[k] << 8) + 0x78);
                char *o2 = *(char **)(D_L07_0015FFD8 + (((SbData_c07_314150 *)d)->i1D8[k2] << 8) + 0x78);
                if (*(int *)(o1 + 0x28) != -1 && *(int *)(o2 + 0x28) != -1) {
                    float d1 = func_001F9B80(D_0013F350 + 0x80, D_L07_001600EC + (*(int *)(o1 + 0x28) << 7) + 0x30);
                    int sel;
                    if (func_001F9B80(D_0013F350 + 0x80, D_L07_001600EC + (*(int *)(o2 + 0x28) << 7) + 0x30) < d1) {
                        sel = *(int *)(o2 + 0x28);
                        k = k2;
                    } else {
                        sel = *(int *)(o1 + 0x28);
                    }
                    *(float *)(m + 0x48) = func_L00_001FF8B0(*(float *)(D_L07_001600EC + sel * 0x80 + 0x30) - *(float *)(m + 0x10),
                                                             *(float *)(D_L07_001600EC + sel * 0x80 + 0x34) - *(float *)(m + 0x14));
                }
            }
            *(unsigned char *)(D_L07_0015FFD8 + ((SbData_c07_314150 *)d)->i1D8[k] * 0x100 + 0xBC) = 1;
        }
        break;
    }
    case 13: {
        int done = 0;
        if (func_0020C9E0(m) > 10.0f) {
            func_L00_0024F7C8(m, 0xA, &va);
            va.f[2] = 34.6f;
            if (func_00213260(0x1A) == 0) func_L00_002715E8(&va, 0, 0x40103080, 1.6f, 14700.0f);
        }
        if (m[0xBC] == 8) {
            float z = *(float *)(d + 0x108);
            func_001F9A68(d + 0x100, d + 0x100, 0.7f);
            ((SbData_c07_314150 *)d)->f108 = z;
            ((SbData_c07_314150 *)d)->f108 -= D_0015ED6C * 9.8f;
            func_001F9A10(m + 0x10, m + 0x10, d + 0x100);
            if (*(float *)(m + 0x18) < *(float *)(d + 0x1B8)) done = 1;
        } else {
            if (func_0020C9E0(m) > 79.0f) done = 1;
        }
        if (done) {
            *(int *)(d + 0x160) = func_001F9688(0xA);
            d[0x1C5] = 2;
            m[0x20] = 0xE;
            *(int *)(d + 0x164) = *(short *)(d + 0x24) / 6;
        }
        break;
    }
    case 14:
        func_001F9A68(d + 0x100, d + 0x100, 0.4f);
        func_001F9A10(m + 0x10, m + 0x10, d + 0x100);
        if (func_0020C9E0(m) < 168.0f) {
            func_L00_0024F7C8(m, 0xA, &va);
            va.f[2] = 34.6f;
            if (func_00213260(0x1A) == 0) func_L00_002715E8(&va, 0, 0x40103080, 1.6f, 14700.0f);
        }
        if (func_00214CC8(m, 168.0f)) {
            func_L00_0024F7C8(m, 5, &va);
            func_L07_0030DBB0((int)m, &va, 0x3A, 1.0f, 25.0f, 2.0f, D_0015ED6C * 8.0f, 1.0f);
        }
        if (func_00214CC8(m, 168.5f)) {
            func_L00_0024F7C8(m, 6, &va);
            func_L07_0030DBB0((int)m, &va, 0x3A, 1.0f, 25.0f, 2.0f, D_0015ED6C * 8.0f, 1.0f);
            func_L07_003120D8((char *)m);
        }
        if (*(int *)(d + 0x164) > 0) {
            if (func_001F9740((int *)(d + 0x160))) {
                *(int *)(d + 0x164) -= 1;
                *t20 -= 1.0f;
                d[0x67] = 0xFA;
                func_L00_0025D458(m, (short *)(d + 0x60));
                if (*t20 <= 0.0f) {
                    *t20 = 0.0f;
                    if (m[0x53] != 8) func_00212F90(m, 8, 0, func_L00_00257B90(0xA, 0xD));
                    if (*(int *)(d + 0x1F0) != -1) {
                        func_L00_00266858(m, 1);
                        func_L00_00257470(m, 0, -1);
                        func_L00_002502A0(m[0xB0]);
                        func_L00_00284E50(D_L07_001600EC + (*(int *)(d + 0x1F0) << 7) + 0x30, D_L07_001600EC + (*(int *)(d + 0x1F0) << 7) + 0x70);
                    }
                    m[0x20] = 0xA;
                } else {
                    if (*(int *)(d + 0x164) > 0) {
                        *(int *)(d + 0x160) = func_001F9688(0xA);
                    } else {
                        *(int *)(d + 0x160) = func_001F9688(0x14);
                    }
                }
            }
        } else {
            if (func_001F9740((int *)(d + 0x160))) {
                switch (d[0x19D]) {
                case 0:
                    qcopy_c07_314150(d + 0x120, D_L07_001B0530[*(int *)(d + 0x1A0)] + 0x10);
                    d[0x19C] &= 0xFE;
                    break;
                case 1:
                    qcopy_c07_314150(d + 0x120, D_L07_001B0530[*(int *)(d + 0x1A0)] + 0x20);
                    d[0x19C] &= 0xFD;
                    break;
                case 2: {
                    float d1 = func_001F9B80(D_0013F350 + 0x80, D_L07_001B0530[*(int *)(d + 0x1A0)] + 0x30);
                    if (func_001F9B80(D_0013F350 + 0x80, D_L07_001B0530[*(int *)(d + 0x1A0)] + 0x40) < d1) {
                        qcopy_c07_314150(d + 0x120, D_L07_001B0530[*(int *)(d + 0x1A0)] + 0x30);
                    } else {
                        qcopy_c07_314150(d + 0x120, D_L07_001B0530[*(int *)(d + 0x1A0)] + 0x40);
                    }
                    d[0x19C] &= 0xFB;
                    break;
                }
                }
                *(int *)(d + 0x1A8) = ((SbData_c07_314150 *)d)->b1D4[d[0x19D]];
                func_L05_0028BE78((int *)&va, (int)func_L07_00313350(m, d, &va), *(int *)(d + 0x188));
                m[0x20] = 0xF;
            }
        }
        break;
    case 15: {
        int iF0;
        if (func_0020C9E0(m) > 198.0f) {
            if (m[0x53] != 0xA) func_00212F90(m, 0xA, 0, 0);
            func_L00_0024F7C8(m, 0xA, &va);
            qcopy_c07_314150(d + 0xC0, &va);
            *(float *)(d + 0xC8) = *(float *)(d + 0x128);
            iF0 = 0xA;
            func_00210850(m, 1, &iF0, &vb);
            func_00214598(&vb, &vg);
            *(float *)(d + 0xCC) = func_001FA580(*(float *)(m + 0x48), vg.f[2]);
        }
        if (m[0x52] == 0xA) {
            qcopy_c07_314150(m + 0x10, d + 0xC0);
            *(float *)(m + 0x48) = *(float *)(d + 0xCC);
            if (((SbData_c07_314150 *)d)->i1D8[d[0x19D]] != -1) {
                *(unsigned char *)(D_L07_0015FFD8 + ((SbData_c07_314150 *)d)->i1D8[d[0x19D]] * 0x100 + 0xBC) = 0;
            }
            *(float *)(m + 0x58) = 1.5f;
            m[0xBC] += 1;
            *(int *)(d + 0x298) = 0;
            m[0x20] = 6;
            *(int *)(d + 0x160) = func_001F9688(0x37);
        }
        break;
    }
    case 8: {
        float fr;
        if (*(int *)(d + 0x294) <= 0) {
            if (func_L07_00313DE8(m, d, (float *)(D_0013F350 + 0x80), 6, 0)) break;
        }
        ((SbData_c07_314150 *)d)->i294 = 1;
        func_L00_00258278((char *)m, hang, (float *)(d + 0x168), D_0015ED70 * 2.0943952f, D_0015ED70 * 0.6981317f, D_0015ED6C * 2.0943952f);
        func_L00_0024F7C8(m, 4, &va);
        fr = func_L07_00317E60(m, &va);
        if (m[0x52] != 6) break;
        qcopy_c07_314150(d + 0x140, D_0013F350 + 0x80);
        *(float *)(d + 0x148) += 0.6f;
        if (*(float *)(d + 0x16C) < hdist) {
            ((SbData_c07_314150 *)d)->i160 = 0;
            *(float *)(d + 0x16C) += D_0015ED6C * 15.0f;
        } else {
            if (d[0x1C4] != 0 || (func_001F9740((int *)(d + 0x160)) && fr < 0.6f)) {
                int slot;
                char *g = D_0013F350;
                d[0x1C4] = 1;
                if (*(int *)(g + 0x2084) != 0x78) {
                    qcopy_c07_314150(d + 0x110, D_0013F350 + 0x80);
                    qcopy_c07_314150(d + 0x120, D_0013F350 + 0x80);
                    if (*(float *)(m + 0x18) < *(float *)(g + 0x88)) {
                        *(float *)(d + 0x128) = *(float *)(g + 0x88) + 2.0f;
                    } else {
                        *(float *)(d + 0x128) = *(float *)(m + 0x18) + 2.0f;
                    }
                    func_L00_002223F8(0x78, 1);
                    *(int *)(d + 0x164) = 0;
                    *(float *)(d + 0x1AC) = 20.0f;
                    *(short *)(d + 0x19E) = 0;
                }
                func_001F9A28(&vb, d + 0x120, d + 0x110);
                func_001F9A68(&vb, &vb, D_0015ED60 * 0.1f);
                func_001F9A10(d + 0x110, d + 0x110, &vb);
                qcopy_c07_314150(D_0013F350 + 0x1D30, d + 0x110);
                qcopy_c07_314150(D_0013F350 + 0x80, d + 0x110);
                {
                    char *g2 = D_0013F350;
                    *(float *)(g2 + 0x1D48) = func_001FA580(*(float *)(g2 + 0x1D48), D_0015ED6C * 25.132742f);
                    *(float *)(g2 + 0x98) = func_001FA580(*(float *)(g2 + 0x1D48), D_0015ED6C * 25.132742f);
                }
                *(float *)(d + 0x16C) = hdist;
                slot = D_L07_0020C600[*(int *)(d + 0x164)][0];
                if (slot == -1) {
                    *(int *)(d + 0x164) = 0;
                    *(short *)(d + 0x196) = func_001F9688(0x320);
                    *(int *)(d + 0x160) = func_001F9688(0xF0);
                    func_L07_00318E98((int)m);
                    { char *g3 = D_0013F350; if (*(int *)(g3 + 0x2084) == 0x78) func_L00_002223F8(0, 1); }
                    d[0x1C7] = 0;
                    if (*(int *)(d + 0x1F8) != -1) {
                        char *sl = D_0013E550 + *(int *)(d + 0x1F8) * 0x70;
                        if (*(void **)(sl + 0x88) == m && *(unsigned char *)(sl + 0x74) != 0) {
                            func_L00_0028D918(*(int *)(d + 0x1F8));
                        }
                    }
                    *(int *)(d + 0x1F8) = -1;
                    m[0x20] = 1;
                } else {
                    int cnt = (D_L07_00179B40 + slot)->s3C;
                    int rem;
                    int pos;
                    if (*(short *)(d + 0x19E) == 0) {
                        *(short *)(d + 0x19E) = *(int *)(D_0013D407 + 0x21 + slot * 4) / 8;
                        if (*(short *)(d + 0x19E) < cnt) *(short *)(d + 0x19E) = cnt;
                        *(float *)(d + 0x1AC) = func_002132A8(*(float *)(d + 0x1AC) + 20.0f, *(float *)(d + 0x1AC) + 100.0f);
                    }
                    cnt = *(short *)(d + 0x19E);
                    if (*(int *)(D_0013D407 + 0x21 + slot * 4) < cnt) {
                        cnt = *(int *)(D_0013D407 + 0x21 + slot * 4);
                        *(short *)(d + 0x19E) = 0;
                        *(int *)(d + 0x164) += 1;
                    }
                    rem = *(int *)(D_0013D407 + 0x21 + slot * 4) - cnt * 8;
                    pos = rem > 0;
                    cnt = cnt + pos * rem;
                    if (cnt > 0) {
                        float zv = 0.0f;
                        if (*(int *)(d + 0x18C) == -1) {
                            func_L00_00257D78((float *)&vb, D_0015ED6C * 4.0f, D_0015ED6C * 10.0f);
                            if (vb.f[2] < D_0015ED6C * 2.0f) vb.f[2] = func_002132A8(D_0015ED6C * 3.0f, D_0015ED6C * 6.0f);
                            func_L00_001FF500(&vc, &vb, 0.9f);
                            func_001F9A10(&vc, &vc, D_0013F350 + 0x80);
                        } else {
                            char *path = D_L07_001B0530[*(int *)(d + 0x18C)];
                            int pidx = 0;
                            float pfrac = 0.0f;
                            float d1;
                            float d2;
                            pidx = func_L01_0028B510(D_0013F350 + 0x80, path, 0.0f);
                            do {
                                *(float *)(d + 0x1AC) += 10.0f;
                                func_L00_0025D808(path, &ve, &pidx, &pfrac, *(float *)(d + 0x1AC), 1);
                                d1 = func_001F9B80(D_0013F350 + 0x80, &ve);
                                d2 = func_001F9B80(m + 0x10, &ve);
                            } while (d1 < 8.0f || d2 < 12.0f);
                            func_L07_00313920((float *)&ve, 0.5f);
                            func_001F9A28(&vc, &ve, D_0013F350 + 0x80);
                            zv = ve.f[2];
                            func_L00_001FF500(&vc, &vc, 0.9f);
                            qcopy_c07_314150(&vb, &vc);
                            vb.f[2] = 0.0f;
                            func_L00_001FF500(&vb, &vb, D_0015ED6C * 15.0f);
                            func_001F9A10(&vc, &vc, D_0013F350 + 0x80);
                            vb.f[2] = func_L00_0025ABF0(&vc, &ve, 0, D_0015ED6C * 15.0f, D_0015ED70 * -9.8f);
                            *(float *)(d + 0x1AC) += func_002132A8(0.7f, 2.0f);
                        }
                        func_0022DA68(0xB, 0, (int)m);
                        {
                            char *obj = func_L00_002C52A8(&vc, slot, cnt, 0);
                            if (obj != 0) {
                                qcopy_nc_c07_314150(*(char **)(obj + 0x78) + 0x10, &vb);
                                *(float *)(*(char **)(obj + 0x78) + 0x28) = zv;
                                *(int *)(D_0013D407 + 0x21 + slot * 4) -= cnt;
                                *(int *)(d + 0x160) = func_001F9688(0);
                            }
                        }
                    }
                }
            }
        }
        if (!a) {
            *(int *)(d + 0x164) = 0;
            *(short *)(d + 0x196) = func_001F9688(0xF);
            func_L07_00318E98((int)m);
            { char *g3 = D_0013F350; if (*(int *)(g3 + 0x2084) == 0x78) func_L00_002223F8(0, 1); }
            *(int *)(d + 0x160) = func_001F9688(0x46);
            d[0x1C7] = 0;
            if (*(int *)(d + 0x1F8) != -1) {
                char *sl = D_0013E550 + *(int *)(d + 0x1F8) * 0x70;
                if (*(void **)(sl + 0x88) == m && *(unsigned char *)(sl + 0x74) != 0) {
                    func_L00_0028D918(*(int *)(d + 0x1F8));
                }
            }
            *(int *)(d + 0x1F8) = -1;
            m[0x20] = 1;
        }
        if (b) {
            *(short *)(d + 0x196) = func_001F9688(0x1E);
            if (m[0x53] != 9) {
                int r = func_L00_00257B90(0, 2);
                func_00212F90(m, 9, r, func_L00_00257B90(0x10, 0x19));
            }
            func_L07_00318E98((int)m);
            { char *g3 = D_0013F350; if (*(int *)(g3 + 0x2084) == 0x78) func_L00_002223F8(0, 1); }
            if (*(int *)(d + 0x1F8) != -1) {
                char *sl = D_0013E550 + *(int *)(d + 0x1F8) * 0x70;
                if (*(void **)(sl + 0x88) == m && *(unsigned char *)(sl + 0x74) != 0) {
                    func_L00_0028D918(*(int *)(d + 0x1F8));
                }
            }
            *(int *)(d + 0x1F8) = -1;
            m[0x20] = 2;
        }
        break;
    }
    case 9:
        if (*(int *)(d + 0x164) <= 0) {
            if (func_L07_00313DE8(m, d, (float *)(d + 0x150), 7, 1)) break;
        }
        func_L00_00258278((char *)m, func_L00_001FF8B0(*(float *)(d + 0x150) - *(float *)(m + 0x10), *(float *)(d + 0x154) - *(float *)(m + 0x14)),
                          (float *)(d + 0x168), D_0015ED70 * 2.0943952f, D_0015ED70 * 0.6981317f, D_0015ED6C * 2.0943952f);
        switch (*(int *)(d + 0x164)) {
        case 0:
            if (m[0x53] != 7) func_00212F90(m, 7, 0, func_L00_00257B90(0xA, 0xD));
            *(int *)(d + 0x160) = 0;
            *(int *)(d + 0x164) = 1;
            break;
        case 1:
            if ((*(unsigned long *)(m + 0x50) & 0xFFFF0000) != 0x07070000 && m[0x53] != 7) {
                func_00212F90(m, 7, 0, func_L00_00257B90(0xA, 0xD));
            }
            if (m[0x52] == 7) {
                if (func_0020C9E0(m) >= 9.5f) {
                    func_L00_0024F7C8(m, 0, &vb);
                    func_L00_0024F7C8(m, 1, &vc);
                    func_001F9A10(&va, &vb, &vc);
                    func_001F9A68(&va, &va, 0.5f);
                    qcopy_c07_314150(d + 0x110, &va);
                    func_001F9A28(d + 0x130, d + 0x150, d + 0x110);
                    func_L00_001FF500(d + 0x130, d + 0x130, D_0015ED60);
                    *(int *)(d + 0x164) = 2;
                }
            }
            break;
        case 2:
            if ((*(unsigned long *)(m + 0x50) & 0xFFFF0000) != 0x07070000 && m[0x53] != 7) {
                func_00212F90(m, 7, 0, func_L00_00257B90(7, 0xA));
            }
            if (func_001F9740((int *)(d + 0x160))) {
                *(int *)(d + 0x160) = func_001F9688(0xA);
                func_L07_0029B8D8((char *)m, 0, 800000.0f, 5.0f);
                func_L07_0029B8D8((char *)m, 1, 800000.0f, 5.0f);
            }
            if (func_0020C9E0(m) >= 31.5f) {
                *(float *)(d + 0x16C) = 0.0f;
                *(int *)(d + 0x164) = 3;
                *(int *)(d + 0x11C) = 0;
            }
            break;
        case 3: {
            float r = func_001F9B80(m + 0x10, &tgt.v30);
            *(float *)(d + 0x16C) += D_0015ED60;
            if (r <= *(float *)(d + 0x16C)) *(int *)(d + 0x164) = 4;
            if (func_001F9740((int *)(d + 0x160))) *(int *)(d + 0x160) = func_001F9688(0x14);
            func_001F4600(func_L07_003131B0, m);
            break;
        }
        case 4:
            func_001F9A28(&va, d + 0x120, d + 0x110);
            func_L00_001FF500(&va, &va, D_0015ED60);
            func_001F9A10(d + 0x110, d + 0x110, &va);
            func_L07_00313298_f_c07_314150(m, d, 3, d + 0x110, func_001F9688(0x1E), 0x800060FF, 0, 10.0f);
            if (*(int *)(d + 0x164) != 0) {
                if (func_001F9740((int *)(d + 0x160))) *(int *)(d + 0x164) = 5;
            }
            func_001F4600(func_L07_003131B0, m);
            break;
        case 5:
            *(float *)(m + 0x58) = 1.0f;
            *(int *)(d + 0x164) = 0;
            *(int *)(d + 0x160) = 0;
            if (m[0x53] != (func_001160D8() & 1)) {
                int r1 = func_001160D8() & 1;
                int r2 = func_L00_00257B90(0, 7);
                func_00212F90(m, r1, r2, func_L00_00257B90(0x1B, 0x2D));
            }
            m[0x20] = 1;
            break;
        }
        break;
    case 10:
        if (*(int *)(d + 0x1B0) != -1) {
            func_001FF570(*(int *)(d + 0x1B0), 0);
            *(int *)(d + 0x1B0) = -1;
        }
        if (m[0x70] & 2) {
            func_L00_001FF500(d + 0x100, d + 0x100, 1.0f);
            *(float *)(d + 0x108) = 1.0f;
            qcopy_c07_314150(&va, m + 0x10);
            va.f[2] = *(float *)(d + 0x1B8) + 1.0f;
            func_L00_0025E450(m, d + 0x100, &va, 0.0f, 0.0f, 0x14, 0xC, 0x40, 7.0f, 5.0f, 10.0f, -1, 1.0f, 50.0f, 1, 1, -1, 0);
            *(unsigned short *)(m + 0x34) |= 0x41;
            if (*(int *)(d + 0x1CC) != -1) func_L06_002E6148(D_L07_0015FFD8 + (*(int *)(d + 0x1CC) << 8));
            m[0x20] = 0x10;
            *(float *)(m + 0x18) = 20.0f;
        }
        break;
    case 11:
        if (m[0x70] & 2) {
            if (e) {
                *(int *)(d + 0x164) = 0;
                { char *g = D_0013F350; qcopy_c07_314150(d + 0x150, *(char **)(g + 0x2FC) + 0x10); }
                *(float *)(d + 0x158) += 3.0f;
                m[0x20] = 9;
            } else {
                m[0x20] = 1;
            }
        }
        break;
    case 16:
        if (*(short *)(d + 0x280) != -1) func_L00_0023E838(*(short *)(d + 0x280));
        if (*(int *)(d + 0x1D0) == -1 || !(*(float *)(D_L07_0015FFD8 + (*(int *)(d + 0x1D0) << 8) + 0x18) < 34.5f)) {
            func_0020C828(m);
            return;
        }
        *(float *)(D_L07_0015FFD8 + (*(int *)(d + 0x1D0) << 8) + 0x18) +=
            (34.6f - *(float *)(D_L07_0015FFD8 + (*(int *)(d + 0x1D0) << 8) + 0x18)) * 0.1f;
        break;
    case 17:
        break;
    }
    func_001F9A28(d + 0x100, m + 0x10, &oldpos);
    if (m[0x20] < 0xA) func_L00_00259FE8(m, 2.5f);
    func_L07_00312150((char *)d);
}
extern int func_0022DA68(int, int, int);
extern void func_001F9A28(void *, void *, void *);
extern void func_001F9A10(void *, void *, void *);
extern void func_L00_001FF500(void *, void *, float);
extern void func_L00_002DEFE0(int arg0);
extern int func_001F9688(int);
extern void func_0020C828(void *);
