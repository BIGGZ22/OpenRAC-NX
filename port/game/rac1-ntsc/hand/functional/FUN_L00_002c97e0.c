/* functional: high. Carried from PAL's matched func_L00_002CABC0 and adapted: the US build drops PAL's else branch that arms the despawn timer (d->i70) while it is negative; the rest is the same instructions with US addresses (registers allocated differently). */
/* From the decompilation of Ratchet & Clank (PAL), src/overlays/shared/vendor_002C96D0.c (games/rac1):
 * func_L00_002CABC0, matched C there; names and addresses carried to this version through the version
 * map (kind "near": the instructions differ), then adapted by hand to the US assembly. Functional C, not matched C. */
/* carry notes, checked against the US assembly:
 *   0x002F2F2F is a colour (US asm 0x002C9C04: lui/ori 0x2F2F2F), not an address: the carry's
 *     0x002F1967 is put back.
 *   0x00830000 is a flag word, the same in both (US asm 0x002C9D2C).
 *   D_L00_00161920..3C -> D_L00_00161850..6C: the gp loads in the US asm name these.
 *   func_L00_0025F4A8 -> FUN_L00_0025e450: same arguments at the call (11 int, 7 float).
 */

typedef short s16_c00_2c97e0;
typedef int s32_c00_2c97e0;
typedef unsigned char u8_c00_2c97e0;
typedef unsigned short u16_c00_2c97e0;
typedef unsigned int u32_c00_2c97e0;
typedef unsigned long u64_c00_2c97e0;
typedef float f32_c00_2c97e0;
/* The quadword helpers in plain C: hostgen maps only the decompilation's own qcopy, qcopy_nc and
 * qzero to the runtime's, and wrote these renamed copies' inline assembly as nothing (every
 * copy through them was lost). Four words in order, as the lq/sq pair moves them. */
