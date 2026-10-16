/* Carried from the PAL decompilation's src/overlays/l14_oltanis/vendor_002FF358.c (matched C there) (games/rac1) through the version
 * map (../../README.md): functional C, not matched C. Names and addresses are this version's. */
#include "types.h"
#include "qcopy.h"
#include "qzero.h"
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
/* What the PAL file this was written in declares before it (its unit, and its own file). */
extern void func_001F99F8(void *);
extern char *D_L14_001601AC_300468 __asm__("D_L14_001600EC") MACRO_ADDR;
extern int func_001F9688(int);
extern float func_001FA6C0(int);
extern float func_001F9B48(void *, void *);
extern float func_L00_001FF8B0(float, float);
extern char * D_L14_001B0BB0[];
extern char *D_L14_0015EF50 MACRO_ADDR;
typedef struct { char pad0[0x184]; char *p184; } Cam_316388_c314f00;
extern Cam_316388_c314f00 D_L14_00167480_c __asm__("D_L14_00167380");
extern void func_L00_00216F90(void *, void *, int, int);
typedef struct { char pad0[0xB0]; float fB0; } View_316388_c314f00;
extern View_316388_c314f00 D_L14_0016D240_v __asm__("D_L14_0016D140");
typedef int u128_316388_c314f00 __attribute__((mode(TI)));
typedef struct {
    int n;
    int pad[3];
    float pts[1][4];
} Path_316388_c314f00;
typedef struct {
    s32 seg;
    f32 t;
    f32 len_a;
    f32 len_b;
    f32 dist;
    s16 started;
    s16 timer;
} Run_316388_c314f00;
typedef struct {
    s16 seg[8];
    f32 fov[8];
    f32 fov_end;
} Stops_316388_c314f00;



/* Activate function of camera 20 (level 14): clears the record, builds the path tables and copies the camera vectors. */
void FUN_L14_00314f00(char *cam) {
    float v[4] __attribute__((aligned(16)));
    char *obj;
    char *data;
    Run_316388_c314f00 *q;
    Stops_316388_c314f00 *s;
    Path_316388_c314f00 *ent;
    char *p;
    char *p10, *p20, *p30;
    int i, k, m;
    float *row;
    short *so;
    float d, f0, f20, f21;

    obj = *(char **)(D_L14_0015EF50 + *(short *)(cam + 0x84) * 32 + 0x1C);
    obj[0x38] = 3;
    q = (Run_316388_c314f00 *)(*(char **)(cam + 0x70) + 0x40);
    q->seg = 0;
    q->t = 0;
    q->len_a = 0;
    q->len_b = 0;
    q->dist = 0;
    q->started = 0;
    q->timer = func_001F9688(*(short *)(obj + 0x3A));

    s = (Stops_316388_c314f00 *)(*(char **)(cam + 0x70) + 0x58);
    for (k = 0; k < 8; k++) {
        s->fov[k] = 0;
        s->seg[k] = -1;
    }

    p30 = cam + 0x30;
    p10 = cam + 0x10;
    p20 = cam + 0x20;
    s->fov_end = func_L00_001FF8B0(1.0f, D_L14_0016D240_v.fB0) * 2.0f;

    if (*(int *)(obj + 0x20) >= 0) {
        ent = (Path_316388_c314f00 *)D_L14_001B0BB0[*(int *)(obj + 0x20)];
        k = 0;
        for (i = 0; i < ent->n; i = m) {
            {
                row = (float *)((char *)ent + i * 16);
                f0 = row[7];
                if (0.0f < f0) {
                    s->fov[k] = f0;
                    s->seg[k] = i;
                    k++;
                }
                m = i + 1;
                d = func_001F9B48(ent->pts[i], ent->pts[m % ent->n]);
                row[7] = d;
                if (i < ent->n - 1) {
                    q->len_a += d;
                }
            }
        }
    }

    if (*(short *)(obj + 0x3A) > 0) {
        f0 = func_001FA6C0(func_001F9688(*(short *)(obj + 0x3A)));
        *(float *)(obj + 0x3C) = q->len_a / f0 * 60.0f;
    }

    if (*(int *)(obj + 0x24) >= 0) {
        ent = (Path_316388_c314f00 *)D_L14_001B0BB0[*(int *)(obj + 0x24)];
        for (i = 0; i < ent->n;) {
            {
                float *r;
                int n;
                n = i + 1;
                d = func_001F9B48(ent->pts[i], ent->pts[n % ent->n]);
                r = (float *)((char *)ent + i * 16);
                r[7] = d;
                if (i < ent->n - 1) {
                    q->len_b += d;
                }
                i = n;
            }
        }
    }

    f20 = 0.03f;
    f21 = 0.2f;
    data = *(char **)(cam + 0x70);
    *(float *)(data + 0) = f20;
    *(float *)(data + 4) = f21;
    func_001F99F8(data + 0x10);
    data = *(char **)(cam + 0x70);
    *(float *)(data + 0x20) = f20;
    *(float *)(data + 0x24) = f21;
    func_001F99F8(data + 0x30);

    p = D_L14_00167480_c.p184;
    qcopy(p30, p + 0x30);
    qcopy(cam, p);
    qcopy(p10, p + 0x10);
    qcopy(p20, p + 0x20);

    p = D_L14_001601AC_300468 + *(int *)(obj + 0x54) * 128 + 0x30;
    *(u128_316388_c314f00 *)v = 0;
    v[3] = 1.0f;
    func_L00_00216F90(p, v, 0x72, 0);
}
