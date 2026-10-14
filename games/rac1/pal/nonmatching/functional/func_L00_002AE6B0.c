/* functional: medium (statically checked against the assembly only). Thrown/landing prediction for a bouncing object: starts from the hero's position/velocity (or the object's, once it has a holder), steps a ballistic path 10 sub-steps per tick with gravity until it hits something (func_L00_001EFFF0) or the timer runs out, records the landing point/normal/hit object in the vars block v, and arms a callback (func_L00_002ADE90). Returns 1 when a landing was found, else 0. */
typedef struct { float f[4]; } __attribute__((aligned(16))) V_AE6B0;
typedef struct {
    V_AE6B0 vel;
    V_AE6B0 landing;
    V_AE6B0 normal;
    V_AE6B0 previous_hit;
    V_AE6B0 difference;
    char *holder;
    short timer;
    short holder_flag;
    char pad58[8];
    char *last_hit;
    int hit_count;
} Land_AE6B0;
extern float atan2_AE6B0(float, float) __asm__("func_L00_001FF860");
extern float addrot_AE6B0(float, float) __asm__("func_001FA748");
extern float cos_AE6B0(float) __asm__("func_001F9F90");
extern float sin_AE6B0(float) __asm__("func_001F9FA8");
extern void add_AE6B0(void *, void *, void *) __asm__("func_001F9BD8");
extern void sub_AE6B0(void *, void *, void *) __asm__("func_001F9BF0");
extern void scale_AE6B0(float, void *, void *) __asm__("func_001F9C30");
extern int ticks_AE6B0(int) __asm__("func_001F9850");
extern int timer_AE6B0(int *) __asm__("func_001F9908");
extern int sweep_AE6B0(void *, void *, int, void *, int) __asm__("func_L00_001EFFF0");
extern int is_down_AE6B0(void) __asm__("func_L00_001F3958");
extern int check_AE6B0(void *) __asm__("func_L00_0025D390");
extern void arm_AE6B0(void *, void *) __asm__("func_001F49B0");
extern void nudge_AE6B0(void *, int, float) __asm__("func_00214358");
extern void cb_AE6B0(void) __asm__("func_L00_002ADE90");
extern int D_L00_0015F6A8_AE6B0 __asm__("D_L00_0015F6A8") MACRO_ADDR;
extern float D_0015EE70_AE6B0 __asm__("D_0015EE70") MACRO_ADDR;
extern char D_0013E633_AE6B0[] __asm__("D_0013E633") __attribute__((section(".data")));
extern char D_0013E15A_AE6B0[] __asm__("D_0013E15A") __attribute__((section(".data")));
extern int D_L00_00173F40_AE6B0[] __asm__("D_L00_00173F40") __attribute__((section(".data")));
extern V_AE6B0 D_L00_00173F60_AE6B0 __asm__("D_L00_00173F60") __attribute__((section(".data")));
extern V_AE6B0 D_L00_00173F80_AE6B0 __asm__("D_L00_00173F80") __attribute__((section(".data")));
extern V_AE6B0 D_L00_00166EC0_AE6B0 __asm__("D_L00_00166EC0") __attribute__((section(".data")));
int func_L00_002AE6B0_impl(Land_AE6B0 *, char *, int) __asm__("func_L00_002AE6B0");
int func_L00_002AE6B0_impl(Land_AE6B0 *v, char *m, int mode) {
    V_AE6B0 position, velocity, previous, hero_position, offset, start;
    int timer, found = 0, object_hit = 0, i;
    char *hero = D_0013E633_AE6B0 + 0xE1D;
    char *hit;
    char *last;

    if (*(int *)(hero + 0x2084) == 0x72)
        return 0;
    if (D_L00_0015F6A8_AE6B0 != 0)
        return 0;
    previous = *(V_AE6B0 *)(m + 0x10);
    if (*(unsigned char *)(m + 0x20) == 0) {
        position.f[0] = cos_AE6B0(addrot_AE6B0(-0.36196801f, atan2_AE6B0(*(float *)(hero + 0x670), *(float *)(hero + 0x674)))) * 0.85903901f;
        position.f[1] = sin_AE6B0(addrot_AE6B0(-0.36196801f, atan2_AE6B0(*(float *)(hero + 0x670), *(float *)(hero + 0x674)))) * 0.85903901f;
        position.f[2] = 0.0f;
        add_AE6B0(&position, &position, hero + 0x80);
        position.f[2] += 0.49082f;
        velocity = v->vel;
        if (*(int *)(hero + 0x208C) == 0xF)
            add_AE6B0(&velocity, &velocity, hero + 0x100);
        if (*(int *)(hero + 0x2FC) != 0 && check_AE6B0(*(void **)(hero + 0x2FC)) && mode)
            add_AE6B0(&velocity, &velocity, hero + 0x100);
        timer = ticks_AE6B0(300);
    } else {
        position = *(V_AE6B0 *)(m + 0x10);
        velocity = v->vel;
        timer = v->timer;
        if (*(int *)(*(char **)(v->holder + 0x78) + 0x50))
            v->holder_flag = 1;
    }
    hero_position = *(V_AE6B0 *)(*(char **)(hero + 0x2080) + 0x10);
    hero_position.f[2] = position.f[2];
    if (sweep_AE6B0(&hero_position, &position, 0x10, *(void **)(hero + 0x2080), 0) &&
        (is_down_AE6B0() || 0.0f < velocity.f[2])) {
        hit = (char *)D_L00_00173F40_AE6B0[6];
        if (hit != 0 && D_L00_00173F40_AE6B0[7] <= 0) {
            if ((hit != *(char **)(hero + 0x2080) && hit != v->holder) ||
                timer < ticks_AE6B0(300) - ticks_AE6B0(10)) {
                offset.f[0] = 0.0f; offset.f[1] = 0.0f; offset.f[2] = 0.0f; offset.f[3] = 0.0f;
                offset.f[2] = 0.5f;
                if (v->last_hit != hit) {
                    v->hit_count = 0;
                    v->last_hit = hit;
                } else {
                    v->hit_count++;
                    sub_AE6B0(&start, hit + 0x10, &v->previous_hit);
                    v->difference = start;
                }
                v->previous_hit = *(V_AE6B0 *)(hit + 0x10);
                object_hit = 1;
                v->landing = *(V_AE6B0 *)(hit + 0x10);
                found = 1;
                add_AE6B0(&offset, &offset, &v->landing);
                nudge_AE6B0(&offset, 0, 0.5f);
            }
        } else if (D_L00_00173F40_AE6B0[7] > 0) {
            v->landing = D_L00_00173F60_AE6B0;
            found = 1;
        }
    }
    if (*(unsigned char *)(D_0013E15A_AE6B0 + 0x4D0) != 0 && *(int *)(hero + 0x2084) == 1) {
        float z = velocity.f[2];
        scale_AE6B0(1.5f, &velocity, &velocity);
        velocity.f[2] = z / 1.5f;
    }
    hero_position = position;
    while (!timer_AE6B0(&timer) && !found) {
        previous = position;
        for (i = 9; i >= 0; i--) {
            add_AE6B0(&position, &position, &velocity);
            velocity.f[2] -= D_0015EE70_AE6B0 * 11.0f;
        }
        if (!sweep_AE6B0(&previous, &position, 0x10, m, 0))
            continue;
        if (!is_down_AE6B0() && 0.0f < velocity.f[2])
            continue;
        hit = (char *)D_L00_00173F40_AE6B0[6];
        if (hit != 0 && D_L00_00173F40_AE6B0[7] <= 0) {
            if (!(position.f[2] - hero_position.f[2] < 2.0f))
                goto check_surface;
            if ((hit == *(char **)(hero + 0x2080) || hit == v->holder) &&
                timer >= ticks_AE6B0(300) - ticks_AE6B0(10))
                continue;
            offset.f[0] = 0.0f; offset.f[1] = 0.0f; offset.f[2] = 0.0f; offset.f[3] = 0.0f;
            offset.f[2] = 0.5f;
            if (v->last_hit != hit) {
                v->hit_count = 0;
                v->last_hit = hit;
            } else {
                v->hit_count++;
                sub_AE6B0(&start, hit + 0x10, &v->previous_hit);
                v->difference = start;
            }
            v->previous_hit = *(V_AE6B0 *)(hit + 0x10);
            object_hit = 1;
            v->landing = *(V_AE6B0 *)(hit + 0x10);
            add_AE6B0(&offset, &offset, &v->landing);
            nudge_AE6B0(&offset, 0, 0.5f);
            break;
        }
check_surface:
        if (D_L00_00173F40_AE6B0[7] > 0) {
            v->landing = D_L00_00173F60_AE6B0;
            break;
        }
    }
    if (!timer)
        return 0;
    if (!mode) {
        hit = (char *)D_L00_00173F40_AE6B0[6];
        if (!hit)
            return 0;
        if (!check_AE6B0(hit))
            return 0;
    }
    hit = (char *)D_L00_00173F40_AE6B0[6];
    if (mode != 2 || !hit || !check_AE6B0(hit))
        arm_AE6B0((void *)cb_AE6B0, m);
    if (!object_hit && v->last_hit) {
        last = v->last_hit;
        if (*(unsigned char *)(last + 0x20) == 0xFE) {
            v->hit_count = 0;
        } else if (*(unsigned char *)(last + 0x20) == 0xFD) {
            v->hit_count = 0;
        } else if (!(*(unsigned short *)(last + 0x34) & 0x1000)) {
            v->hit_count = 0;
        } else {
            if (v->hit_count <= ticks_AE6B0(30))
                goto keep_hit;
            v->hit_count = 0;
        }
        v->last_hit = 0;
    }
keep_hit:
    v->normal = D_L00_00173F80_AE6B0;
    sub_AE6B0(&previous, &v->landing, &D_L00_00166EC0_AE6B0);
    scale_AE6B0(0.95f, &previous, &previous);
    add_AE6B0(&v->landing, &previous, &D_L00_00166EC0_AE6B0);
    return 1;
}
