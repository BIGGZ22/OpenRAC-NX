/* NON_MATCHING func_L02_002FCE00 -- src/overlays/shared/vendor_002A5218.c
 * Best so far: SIZE ours 2236 / retail 2244, checked 2026-10-19.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * Lead note 2026-10-19: the 8 missing bytes are the first block's base/id copies (lw a0 + daddu a1,a0; daddu a3).
 *   Only a second extern alias of D_L02_00160058 (build-sn opus/p9) keeps cse1 from merging the first block into
 *   the loop, which cannot land; a base local in the loop merges it (2208). Natural form still unknown.
 * What the last attempts found:
 *   p2 decl reorder no effect; p3 drop t: worse; p4 tm: no change; p5 separate 'hit' var for post-speed s18 flag: 
 *   p6 decl swap no change; p7 (v/id copies+base var) restructured loop worse (2220); p8 base var: cse collapses c
 *   p10 loop uses array index: same; p11 alias+v: worse; p12 for(;;)break: size 2208; p13 v/id copies no alias: cr
 *   p16 id int: loop merged 2208; p17-19 ++count: fixes sext but s18/s19 reg flip; p20/21 t=(unsigned short)count+
 *   p23-p31: count block variants. `s->count = s->count + 1; if (s->count >= s19)` reproduces retail's count block
 *   Best: p9.c = BYTES 70/2244 (needs a second extern alias of D_L02_00160058 for the first block, which stops cse
 *   Fail block placement fixed in p1+ (gotos put fail between zoom 3.36 and zoom 8.0 paths).
 *   Round 2 (from p5, no alias): p32 e=base;e+=off, p33 base+idx both blocks, p41 base local inside if/do-while: a
 */
typedef struct {
    float speed;
    char pad4[0x1C];
    short timer;
    char pad22[6];
    int target_id;
    float range_b;
    float range_a;
    float shake;
    float shake2;
    short mode;
    short count;
    float heading;
    int list;
    void *target;
    float rumble;
    int done;
} Cam_2FCE00;
typedef struct { char pad0[0x18]; float heading; Cam_2FCE00 *cam; } Spot_2FCE00;
typedef struct { char pad0[0x10]; float pos[4]; signed char state; char pad21[0xDF]; } Ent_2FCE00;
typedef struct { char pad0[0x84]; short spot; } Moby_2FCE00;
typedef struct { float x, y; } Pos_2FCE00;
typedef struct { char pad0[0x100]; float lx; float ly; } Pad_2FCE00;

extern Spot_2FCE00 *D_L02_0015F050_2FCE00 __asm__("D_L02_0015F050") MACRO_ADDR;
extern unsigned short *D_L02_001AC140_2FCE00[] __asm__("D_L02_001AC140");
extern Ent_2FCE00 *D_L02_00160058_2FCE00 __asm__("D_L02_00160058") MACRO_ADDR;
extern Pos_2FCE00 *D_L02_00167480_2FCE00 __asm__("D_L02_00167480");
extern char D_0013A5E0_2FCE00[] __asm__("D_0013A5E0");
extern char D_0013E633_2FCE00[] __asm__("D_0013E633");
extern void release_2FCE00(Moby_2FCE00 *) __asm__("func_L00_002E9838");
extern int active_2FCE00(Moby_2FCE00 *) __asm__("func_L00_002E9870");
extern int ready_2FCE00(Moby_2FCE00 *) __asm__("func_L02_002FCA80");
extern float angle_2FCE00(float, float) __asm__("func_L00_001FF860");
extern float fabs_2FCE00(float) __asm__("func_001F9B88");
extern float angle_delta_2FCE00(float, float) __asm__("func_001FA850");
extern float angle_add_2FCE00(float, float) __asm__("func_001FA748");
extern int ticks_2FCE00(int) __asm__("func_001F9850");
extern float fixed_2FCE00(int) __asm__("func_001FA888");
extern float sine_2FCE00(float) __asm__("func_001F9F90");
extern float cosine_2FCE00(float) __asm__("func_001F9FA8");
extern void push_dir_2FCE00(float *, float, float) __asm__("func_L00_002E9DC8");
extern void push_to_2FCE00(float *, float, float) __asm__("func_L00_002E9E20");
extern void zoom_2FCE00(int, float, float) __asm__("func_L00_002E9900");
extern void zoom_end_2FCE00(void) __asm__("func_L00_002E9AD0");
extern void tilt_2FCE00(float, float) __asm__("func_L00_002E9968");
extern void roll_2FCE00(int, float, float) __asm__("func_L00_002E99A0");
extern void blend_2FCE00(int) __asm__("func_L00_002E9A18");
extern void blend_end_2FCE00(void) __asm__("func_L00_002E9AF8");
extern void fov_2FCE00(float, float) __asm__("func_L00_002E9A40");
extern void fov2_2FCE00(float, float) __asm__("func_L02_002F78C8");
extern void subtract_2FCE00(float *, float *, void *) __asm__("func_001F9BF0");
extern void sound_2FCE00(int) __asm__("func_L02_002F79A8");
extern void rumble_2FCE00(float, float, float) __asm__("func_L02_002F79D0");
extern int can_see_2FCE00(Moby_2FCE00 *, float, float) __asm__("func_L02_002FCBC0");

