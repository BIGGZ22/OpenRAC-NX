/* Shared level code: func_L00_001ED6D8 in a file of its own, built with -fno-force-mem
 * (config/file_cflags.txt). GCC 2.95 takes options per translation unit,
 * and the functions of the file it came from do not build with this one
 * (docs/BUILD_FIDELITY.md, "Flags"). */
#include "common.h"
#include "include_asm.h"

typedef struct { char pad[0x254]; int p; char pad2[0x18]; short s; char pad3[1]; char c; char pad4[0x14]; float f0; char pad5[8]; float f1; } S;
extern S D_L00_00166D80;
extern char D_0013E633[];
extern void func_L00_001FF4B0(void *, void *, float);
extern char D_L00_00166FF0[];
typedef struct {
    char pad[0x74];
    int i74;
    float f78;
    char c7C, c7D;
    short s7E, s80, s82, s84, s86;
    char pad2;
    char c89;
    short s8A, s8C, s8E;
    char pad3[0x10];
} Elem;
extern Elem D_L00_00167250[];
extern int D_L00_00169990[];
extern int D_L00_0015F04C MACRO_ADDR;
extern int D_L00_0015F054 MACRO_ADDR;
extern short D_L00_0015F040;
extern char *D_L00_0015F050 MACRO_ADDR;
extern void func_001F9BC0(void *);
extern void func_L00_001ED600(void);

/* Initialises the camera slot tables and the camera state from the hero's transform. */
void Camera_InitSlots(void) {
    char *g = (char *)&D_L00_00166D80;
    char *g1;
    char *q;
    char *p1;
    char *base;
    int i;
    int k;

    *(int *)(g + 0x394) = 0;
    *(int *)(g + 0x398) = 0;
    *(float *)&D_L00_0015F040 = 0.75f;
    D_L00_0015F04C = 0;
    for (k = 0; k < 48; k++) {
        D_L00_00167250[k].s8E = 0;
        D_L00_00169990[k] = 0;
    }
    g1 = (char *)&D_L00_00166D80;
    base = D_0013E633 + 0xE1D;
    q = g1 + 0x190;
    p1 = g1 + 0x1B0;
    *(int *)(g1 + 0x184) = 0;
    *(short *)(g1 + 0x270) = 0;
    func_L00_001FF4B0(p1, (char *)(*(int *)(base + 0x2080) + 0xE0), 1.0f);
    qcopy(g1 + 0x1C0, p1);
    qcopy(g1 + 0x1D0, p1);
    clear_u64_value(g1 + 0x1E0);
    qcopy(g1 + 0x1F0, base + 0x80);
    {
        float f0 = *(float *)(base + 0x98);
        float f1 = *(float *)(base + 0x88);
        *(int *)(q + 0xC4) = 0;
        *(int *)(q + 0xCC) = 0;
        *(int *)(q + 0xC0) = 0;
        *(int *)(q + 0xC8) = 0;
        *(int *)(q + 0xD0) = 0;
        *(float *)(q + 0xBC) = f0;
        *(float *)(q + 0xB8) = f0;
        *(float *)(q + 0xB4) = f0;
        *(float *)(q + 0xB0) = f0;
        *(int *)(q + 0xD4) = 0;
        *(int *)(q + 0xDC) = 0;
        *(float *)(q + 0xAC) = f0;
        *(float *)(q + 0xD8) = f1;
    }
    i = 0;
    if (D_L00_0015F054 > 0) {
        do {
            int off = i * 32;
            char *e = D_L00_0015F050 + off;
            char *r = *(char **)(e + 0x1C);
            Elem *el = &D_L00_00167250[i];
            int h;
            el->s84 = i;
            *(Elem **)(r + 4) = el;
            el->s8E = 0;
            el->c89 = 0;
            el->s8A = 0;
            el->s82 = -1;
            el->s80 = -1;
            el->c7D = 0;
            el->s7E = 0;
            el->f78 = -1.0f;
            el->c7C = r[0x1C];
            el->i74 = (unsigned char)r[0x1F];
            h = *(unsigned short *)(e + 0xC);
            D_L00_00169990[i] = 1;
            el->s86 = h;
            el->s8C = Camera_FindTypeIndex((short)h);
            i++;
        } while (i < D_L00_0015F054);
    }
    CameraResetBehindHero();
    *(short *)D_L00_00166FF0 = 0;
    {
        int *q = (int *)(D_L00_00166FF0 + 0x110);
        q[0] = 0;
        q[3] = 0;
        q[1] = 0;
        q[4] = 0;
    }
}
