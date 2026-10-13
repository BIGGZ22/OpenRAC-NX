/* functional: medium. Batalia rocket-like moby: state 1 flies along its velocity (sweep test, may hit the hero object -> state 99), state 99 bursts 50 particles (4 kinds by index mod 4) and removes the moby; derived from the NTSC decomp draft and checked against the PAL assembly. Not runnable in capture. */
typedef struct Rec_DDD20 {
    float pos[4];
    float vel[4];
    void *owner;
    int flags;
    int unknown;
    float scale;
    int state;
} Rec_DDD20;
extern int coll_DDD20(float, void *, int, void *, void *) __asm__("func_L00_001F2BE8");
extern int trace_DDD20(void *, void *, int, void *, int) __asm__("func_L00_001EFFF0");
extern void add_DDD20(void *, void *, void *) __asm__("func_001F9BD8");
extern void add2_DDD20(void *, void *, void *) __asm__("func_L00_001FF610");
extern void scale_DDD20(void *, void *, float) __asm__("func_L00_001FF4B0");
extern int test_DDD20(void *) __asm__("func_001F9908");
extern void setup_DDD20(void *) __asm__("func_001F9BC0");
extern int alloc_DDD20(int, int, void *) __asm__("func_0022ED80");
extern float rnd_DDD20(float, float) __asm__("func_002140F8");
extern float rndang_DDD20(void) __asm__("func_00214158");
extern float fa_DDD20(float) __asm__("func_001F9F90");
extern float fb_DDD20(float) __asm__("func_001F9FA8");
extern int rndbelow_DDD20(int) __asm__("func_002140B0");
extern int rndcol_DDD20(int) __asm__("func_001F9850");
extern int blend_DDD20(int, int) __asm__("func_L00_00258BC8");
extern int col_DDD20(float, int, int) __asm__("func_L00_00237B70");
extern void emit_DDD20(float, void *, void *, int, int, int, int) __asm__("func_L00_0026CD70");
extern void special_DDD20(void *, float, float, float) __asm__("func_00215C00");
extern void finish_DDD20(void *) __asm__("func_0020D678");
extern float D_0015EE6C_DDD20 __asm__("D_0015EE6C");
extern float D_0015EE70_DDD20 __asm__("D_0015EE70");
extern char D_L08_001746C0_DDD20[] __asm__("D_L08_001746C0");
extern int D_L08_001746D8_DDD20 __asm__("D_L08_001746D8");

