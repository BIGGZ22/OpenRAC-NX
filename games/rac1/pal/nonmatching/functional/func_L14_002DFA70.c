/* functional: medium. Per-frame update of a flying debris moby: occasionally spawns the small effect (func_L14_002E0040), moves it by its velocity, damps x/y (and z while rising) by a frame-scaled factor, applies gravity, spins it, and if it leaves the 2..1021 box does nothing but remove it; otherwise on a hit (func_001F9908 or the sphere test) it spawns the impact effect and removes itself, and on no hit leaves two sparks. From the NTSC decomp's draft for the same function. */
extern float D_L14_THRESH_DFA70 __asm__("D_L14_00161BF0");
extern float D_L14_DAMP_DFA70 __asm__("D_L14_00161BEC");
extern float D_L14_GRAV_DFA70 __asm__("D_L14_00161BE8");
extern float D_0015EE60_DFA70 __asm__("D_0015EE60");
extern float D_0015EE64_DFA70 __asm__("D_0015EE64");
extern unsigned char D_L14_0015F660_DFA70[] __asm__("D_L14_0015F660");
extern int rnd_DFA70(int) __asm__("func_002140B0");
extern void spawn_DFA70(void (*)(void *), void *) __asm__("func_001F49B0");
extern void fx_DFA70(void *) __asm__("func_L14_002E0040");
extern void vadd_DFA70(void *, void *, void *) __asm__("func_001F9BD8");
extern float fabs_DFA70(float) __asm__("func_001F9B88");
extern float addrot_DFA70(float, float) __asm__("func_001FA748");
extern int hitq_DFA70(void *) __asm__("func_001F9908");
extern int sphere_DFA70(float, void *, int, void *) __asm__("func_L00_001F10E0");
extern void impact_DFA70(void *, void *, void *, float, float, int, int, int,
                         float, float, float, float, int, float, int, int, int, int) __asm__("func_L00_0025F4A8");
extern void remove_DFA70(void *) __asm__("func_0020D678");
extern int scale_DFA70(int) __asm__("func_001F9850");
extern int randi_DFA70(int, int) __asm__("func_L00_00258BC8");
extern float randf_DFA70(float, float) __asm__("func_002140F8");
extern void burst_DFA70(void *, void *, int, int, int, int, float) __asm__("func_L00_0026DA50");
extern unsigned char *spark_DFA70(void *, float, float, float, int, void *, float, int) __asm__("func_L00_0026DEA0");

void func_L14_002DFA70(unsigned char *moby) {
    float *velocity = *(float **)(moby + 0x78);
    float *position;
    int i, n, color, lo, hi;
    float size, a;
    unsigned char *particle, *fade;

    if (rnd_DFA70(2) != 0) {
        spawn_DFA70((void (*)(void *))fx_DFA70, moby);
    }
    position = (float *)(moby + 0x10);
    vadd_DFA70(position, position, velocity);
    a = fabs_DFA70(velocity[0]);
    if (D_L14_THRESH_DFA70 * D_0015EE60_DFA70 < a) {
        velocity[0] *= (D_L14_DAMP_DFA70 - 1.0f) * D_0015EE60_DFA70 + 1.0f;
    }
    a = fabs_DFA70(velocity[1]);
    if (D_L14_THRESH_DFA70 * D_0015EE60_DFA70 < a) {
        velocity[1] *= (D_L14_DAMP_DFA70 - 1.0f) * D_0015EE60_DFA70 + 1.0f;
    }
    if (velocity[2] > 0.0f) {
        velocity[2] *= (D_L14_DAMP_DFA70 - 1.0f) * D_0015EE60_DFA70 + 1.0f;
    }
    velocity[2] = velocity[2] - D_L14_GRAV_DFA70 * D_0015EE64_DFA70;
    *(float *)(moby + 0x40) = addrot_DFA70(*(float *)(moby + 0x40), velocity[4]);
    *(float *)(moby + 0x44) = addrot_DFA70(*(float *)(moby + 0x44), velocity[5]);
    *(float *)(moby + 0x48) = addrot_DFA70(*(float *)(moby + 0x48), velocity[6]);

    if (!(*(float *)(moby + 0x10) < 2.0f || 1021.0f < *(float *)(moby + 0x10) ||
          *(float *)(moby + 0x14) < 2.0f || 1021.0f < *(float *)(moby + 0x14) ||
          *(float *)(moby + 0x18) < 2.0f || 1021.0f < *(float *)(moby + 0x18))) {
        if (hitq_DFA70(velocity + 8) != 0) {
            goto effect;
        }
        if (sphere_DFA70(0.5f, position, 0, *(void **)(velocity + 9)) == 0) {
            goto particles;
        }
effect:
        impact_DFA70(moby, velocity, position, 0.0f, 0.0f, 5, 2, 4,
                     1.0f, 0.5f, 9.0f, 1.0f, 0, 15.0f, 1, 5, -1, 0);
        sphere_DFA70(1.0f, position, 0, *(void **)0x1414D0);
    }
    remove_DFA70(moby);
    return;
particles:
    lo = scale_DFA70(10);
    hi = scale_DFA70(20);
    burst_DFA70(position, D_L14_0015F660_DFA70, 0x4F007FFF, 0x1FFFFFFF,
                randi_DFA70(lo, hi), 1, 60000.0f);
    for (i = 1; i >= 0; --i) {
        n = rnd_DFA70(6);
        if (rnd_DFA70(2) != 0) n = -n;
        size = randf_DFA70(40000.0f, 100000.0f);
        color = randi_DFA70(0x30, 0xFF);
        particle = spark_DFA70(position, 0.025f, 1.0f, 1.0f, n, D_L14_0015F660_DFA70, size,
                               color | ((color << 16) | ((color << 8) | 0x60000000)));
        if (particle != 0) {
            particle[3] = 0x44;
            fade = particle + 0x20;
            *(short *)(particle + 0xA) = scale_DFA70(0x3C);
            *(int *)(fade + 4) = 2;
            fade[0xA] = 0x60;
            fade[0xB] = particle[0xA];
        }
    }
}
