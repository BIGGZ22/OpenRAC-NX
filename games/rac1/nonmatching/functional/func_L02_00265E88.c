/* functional: medium. The NTSC decomp's pending C for the same instructions (a draft it has not matched
   yet), carried to PAL's names by OpenRAC's tools/port.py. Never compared with retail's bytes here. */
extern int D_L02_0015F6B0 MACRO_ADDR;
extern float D_0015EE6C MACRO_ADDR;
extern float D_0015EE70 MACRO_ADDR;
extern float D_L02_001744E0[];
extern short D_L02_00160150;
extern short D_L02_00160154;
extern short D_L02_00160158;
extern short D_L02_0016015C;
extern short D_L02_00160160;
extern int func_L00_001EFFF0(void *, void *, int, int, int);
extern float func_001F9D10(void *, void *);
extern void func_L00_001FF4B0(void *, void *, float);
extern void func_001F9BD8(void *, void *, void *);
extern void func_001F9C30(void *, void *, float);
extern void func_00214D88(float *, float *, float, float, float, float);
extern void func_L00_00258DB0(float *, float, float);
extern float func_002140F8(float, float);
extern float func_001F9878(float);
extern int func_001FA898(float);
extern int func_001F9938(void *);
extern char *func_L00_0026C0D0(void *, void *, unsigned long, float);
extern void func_L00_0026E438(void *, void *, int);

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/overlays/shared/gameplay/entities/00264e40.c, FUN_L02_00264e70. */
void func_L02_00265E88(char *slots, float *position, float *direction, float distance) {
    float origin[4] __attribute__((aligned(16)));
    float end[4] __attribute__((aligned(16)));
    float adjusted[4] __attribute__((aligned(16)));
    float velocity[4] __attribute__((aligned(16)));
    float offset[4] __attribute__((aligned(16)));
    float travel_step = 0.0f;
    float speed = 0.0f;
    float travel;
    float acceleration;
    int count;
    int first_count;
    int i;
    int save_shot;
    char *shot;

    save_shot = (D_L02_0015F6B0 & 3) == 0;
    qcopy(origin, position);
    func_L00_001FF4B0(adjusted, direction, distance + 0.75f);
    func_001F9BD8(end, adjusted, origin);
    if (func_L00_001EFFF0(origin, end, 2, 0, 0)) {
        travel = func_001F9D10(origin, D_L02_001744E0) - 0.75f;
        if (travel < 0.0f) travel = 0.0f;
    } else {
        travel = distance;
    }
    acceleration = D_0015EE70;
    func_00214D88(&speed, &travel_step, travel, 1.0f, acceleration * 48.0f,
                  (*(float *)&D_L02_00160150) * D_0015EE6C);
    func_L00_001FF4B0(adjusted, adjusted, travel_step);
    first_count = (*(int *)&D_L02_00160154);
    if (travel < 4.0f) first_count = 1;
    count = (*(int *)&D_L02_00160158) + first_count;
    for (i = 0; i < count; i++) {
        func_L00_00258DB0(offset, 0.0f,
            (((*(float *)&D_L02_0016015C) - (*(float *)&D_L02_00160160)) * (travel / distance) + (*(float *)&D_L02_00160160)) * D_0015EE6C);
        func_001F9BD8(offset, offset, adjusted);
        func_001F9C30(velocity, adjusted, func_002140F8(0.0f, 1.0f));
        func_001F9BD8(velocity, velocity, origin);
        shot = func_L00_0026C0D0(velocity, offset, i < first_count ? 1ULL : 0ULL, travel);
        if (i == 2 && *(volatile int *)&save_shot) {
            int j;
            for (j = 0; j < 7; j++) {
                if (((char **)slots)[j] == 0) {
                    ((char **)slots)[j] = shot;
                    break;
                }
            }
        }
    }
    if (func_001F9938(slots + 0x1c)) {
        speed = func_002140F8(10.0f, 30.0f);
        speed = func_001F9878(speed);
        *(short *)(slots + 0x1c) = func_001FA898(speed);
        func_L00_00258DB0(offset, 0.0f, D_0015EE6C + D_0015EE6C);
        func_001F9C30(velocity, adjusted, func_002140F8(0.5f, 0.1f));
        func_001F9BD8(velocity, velocity, offset);
        velocity[3] = 0.0f;
        func_L00_0026E438(position, velocity, 0);
    }
}