void func_L08_002DDD20(char *m) {
    char *data = *(char **)(m + 0x78);
    int original[4];
    Rec_DDD20 rec;
    int velocity[4];
    float effect[4];
    float *ef = effect;
    int *pos = (int *)(m + 0x10);
    int state;
    int i, axis;
    float radius, angle, ft;

    if (coll_DDD20(0.5f, m + 0x10, 0, m, 0) != 0 && D_L08_001746D8_DDD20 != 0 &&
        (D_L08_001746D8_DDD20 != *(int *)(data + 0x20) || *(int *)(data + 0x14) != 0)) {
        *(unsigned char *)(m + 0x20) = 0x63;
    }
    state = *(unsigned char *)(m + 0x20);
    if (state == 1) {
        int *vs = (int *)data;
        original[0] = pos[0]; original[1] = pos[1]; original[2] = pos[2]; original[3] = pos[3];
        add_DDD20(pos, pos, data);
        *(float *)(data + 8) = *(float *)(data + 8) - D_0015EE70_DDD20 * 10.8f;
        ((int *)rec.pos)[0] = pos[0]; ((int *)rec.pos)[1] = pos[1];
        ((int *)rec.pos)[2] = pos[2]; ((int *)rec.pos)[3] = pos[3];
        rec.state = state;
        rec.flags = 0x810000;
        rec.scale = 1.0f;
        rec.owner = m;
        ((int *)rec.vel)[0] = vs[0]; ((int *)rec.vel)[1] = vs[1];
        ((int *)rec.vel)[2] = vs[2]; ((int *)rec.vel)[3] = vs[3];
        if (trace_DDD20(original, &rec, 0, m, 0) != 0) {
            int w = *(int *)(data + 0x20);
            if (w != 0 && *(int *)(D_L08_001746C0_DDD20 + 0x18) != w) {
                int *s = (int *)(D_L08_001746C0_DDD20 + 0x20);
                *(unsigned char *)(m + 0x20) = 0x63;
                pos[0] = s[0]; pos[1] = s[1]; pos[2] = s[2]; pos[3] = s[3];
                add2_DDD20(data, data, D_L08_001746C0_DDD20 + 0x40);
                scale_DDD20(data, data, D_0015EE6C_DDD20 + D_0015EE6C_DDD20);
            }
        } else if (test_DDD20(data + 0x10) != 0) {
            *(unsigned char *)(m + 0x20) = 0x63;
        }
    } else if (state == 0x63) {
        ft = D_0015EE6C_DDD20;
        rec.pos[0] = ft * 14.0f;
        rec.pos[1] = ft * 22.0f;
        rec.pos[2] = ft * 12.0f;
        rec.pos[3] = 0.0f;
        ((int *)original)[0] = ((int *)rec.pos)[0]; ((int *)original)[1] = ((int *)rec.pos)[1];
        ((int *)original)[2] = ((int *)rec.pos)[2]; ((int *)original)[3] = ((int *)rec.pos)[3];
        rec.flags = 0x810001;
        rec.owner = m;
        rec.scale = 1.0f;
        setup_DDD20(rec.vel);
        rec.state = 0;
        coll_DDD20(1.0f, m + 0x10, 9, m, rec.vel);
        alloc_DDD20(0, 0, m);
        for (i = 0; i < 50; i++) {
            float *orig = (float *)original;
            axis = i % 4;
            ft = D_0015EE6C_DDD20;
            velocity[0] = velocity[1] = velocity[2] = velocity[3] = 0;
            ((float *)velocity)[0] = rnd_DDD20(ft * -0.5f, ft * 0.5f);
            ft = D_0015EE6C_DDD20;
            ((float *)velocity)[1] = rnd_DDD20(ft * -0.5f, ft * 0.5f);
            ft = D_0015EE6C_DDD20;
            ((float *)velocity)[2] = rnd_DDD20(ft * 4.0f, orig[axis]);
            ((int *)effect)[0] = velocity[0]; ((int *)effect)[1] = velocity[1];
            ((int *)effect)[2] = velocity[2]; ((int *)effect)[3] = velocity[3];
            radius = rnd_DDD20(0.0f, 0.25f);
            angle = rndang_DDD20();
            rec.pos[0] = fa_DDD20(angle) * radius;
            rec.pos[1] = fb_DDD20(angle) * radius;
            rec.pos[2] = 0.0f;
            add_DDD20(rec.pos, rec.pos, m + 0x10);
            {
                float fr = radius * D_0015EE6C_DDD20;
                float r2 = radius * 0.717f;
                float z = rec.pos[2] - r2;
                float ez = ef[2] - fr * 8.0f;
                rec.pos[2] = z;
                ef[2] = ez;
            }
            if (axis == 1) {
                continue;
            } else if (axis == 0) {
                float size = rnd_DDD20(200000.0f, 300000.0f);
                int c1 = rndcol_DDD20(60);
                int c2 = rndcol_DDD20(120);
                emit_DDD20(size, rec.pos, effect, 0x1F101820, 0x101010, blend_DDD20(c1, c2), 0);
            } else if (axis == 2) {
                int color = (rndbelow_DDD20(100) > 39) ? 0x2F486078 : 0x5FF8F8F8;
                int c2 = rndcol_DDD20(30);
                int c3 = rndcol_DDD20(45);
                emit_DDD20(150000.0f, rec.pos, effect, color, 0x0F000020, blend_DDD20(c2, c3), 2);
            } else if (axis == 3) {
                int colA = col_DDD20(rnd_DDD20(0.25f, 1.0f), 0x7F000000, 0x7F182030);
                int colB = col_DDD20(rnd_DDD20(0.5f, 1.0f), 0, 0x5F5F5F);
                float r = rnd_DDD20(0.0f, 1.0f) * D_0015EE6C_DDD20;
                float sz;
                int a, b;
                special_DDD20(effect, r, angle, rndang_DDD20());
                sz = rnd_DDD20(150000.0f, 200000.0f);
                a = rndcol_DDD20(60);
                b = rndcol_DDD20(120);
                emit_DDD20(sz, rec.pos, effect, colA, colB, blend_DDD20(a, b), 3);
            }
        }
        finish_DDD20(m);
    }
}