static __inline__ void qcopy_c00_2c97e0(void *dst, void *src) {
    int *d_ = (int *)dst, *s_ = (int *)src;
    d_[0] = s_[0]; d_[1] = s_[1]; d_[2] = s_[2]; d_[3] = s_[3];
}
typedef struct {
    f32_c00_2c97e0 x;
    f32_c00_2c97e0 y;
    f32_c00_2c97e0 z;
    f32_c00_2c97e0 w;
} Vec4f_c00_2c97e0;
typedef u32_c00_2c97e0 VecQuad_c00_2c97e0 __attribute__((mode(TI), aligned(16)));
typedef struct {
    f32_c00_2c97e0 x;
    f32_c00_2c97e0 y;
    f32_c00_2c97e0 z;
    f32_c00_2c97e0 w;
} __attribute__((aligned(16))) Vec4A_c00_2c97e0;
struct Moby_c00_2c97e0;
struct Manip_c00_2c97e0;
struct GifEntry_c00_2c97e0;
struct AnimSeq_c00_2c97e0;
struct MobyClass_c00_2c97e0 {
    u8_c00_2c97e0 pad_0[0xC];
    u8_c00_2c97e0 seq_count;
    u8_c00_2c97e0 pad_D[3];
    u32_c00_2c97e0 unk10;
    u8_c00_2c97e0 pad_14[0x8];
    void *unk1C;
    struct GifEntry_c00_2c97e0 *gifs;
    f32_c00_2c97e0 scale;
    s32_c00_2c97e0 unk28;
    void **callbacks;
    u8_c00_2c97e0 pad_30[0x14];
    u16_c00_2c97e0 flags;
    s16_c00_2c97e0 unk46;
    struct AnimSeq_c00_2c97e0 *seqs[1];
};
struct Moby_c00_2c97e0 {
    Vec4f_c00_2c97e0 bsphere;
    Vec4f_c00_2c97e0 pos;
    u8_c00_2c97e0 state;
    u8_c00_2c97e0 group;
    u8_c00_2c97e0 unk22;
    u8_c00_2c97e0 unk23;
    struct MobyClass_c00_2c97e0 *pclass;
    struct Moby_c00_2c97e0 *next;
    f32_c00_2c97e0 scale;
    u8_c00_2c97e0 unk30;
    u8_c00_2c97e0 unk31;
    s16_c00_2c97e0 unk32;
    u16_c00_2c97e0 flags;
    u16_c00_2c97e0 unk36;
    u64_c00_2c97e0 spawn_frame;
    Vec4f_c00_2c97e0 rot;
    u8_c00_2c97e0 frame;
    u8_c00_2c97e0 prev_frame;
    u8_c00_2c97e0 seq;
    u8_c00_2c97e0 prev_seq;
    f32_c00_2c97e0 unk54;
    f32_c00_2c97e0 unk58;
    u8_c00_2c97e0 pad5C[8];
    struct Manip_c00_2c97e0 *manips;
    void *cur_frame_data;
    void *prev_frame_data;
    u8_c00_2c97e0 unk70;
    u8_c00_2c97e0 unk71;
    u8_c00_2c97e0 unk72;
    u8_c00_2c97e0 unk73;
    void (*update)(struct Moby_c00_2c97e0 *moby);
    u8_c00_2c97e0 *pvars;
    u8_c00_2c97e0 unk7C;
    u8_c00_2c97e0 pad7D;
    u8_c00_2c97e0 unk7E;
    u8_c00_2c97e0 unk7F;
    s32_c00_2c97e0 unk80;
    u8_c00_2c97e0 pad84[2];
    s16_c00_2c97e0 unk86;
    u8_c00_2c97e0 pad88[8];
    s32_c00_2c97e0 unk90;
    u32_c00_2c97e0 unk94;
    s32_c00_2c97e0 unk98;
    u8_c00_2c97e0 pad9C[8];
    u8_c00_2c97e0 unkA4;
    u8_c00_2c97e0 padA5;
    s16_c00_2c97e0 oclass;
    u8_c00_2c97e0 padA8[8];
    u8_c00_2c97e0 unkB0;
    u8_c00_2c97e0 unkB1;
    u16_c00_2c97e0 save_id;
    s16_c00_2c97e0 unkB4;
    s16_c00_2c97e0 unkB6;
    void *unkB8;
    u8_c00_2c97e0 unkBC;
    u8_c00_2c97e0 padBD[3];
    Vec4f_c00_2c97e0 unkC0;
    Vec4f_c00_2c97e0 unkD0;
    Vec4f_c00_2c97e0 unkE0;
    u8_c00_2c97e0 padF0[0x10];
};
typedef struct Moby_c00_2c97e0 Moby_c00_2c97e0;
typedef struct MobyClass_c00_2c97e0 MobyClass_c00_2c97e0;
struct Moby_c00_2c97e0;
extern float atan2_CB_c00_2c97e0(float, float) __asm__("func_L00_001FF8B0");
extern int st_CB_c00_2c97e0(int) __asm__("func_001F9688");
extern int rr_CB_c00_2c97e0(int, int) __asm__("func_L00_00257B90");
extern void f264B40_CB_c00_2c97e0(void *, int, void *, float) __asm__("func_L00_00263AC8");
extern int t32_CB_c00_2c97e0(int *) __asm__("func_001F9740");
extern int t16_CB_c00_2c97e0(short *) __asm__("func_001F9770");
extern int rnd_CB_c00_2c97e0(int) __asm__("func_00213260");
extern float itof_CB_c00_2c97e0(int) __asm__("func_001FA6C0");
extern Moby_c00_2c97e0 *contact_CB_c00_2c97e0(void *, int, int) __asm__("func_L00_0025A420");
extern int sound_CB_c00_2c97e0(int, int, Moby_c00_2c97e0 *) __asm__("func_0022DA68");
extern float rfb_CB_c00_2c97e0(float, float) __asm__("func_002132A8");
extern float rang_CB_c00_2c97e0(void) __asm__("func_00213308");
extern void sph_CB_c00_2c97e0(void *, float, float, float) __asm__("func_00214DB0");
extern void vadd_CB_c00_2c97e0(void *, void *, void *) __asm__("func_001F9A10");
extern void f26DD70_CB_c00_2c97e0(void *, void *, int, int, float, int) __asm__("func_L00_0026CED0");
extern int ftoi_CB_c00_2c97e0(float) __asm__("func_001FA6D0");
extern void f25F4A8_CB_c00_2c97e0(void *, void *, void *, float, float, int, int, int, float, float, float, float, int, float, int, int, int, int) __asm__("func_L00_0025E450");
extern int f1F2BE8_CB_c00_2c97e0(void *, float, int, void *, void *) __asm__("func_L00_001F2868");
extern void f25BA50_CB_c00_2c97e0(Moby_c00_2c97e0 *, void *, void *, int, int, int, int, int, float, float, float) __asm__("func_L00_0025A9F8");
extern char *debris_CB_c00_2c97e0(void *, void *, void *) __asm__("func_L00_002CBF68");
extern float dist_CB_c00_2c97e0(void *, void *) __asm__("func_001F9B80");
extern float cos_CB_c00_2c97e0(float) __asm__("func_001F9DC8");
extern float sin_CB_c00_2c97e0(float) __asm__("func_001F9DE0");
extern int f2594C8_CB_c00_2c97e0(void *, void *, void *, int, float, float, float, float) __asm__("func_L00_00258490");
extern float angdiff_CB_c00_2c97e0(float, float) __asm__("func_001FA688");
extern float dist3_CB_c00_2c97e0(void *, void *) __asm__("func_001F9B48");
extern void f2592B0_CB_c00_2c97e0(void *, float *, float, float, float, float) __asm__("func_L00_00258278");
extern float ground_CB_c00_2c97e0(void *, int, float) __asm__("func_00213508");
extern float fabs_CB_c00_2c97e0(float) __asm__("func_001F99C0");
extern int f2DDEA0_CB_c00_2c97e0(void *, void *) __asm__("func_L00_002DC9F0");
extern void vsub_CB_c00_2c97e0(void *, void *, void *) __asm__("func_001F9A28");
extern float vlen_CB_c00_2c97e0(void *) __asm__("func_001F9AF0");
extern void vscale_CB_c00_2c97e0(void *, void *, float) __asm__("func_L00_001FF500");
extern float f25A748_CB_c00_2c97e0(void *) __asm__("func_L00_00259710");
extern int f1F3958_CB_c00_2c97e0(void) __asm__("func_L00_001F35D8");
extern float sqrt_CB_c00_2c97e0(float) __asm__("func_001F9988");
extern float angsub_CB_c00_2c97e0(float, float) __asm__("func_001FA5C8");
extern float angadd_CB_c00_2c97e0(float, float) __asm__("func_001FA580");
extern void del_CB_c00_2c97e0(Moby_c00_2c97e0 *) __asm__("func_0020C828");
extern void anim_CB_c00_2c97e0(Moby_c00_2c97e0 *, int, int, int) __asm__("func_00212F90");
extern float F6B4_CB_c00_2c97e0 __asm__("D_L00_0015F5D0") ;
extern float F6B8_CB_c00_2c97e0 __asm__("D_L00_0015F5D4") ;
extern float E6C_CB_c00_2c97e0 __asm__("D_0015ED6C") ;
extern float E70_CB_c00_2c97e0 __asm__("D_0015ED70") ;
extern float E60_CB_c00_2c97e0 __asm__("D_0015ED60") ;
extern int G1920_CB_c00_2c97e0 __asm__("D_L00_00161850" "__gp") ;
extern float G1928_CB_c00_2c97e0 __asm__("D_L00_00161858" "__gp") ;
extern float G192C_CB_c00_2c97e0 __asm__("D_L00_0016185C" "__gp") ;
extern float G1930_CB_c00_2c97e0 __asm__("D_L00_00161860" "__gp") ;
extern float G1934_CB_c00_2c97e0 __asm__("D_L00_00161864" "__gp") ;
extern float G1938_CB_c00_2c97e0 __asm__("D_L00_00161868" "__gp") ;
extern float G193C_CB_c00_2c97e0 __asm__("D_L00_0016186C" "__gp") ;
extern float D_L00_00173F40_CB_c00_2c97e0[] __asm__("D_L00_00173E40");
extern char D_L00_00178000_CB_c00_2c97e0[] __asm__("D_L00_00177F00");
extern char D_0013F450_CB_c00_2c97e0[] __asm__("D_0013F350");
extern unsigned char D_0013E633_CB_c00_2c97e0[] __asm__("D_0013E533");
typedef struct {
    u8_c00_2c97e0 pad0[0x10];
    void *p10;
    char *p14;
    u8_c00_2c97e0 pad18[8];
    u8_c00_2c97e0 v20[0x44];
    short h64;
    short h66;
    float f68;
    float f6C;
    int i70;
    int i74;
    float f78;
    int i7C;
} Vars_2CABC0_c00_2c97e0;
void FUN_L00_002c97e0(Moby_c00_2c97e0 *m) {
    Vec4A_c00_2c97e0 T;
    Vec4A_c00_2c97e0 v;
    Vec4A_c00_2c97e0 W;
    Vec4A_c00_2c97e0 V;
    Vars_2CABC0_c00_2c97e0 *d;
    char *h;
    float save58;
    float ang;
    float s;
    float speed;
    float r;
    float a;
    int hit;
    int i;
    int j;
    int n;
    Moby_c00_2c97e0 *o;
    hit = 0;
    h = D_0013F450_CB_c00_2c97e0;
    d = (Vars_2CABC0_c00_2c97e0 *)m->pvars;
    save58 = m->unk58;
    ang = atan2_CB_c00_2c97e0(m->pos.x - *(float *)(h + 0x80), m->pos.y - *(float *)(h + 0x84));
    if (m->state == 0) {
        m->state = 1;
        d->h66 = 0;
        d->i70 = -1;
        d->h64 = rr_CB_c00_2c97e0(st_CB_c00_2c97e0(0x78), st_CB_c00_2c97e0(0x12C));
        d->f78 = 1.0f;
    }
    if (m->state == 6) {
        return;
    }
    if (D_0013E633_CB_c00_2c97e0[2] != 0) {
        f264B40_CB_c00_2c97e0(m, G1920_CB_c00_2c97e0, d->v20, 5.0f);
    } else {
        f264B40_CB_c00_2c97e0(m, G1920_CB_c00_2c97e0, d->v20, 7.7f);
    }
    qcopy_c00_2c97e0(&T, &m->pos);
    if (d->i70 >= 0 && t32_CB_c00_2c97e0(&d->i70) != 0 && m->unk31 == 0) {
        del_CB_c00_2c97e0(m);
        return;
    }
    if (d->i70 >= 0) {
        if ((0.9f < F6B8_CB_c00_2c97e0 || 0.9f < F6B4_CB_c00_2c97e0) && m->unk31 == 0 && d->i70 != -1) {
            if (rnd_CB_c00_2c97e0(0xC7) == 0) {
                del_CB_c00_2c97e0(m);
                return;
            }
            d->i70 = rr_CB_c00_2c97e0(0x14, 0x1E);
        }
        if (1.0f < F6B8_CB_c00_2c97e0 || 1.0f < F6B4_CB_c00_2c97e0) {
            if (m->unk31 != 0) {
                if (rnd_CB_c00_2c97e0(0x63) == 0) {
                    d->f78 = 0.0f;
                } else {
                    d->i70 = rr_CB_c00_2c97e0(0x14, 0x1E);
                }
            } else if (rnd_CB_c00_2c97e0(0x18) == 0) {
                del_CB_c00_2c97e0(m);
                return;
            }
        }
    }
    /* PAL also arms the timer here when d->i70 < 0 (0x1E..0x32 frames, or 0 on a 1-in-9 roll);
       the US build has no such branch (US asm 0x002C9900/0x002C9928 skip straight to 0x002C9A1C). */
    if (d->i74 > 0) {
        m->scale += ((itof_CB_c00_2c97e0(d->i74) * 3.0f + 1.0f) * m->pclass->scale - m->scale) * 0.1f;
    }
    s = m->scale / m->pclass->scale;
    if (t32_CB_c00_2c97e0(&d->i7C) != 0) {
        o = contact_CB_c00_2c97e0(m, -1, 0);
        if (o != 0) {
            sound_CB_c00_2c97e0(1, 0, m);
            d->f78 -= o->scale;
            d->i7C = st_CB_c00_2c97e0(0x19);
        }
    }
    m->unkA4 = 0xFF;
    if (d->f78 < 1.0f) {
        if (d->i74 == 0) {
            for (i = 0; i < ftoi_CB_c00_2c97e0(s) * 20; i++) {
                r = rfb_CB_c00_2c97e0(0.0f, 0.37f) * s;
                a = rang_CB_c00_2c97e0();
                sph_CB_c00_2c97e0(&V, r, a, rang_CB_c00_2c97e0());
                V.z += rfb_CB_c00_2c97e0(0.25f, 0.5f) * s;
                vadd_CB_c00_2c97e0(&V, &V, &T);
                r = rfb_CB_c00_2c97e0(E6C_CB_c00_2c97e0 * 0.1f, E6C_CB_c00_2c97e0) * s;
                a = rang_CB_c00_2c97e0();
                sph_CB_c00_2c97e0(&W, r, a, rang_CB_c00_2c97e0());
                f26DD70_CB_c00_2c97e0(&V, &W, 0x3F7F7F7F, 0x2F2F2F, s * 150000.0f, rr_CB_c00_2c97e0(st_CB_c00_2c97e0(0x1E), st_CB_c00_2c97e0(0x39)));
            }
        } else {
            *(VecQuad_c00_2c97e0 *)&V = 0;
            V.x = E60_CB_c00_2c97e0 * 0.5f;
            V.z = E60_CB_c00_2c97e0 * 0.7f;
            V.y = E60_CB_c00_2c97e0 * 0.5f;
            *(VecQuad_c00_2c97e0 *)&W = *(VecQuad_c00_2c97e0 *)&V;
            f25F4A8_CB_c00_2c97e0(m, &W, 0, 0.0f, 2.0f, 0, 0, 0, 4.0f, 3.0f, 9.0f, 1.0f, -1, 12.0f, 1, 1, -1, (unsigned char)d->i74);
            n = f1F2BE8_CB_c00_2c97e0(&m->pos, 3.0f, 0x10, m, 0);
            *(VecQuad_c00_2c97e0 *)&V = *(VecQuad_c00_2c97e0 *)&m->pos;
            f25BA50_CB_c00_2c97e0(m, &V, D_L00_00178000_CB_c00_2c97e0, n, 0, 0x830000, 2, 1, 1.0f, 1.0f, 1.0f);
        }
        n = rr_CB_c00_2c97e0(5, 8);
        if (0.85f < F6B4_CB_c00_2c97e0 || 0.85f < F6B8_CB_c00_2c97e0) {
            if (1.0f < F6B4_CB_c00_2c97e0 || 1.0f < F6B8_CB_c00_2c97e0) {
                n = 1;
            } else {
                n /= 2;
            }
        }
        for (j = 0; j < n; j++) {
            r = rfb_CB_c00_2c97e0(G1938_CB_c00_2c97e0, G193C_CB_c00_2c97e0) * s;
            a = rang_CB_c00_2c97e0();
            sph_CB_c00_2c97e0(&V, r, a, rang_CB_c00_2c97e0());
            V.z += G1934_CB_c00_2c97e0 * s;
            vadd_CB_c00_2c97e0(&V, &V, &T);
            a = rfb_CB_c00_2c97e0(G192C_CB_c00_2c97e0 * E6C_CB_c00_2c97e0, G1930_CB_c00_2c97e0 * E6C_CB_c00_2c97e0);
            sph_CB_c00_2c97e0(&W, a, rang_CB_c00_2c97e0(), G1928_CB_c00_2c97e0 * 0.017453292f);
            W.z = rfb_CB_c00_2c97e0(3.5f, 5.5f) * E6C_CB_c00_2c97e0;
            debris_CB_c00_2c97e0(m, &V, &W);
        }
        sound_CB_c00_2c97e0(1, 0, m);
        if (d->i70 >= 0) {
            del_CB_c00_2c97e0(m);
            return;
        }
        m->unk94 = 0;
        m->flags = (m->flags & 0xEFFF) | 0x41;
        d->i74 = 0;
        m->state = 6;
        m->unkBC = 0;
        return;
    }
    if (t16_CB_c00_2c97e0(&d->h64) != 0) {
        sound_CB_c00_2c97e0(rnd_CB_c00_2c97e0(100) >= 75, 0, m);
        d->h64 = rr_CB_c00_2c97e0(st_CB_c00_2c97e0(0x78), st_CB_c00_2c97e0(0x12C));
    }
    if (d->i70 >= 0) {
        speed = (E6C_CB_c00_2c97e0 + E6C_CB_c00_2c97e0) * s;
    } else {
        speed = 0.333f;
    }
    switch (m->state) {
    case 0:
        break;
    case 1:
        if (m->unk70 & 2) {
            anim_CB_c00_2c97e0(m, rnd_CB_c00_2c97e0(2) == 0, 0, st_CB_c00_2c97e0(5));
        }
        if (t16_CB_c00_2c97e0(&d->h66) != 0 && dist_CB_c00_2c97e0(&m->pos, D_0013F450_CB_c00_2c97e0 + 0x80) < s * 5.0f) {
            v.x = cos_CB_c00_2c97e0(m->rot.z) * E6C_CB_c00_2c97e0 * s;
            v.y = sin_CB_c00_2c97e0(m->rot.z) * E6C_CB_c00_2c97e0 * s;
            v.z = 0.0f;
            vadd_CB_c00_2c97e0(&v, &v, &T);
            hit = f2594C8_CB_c00_2c97e0(m, &T, &v, 0, s * 0.2f, s * 0.5f, speed, 0.2617994f);
            if (hit != 0 || (angdiff_CB_c00_2c97e0(m->rot.z, atan2_CB_c00_2c97e0(T.x - m->pos.x, T.y - m->pos.y)) < 1.5707964f && dist3_CB_c00_2c97e0(&m->pos, &T) > s * 0.01f)) {
                m->state = 3;
                anim_CB_c00_2c97e0(m, 3, 0, st_CB_c00_2c97e0(10));
            } else {
                d->h66 = rr_CB_c00_2c97e0(st_CB_c00_2c97e0(0x3C), st_CB_c00_2c97e0(0xB4));
            }
        } else if (dist_CB_c00_2c97e0(&m->pos, D_0013F450_CB_c00_2c97e0 + 0x80) < s * 5.0f && 0.5235988f < angdiff_CB_c00_2c97e0(ang, m->rot.z)) {
            f2594C8_CB_c00_2c97e0(m, &T, &T, 0, s * 0.2f, s * 0.5f, speed, 0.2617994f);
            m->state = 2;
            anim_CB_c00_2c97e0(m, 3, 0, st_CB_c00_2c97e0(10));
        } else {
            f2594C8_CB_c00_2c97e0(m, &T, &T, 0, s * 0.2f, s * 0.5f, speed, 0.2617994f);
        }
        break;
    case 2:
        f2592B0_CB_c00_2c97e0(m, &d->f68, ang, E70_CB_c00_2c97e0 * 0.7853982f, E70_CB_c00_2c97e0 * 1.5707964f, E6C_CB_c00_2c97e0 * 0.7853982f);
        f2594C8_CB_c00_2c97e0(m, &T, &T, 0, s * 0.2f, s * 0.5f, speed, 0.2617994f);
        if (t16_CB_c00_2c97e0(&d->h66) != 0 && dist_CB_c00_2c97e0(&m->pos, D_0013F450_CB_c00_2c97e0 + 0x80) < s * 5.0f) {
            v.x = cos_CB_c00_2c97e0(m->rot.z) * E6C_CB_c00_2c97e0 * s;
            v.y = sin_CB_c00_2c97e0(m->rot.z) * E6C_CB_c00_2c97e0 * s;
            v.z = 0.0f;
            vadd_CB_c00_2c97e0(&v, &v, &T);
            if (fabs_CB_c00_2c97e0(ground_CB_c00_2c97e0(&v, 0, 0.5f) - v.z) <= (E6C_CB_c00_2c97e0 + E6C_CB_c00_2c97e0) * s) {
                m->state = 3;
                anim_CB_c00_2c97e0(m, 3, 0, st_CB_c00_2c97e0(10));
            }
        } else if (angdiff_CB_c00_2c97e0(ang, m->rot.z) < 0.08726646f || dist_CB_c00_2c97e0(&m->pos, D_0013F450_CB_c00_2c97e0 + 0x80) > s * 6.0f) {
            m->state = 1;
            d->h66 = rr_CB_c00_2c97e0(st_CB_c00_2c97e0(0x3C), st_CB_c00_2c97e0(0xB4));
            anim_CB_c00_2c97e0(m, rnd_CB_c00_2c97e0(2) == 0, 0, st_CB_c00_2c97e0(5));
        }
        break;
    case 3:
        if (dist_CB_c00_2c97e0(&m->pos, D_0013F450_CB_c00_2c97e0 + 0x80) > s * 6.0f) {
            m->state = 1;
            d->h66 = rr_CB_c00_2c97e0(st_CB_c00_2c97e0(0x3C), st_CB_c00_2c97e0(0xB4));
            anim_CB_c00_2c97e0(m, rnd_CB_c00_2c97e0(2) == 0, 0, st_CB_c00_2c97e0(5));
            f2594C8_CB_c00_2c97e0(m, &T, &T, 0, s * 0.2f, s * 0.5f, speed, 0.2617994f);
        } else if (dist_CB_c00_2c97e0(&m->pos, D_0013F450_CB_c00_2c97e0 + 0x80) < s * 3.0f) {
            f2592B0_CB_c00_2c97e0(m, &d->f68, ang, E70_CB_c00_2c97e0 * 0.7853982f, E70_CB_c00_2c97e0 * 1.5707964f, E6C_CB_c00_2c97e0 * 0.7853982f);
            v.x = cos_CB_c00_2c97e0(m->rot.z) * (E6C_CB_c00_2c97e0 + E6C_CB_c00_2c97e0);
            v.y = sin_CB_c00_2c97e0(m->rot.z) * (E6C_CB_c00_2c97e0 + E6C_CB_c00_2c97e0);
            v.z = 0.0f;
            vadd_CB_c00_2c97e0(&v, &v, &T);
            hit = f2594C8_CB_c00_2c97e0(m, &T, &v, 0, s * 0.2f, s * 0.5f, speed, 0.2617994f);
            if (hit == 0 && (1.5707964f < angdiff_CB_c00_2c97e0(m->rot.z, atan2_CB_c00_2c97e0(T.x - m->pos.x, T.y - m->pos.y)) || dist3_CB_c00_2c97e0(&m->pos, &T) < s * 0.01f)) {
                m->state = 1;
                d->h66 = rr_CB_c00_2c97e0(st_CB_c00_2c97e0(0x3C), st_CB_c00_2c97e0(0xB4));
                anim_CB_c00_2c97e0(m, rnd_CB_c00_2c97e0(2) == 0, 0, st_CB_c00_2c97e0(5));
            } else {
                m->state = 4;
                anim_CB_c00_2c97e0(m, 2, 0, st_CB_c00_2c97e0(10));
            }
        } else {
            f2592B0_CB_c00_2c97e0(m, &d->f68, ang, E70_CB_c00_2c97e0 * 0.7853982f, E70_CB_c00_2c97e0 * 1.5707964f, E6C_CB_c00_2c97e0 * 0.7853982f);
            v.x = cos_CB_c00_2c97e0(m->rot.z) * E6C_CB_c00_2c97e0 * s;
            v.y = sin_CB_c00_2c97e0(m->rot.z) * E6C_CB_c00_2c97e0 * s;
            v.z = 0.0f;
            vadd_CB_c00_2c97e0(&v, &v, &T);
            hit = f2594C8_CB_c00_2c97e0(m, &T, &v, 0, s * 0.2f, s * 0.5f, speed, 0.2617994f);
            if (hit == 0 && (1.5707964f < angdiff_CB_c00_2c97e0(m->rot.z, atan2_CB_c00_2c97e0(T.x - m->pos.x, T.y - m->pos.y)) || dist3_CB_c00_2c97e0(&m->pos, &T) < s * 0.01f)) {
                m->state = 1;
                d->h66 = rr_CB_c00_2c97e0(st_CB_c00_2c97e0(0x3C), st_CB_c00_2c97e0(0xB4));
                anim_CB_c00_2c97e0(m, rnd_CB_c00_2c97e0(2) == 0, 0, st_CB_c00_2c97e0(5));
            }
        }
        break;
    case 4:
        if (t16_CB_c00_2c97e0(&d->h66) != 0 && dist_CB_c00_2c97e0(&m->pos, D_0013F450_CB_c00_2c97e0 + 0x80) > s * 4.0f) {
            f2592B0_CB_c00_2c97e0(m, &d->f68, ang, E70_CB_c00_2c97e0 * 0.7853982f, E70_CB_c00_2c97e0 * 1.5707964f, E6C_CB_c00_2c97e0 * 0.7853982f);
            v.x = cos_CB_c00_2c97e0(m->rot.z) * E6C_CB_c00_2c97e0 * s;
            v.y = sin_CB_c00_2c97e0(m->rot.z) * E6C_CB_c00_2c97e0 * s;
            v.z = 0.0f;
            vadd_CB_c00_2c97e0(&v, &v, &T);
            hit = f2594C8_CB_c00_2c97e0(m, &T, &v, 0, s * 0.2f, s * 0.5f, speed, 0.2617994f);
            if (hit == 0 && (1.5707964f < angdiff_CB_c00_2c97e0(m->rot.z, atan2_CB_c00_2c97e0(T.x - m->pos.x, T.y - m->pos.y)) || dist3_CB_c00_2c97e0(&m->pos, &T) < s * 0.01f)) {
                m->state = 1;
                d->h66 = rr_CB_c00_2c97e0(st_CB_c00_2c97e0(0x3C), st_CB_c00_2c97e0(0xB4));
                anim_CB_c00_2c97e0(m, rnd_CB_c00_2c97e0(2) == 0, 0, st_CB_c00_2c97e0(5));
            } else {
                m->state = 3;
                anim_CB_c00_2c97e0(m, 3, 0, st_CB_c00_2c97e0(10));
            }
        } else {
            f2592B0_CB_c00_2c97e0(m, &d->f68, ang, E70_CB_c00_2c97e0 * 0.7853982f, E70_CB_c00_2c97e0 * 1.5707964f, E6C_CB_c00_2c97e0 * 0.7853982f);
            v.x = cos_CB_c00_2c97e0(m->rot.z) * (E6C_CB_c00_2c97e0 + E6C_CB_c00_2c97e0) * s;
            v.y = sin_CB_c00_2c97e0(m->rot.z) * (E6C_CB_c00_2c97e0 + E6C_CB_c00_2c97e0) * s;
            v.z = 0.0f;
            vadd_CB_c00_2c97e0(&v, &v, &T);
            hit = f2594C8_CB_c00_2c97e0(m, &T, &v, 0, s * 0.2f, s * 0.5f, speed, 0.2617994f);
            if (hit == 0 && (1.5707964f < angdiff_CB_c00_2c97e0(m->rot.z, atan2_CB_c00_2c97e0(T.x - m->pos.x, T.y - m->pos.y)) || dist3_CB_c00_2c97e0(&m->pos, &T) < s * 0.01f)) {
                m->state = 1;
                d->h66 = rr_CB_c00_2c97e0(st_CB_c00_2c97e0(0x3C), st_CB_c00_2c97e0(0xB4));
                anim_CB_c00_2c97e0(m, rnd_CB_c00_2c97e0(2) == 0, 0, st_CB_c00_2c97e0(5));
            }
        }
        break;
    case 5:
        if (f2DDEA0_CB_c00_2c97e0(m, d->p10) != 0) {
            m->state = 0;
            *(short *)(d->p14 + 0x68) = 0;
        }
        if (rnd_CB_c00_2c97e0(4) != 0) {
            return;
        }
        {
            Vec4A_c00_2c97e0 *pv;
            Vec4A_c00_2c97e0 *pw;
            pv = &V;
            r = rfb_CB_c00_2c97e0(G1938_CB_c00_2c97e0, G193C_CB_c00_2c97e0) * s;
            a = rang_CB_c00_2c97e0();
            sph_CB_c00_2c97e0(pv, r, a, rang_CB_c00_2c97e0());
            V.z += G1934_CB_c00_2c97e0 * s;
            vadd_CB_c00_2c97e0(pv, pv, &T);
            pw = &W;
            a = rfb_CB_c00_2c97e0(G192C_CB_c00_2c97e0 * E6C_CB_c00_2c97e0, G1930_CB_c00_2c97e0 * E6C_CB_c00_2c97e0);
            sph_CB_c00_2c97e0(pw, a, rang_CB_c00_2c97e0(), G1928_CB_c00_2c97e0 * 0.017453292f);
            W.z = rfb_CB_c00_2c97e0(3.5f, 5.5f) * E6C_CB_c00_2c97e0;
            debris_CB_c00_2c97e0(m, pv, pw);
        }
        return;
    }
    if (hit == 0) {
        float len;
        float max;
        vsub_CB_c00_2c97e0(&W, &T, &m->pos);
        len = vlen_CB_c00_2c97e0(&W);
        max = E6C_CB_c00_2c97e0 * s;
        if (max < len) {
            vscale_CB_c00_2c97e0(&W, &W, max);
        }
        vadd_CB_c00_2c97e0(&T, &m->pos, &W);
        if (fabs_CB_c00_2c97e0(f25A748_CB_c00_2c97e0(&T) - m->pos.z) <= E6C_CB_c00_2c97e0 * s) {
            qcopy_c00_2c97e0(&m->pos, &T);
        }
    } else {
        qcopy_c00_2c97e0(&m->pos, &T);
    }
    d->f6C += E70_CB_c00_2c97e0 * 9.8f;
    m->pos.z -= d->f6C;
    {
        float g = ground_CB_c00_2c97e0(&m->pos, 0, 0.5f);
        if (m->pos.z < g && g < m->pos.z + s * 0.25f) {
            d->f6C = 0.0f;
            m->pos.z = g;
        }
    }
    if (f1F3958_CB_c00_2c97e0() == 0) {
        d->f78 = 0.0f;
    }
    if (m->state != 5) {
        float pitch;
        float roll;
        m->pos.z = D_L00_00173F40_CB_c00_2c97e0[10];
        qcopy_c00_2c97e0(&W, &D_L00_00173F40_CB_c00_2c97e0[16]);
        V.x = W.x * cos_CB_c00_2c97e0(m->rot.z) + W.y * sin_CB_c00_2c97e0(m->rot.z);
        V.y = W.y * cos_CB_c00_2c97e0(m->rot.z) - W.x * sin_CB_c00_2c97e0(m->rot.z);
        V.z = W.z;
        pitch = -atan2_CB_c00_2c97e0(sqrt_CB_c00_2c97e0(V.x * V.x + V.z * V.z), V.y);
        roll = atan2_CB_c00_2c97e0(V.z, V.x);
        m->rot.x = angadd_CB_c00_2c97e0(m->rot.x, angsub_CB_c00_2c97e0(pitch, m->rot.x) * (E6C_CB_c00_2c97e0 * 6.2831855f));
        m->rot.y = angadd_CB_c00_2c97e0(m->rot.y, angsub_CB_c00_2c97e0(roll, m->rot.y) * (E6C_CB_c00_2c97e0 * 6.2831855f));
    }
    if (m->pos.z < 0.0f) {
        del_CB_c00_2c97e0(m);
    } else {
        m->unk58 = save58;
    }
}
struct Manip_c00_2c97e0;
struct GifEntry_c00_2c97e0;
struct AnimSeq_c00_2c97e0;
struct Moby_c00_2c97e0;