void func_L02_002FCE00_r(Moby_2FCE00 *m) __asm__("func_L02_002FCE00");

/* Cutscene camera update for a spot: finds its target entity, then turns, zooms and shakes toward it. */
void func_L02_002FCE00_r(Moby_2FCE00 *m) {
    Cam_2FCE00 *s;
    Ent_2FCE00 *ent;
    float v[3];
    int s16, s18, s19, s21, flag, t, arg, hit;
    float k, speed;

    s = D_L02_0015F050_2FCE00[m->spot].cam;
    if (s->done != 0) {
        s->timer = 0;
        release_2FCE00(m);
        return;
    }
    if (s->list >= 0) {
        unsigned short *p = D_L02_001AC140_2FCE00[s->list];
        Ent_2FCE00 *e;
        unsigned short id;
        unsigned int v;
        if (p == 0) goto fail;
        id = *p;
        e = (Ent_2FCE00 *)((char *)D_L02_00160058_2FCE00 + ((*p & 0x7FFF) << 8));
        while (e->state < 0) {
            if ((short)id < 0) goto fail;
            p++;
            v = *p;
            e = (Ent_2FCE00 *)((char *)D_L02_00160058_2FCE00 + ((v & 0x7FFF) << 8));
            id = v;
        }
        ent = e;
    } else {
        ent = &D_L02_00160058_2FCE00[s->target_id];
        if (ent->state < 0) goto fail;
    }
    s->target = ent;
    if (s->mode == 6 && *(Ent_2FCE00 **)(D_0013E633_2FCE00 + 0x1119) != ent) {
        s->timer = 0;
        release_2FCE00(m);
        return;
    }
    if (active_2FCE00(m) <= 0) {
        s->timer = 0;
        return;
    }
    if (ready_2FCE00(m) == 0) {
        s->timer = 0;
        release_2FCE00(m);
        return;
    }
    if (s->mode == 5) {
        float h;
        float a = angle_2FCE00(D_L02_00167480_2FCE00->x, D_L02_00167480_2FCE00->y);
        if (s->timer < 2) {
            s->heading = D_L02_0015F050_2FCE00[m->spot].heading;
        } else if (0.3f <= fabs_2FCE00(((Pad_2FCE00 *)(D_0013A5E0_2FCE00 + 0x2460))->lx)) {
            s->heading = a;
        } else if (0.3f <= fabs_2FCE00(((Pad_2FCE00 *)(D_0013A5E0_2FCE00 + 0x2460))->ly)) {
            s->heading = a;
        }
        h = s->heading;
        if (1.5707964f < angle_delta_2FCE00(h, a)) {
            angle_add_2FCE00(h, 3.1415927f);
        }
    }
    if (s->mode == 4) {
        float h = D_L02_0015F050_2FCE00[m->spot].heading;
        if (1.5707964f < angle_delta_2FCE00(h, angle_2FCE00(D_L02_00167480_2FCE00->x, D_L02_00167480_2FCE00->y))) {
            h = angle_add_2FCE00(h, 3.1415927f);
        }
        s->heading = h;
    }
    s->timer++;
    flag = 1;
    k = 1.0f;
    if (s->mode == 1) {
        s16 = ticks_2FCE00(300);
        t = ticks_2FCE00(400);
        arg = 560;
    } else {
        s16 = ticks_2FCE00(300);
        t = ticks_2FCE00(350);
        arg = 400;
    }
    s19 = t;
    s21 = ticks_2FCE00(arg);
    s18 = ticks_2FCE00(200);
    if (s->mode == 7) {
        if (*(float *)(D_0013A5E0_2FCE00 + 0x2560) != 0.0f) s->timer = s16;
        s19 = s->timer;
        if (s19 >= s16) {
            flag = 0;
            s->count++;
            if (s16 + ticks_2FCE00(30) < s19) {
                k = 0.1f;
                flag = 1;
            }
        }
        if (s->timer >= s21) {
            flag = 1;
            s->count = 0;
            s->timer = 1;
        }
        if (s18 < s->timer && s->timer < s16) {
            s->count = s16;
            s->timer = s18;
        }
    } else if ((unsigned short)(s->mode - 1) < 2) {
        sound_2FCE00(0xB0);
        rumble_2FCE00(1.0f, 12.0f, 0.11f);
        if (((Pad_2FCE00 *)(D_0013A5E0_2FCE00 + 0x2460))->lx != 0.0f) {
            s->timer = s16;
        } else if (((Pad_2FCE00 *)(D_0013A5E0_2FCE00 + 0x2460))->ly != 0.0f) {
            s->timer = s16;
        }
        if (s->timer >= s16) {
            t = (short)(s->count + 1);
            s->count = t;
            if (t >= s19) {
                if (can_see_2FCE00(m, 30.0f, 0.0f) != 0) {
                    s->timer = s21;
                } else {
                    flag = 0;
                }
            } else {
                flag = 0;
            }
        }
        if (s->timer >= s21) {
            flag = 1;
            s->count = 0;
            s->timer = 1;
        }
        if (s18 < s->timer && s->timer < s16) {
            s->count = s16;
            s->timer = s18;
        }
    } else {
        if (s18 < s->timer) s->timer = s18;
    }
    {
        float f = fixed_2FCE00(s18);
        hit = 0;
        speed = s->speed * k * 0.017453292f * ((float)s->timer / f);
    }
    if (flag) {
        if (can_see_2FCE00(m, s->range_a, s->range_b) != 0) {
            hit = 1;
            if ((unsigned short)(s->mode - 4) < 2) {
                v[0] = sine_2FCE00(s->heading);
                v[1] = cosine_2FCE00(s->heading);
                v[2] = 0.0f;
                push_dir_2FCE00(v, speed, v[2]);
            } else if (s->mode == 3) {
                float f12;
                if (s->timer < ticks_2FCE00(90)) {
                    f12 = (speed - 0.0f) * ((float)s->timer / fixed_2FCE00(ticks_2FCE00(90))) + 0.0f;
                } else {
                    f12 = speed;
                }
                push_to_2FCE00(ent->pos, f12, 0.0f);
            } else {
                push_to_2FCE00(ent->pos, speed, 0.0f);
            }
        }
    }
    if (s->shake != 0.0f) {
        zoom_2FCE00(0, s->shake, 0.003f);
        zoom_end_2FCE00();
    }
    if (s->shake2 != 0.0f) {
        tilt_2FCE00(s->shake2, 0.003f);
    }
    if (s->rumble != 0.0f) {
        roll_2FCE00(0, s->rumble, 0.005f);
    }
    if (s->mode == 3) {
        float f12;
        float base = 0.01f;
        blend_2FCE00(0);
        zoom_end_2FCE00();
        blend_end_2FCE00();
        fov_2FCE00(base, 0.2f);
        if (s->timer >= ticks_2FCE00(120)) {
            f12 = 0.03f;
        } else {
            f12 = (float)s->timer / fixed_2FCE00(ticks_2FCE00(120)) * 0.02f + base;
        }
        fov2_2FCE00(f12, 0.2f);
    }
    if ((unsigned short)(s->mode - 4) >= 2) return;
    {
    float z = 0.003f;
    blend_2FCE00(0);
    zoom_end_2FCE00();
    blend_end_2FCE00();
    fov_2FCE00(0.02f, 0.2f);
    zoom_2FCE00(0, 5.84f, z);
    if (s->mode == 4) {
        float f;
        subtract_2FCE00(v, ent->pos, D_0013E633_2FCE00 + 0xE9D);
        f = angle_2FCE00(v[0], v[1]);
        if (hit != 0 && 1.9198622f < angle_delta_2FCE00(f, s->heading)) {
            if (s->shake == 0.0f) goto eight;
            zoom_2FCE00(0, s->shake + 3.36f, z);
            goto end;
fail:
            s->timer = -1;
            release_2FCE00(m);
            return;
eight:
            zoom_2FCE00(0, 8.0f, z);
        }
    }
    }
end:
    sound_2FCE00(0xB0);
}
