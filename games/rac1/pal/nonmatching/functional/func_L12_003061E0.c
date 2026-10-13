/* functional: high. Same reads, writes, calls and arguments as retail (the near miss's swapped stack arguments of func_L00_0025F4A8, its data+0x53 read for moby+0x53 and its late store of the step vector's y are fixed); only register use, scheduling and the order of independent stores differ, quad copies use qcopy, and the vector add helper func_001F9BD8 (xyz only) is still called, so nothing reads the unset w of the step vector. */
/* Level 12 (Hoven) moby update for a small hovering/leaping creature (class 0x4F5 family). data = moby+0x78 is its
 * state block. It first fetches a target point (func_L00_00260D30, radius 8) and the nearest hit of message
 * 0x230000 (func_L00_0025B478 / func_L00_0025B4D0, count into a local). When the target point is within 0.25 units,
 * or there is at least one other hit whose class is not 0x4F5, it bursts (func_L00_0025F4A8 with D_L12_0015F660),
 * drops its item (func_L00_002584A8) and deletes itself (func_0020D678). Otherwise a state machine on moby+0x20:
 * 0 setup (remember the spawn position at data+0x60, set flag 0x1000 at moby+0x34, turn rate +-40 degrees at
 * data+0x80 by the parent's side byte, tick counter data+0x84 = 0, go to 1); 1 the leap: for the first 30 ticks a
 * sine hop on z, after 15 ticks the speed data+0x70 grows (capped at 5.75 * frame scale), after 30 ticks sounds,
 * then it heads toward the target (atan2 of the xy offset, turned by the rate when closer than 7 units) and moves
 * by speed * (cos, sin, 0); after 30 ticks a z bob by sin(data+0x78), snapping to the ground (func_00214358, at
 * least the water level D_0013E633+0x10F5) with the spring func_L00_0025C918, and a collision probe: a sphere hit
 * on a moby other than the parent and not of class 0x4F5 (D_L12_00174358 = last hit moby), or a world hit,
 * sets state 2; 2 a bigger burst and deletion. States other than 2 end with func_L00_0025B040(moby, 1.0). */
extern int func_L00_00260D30_3061E0(void *, void *, float) __asm__("func_L00_00260D30");
extern char *func_L00_0025B478_3061E0(void *, int, int) __asm__("func_L00_0025B478");
extern int func_L00_0025B4D0_3061E0(void *, void *, void *, int, int *, float *, int, int) __asm__("func_L00_0025B4D0");
extern float func_001F9D48_3061E0(void *, void *) __asm__("func_001F9D48");
extern void func_L00_0025F4A8_3061E0(void *, void *, void *, float, float, int, int, int, float, float, float, int, float, float, int, int, int, int) __asm__("func_L00_0025F4A8");
extern void func_L00_002584A8_3061E0(void *, int, int) __asm__("func_L00_002584A8");
extern int func_001F9850_3061E0(int) __asm__("func_001F9850");
extern float func_001F9FA8_3061E0(float) __asm__("func_001F9FA8");
extern float func_001F9F90_3061E0(float) __asm__("func_001F9F90");
extern void func_00213DE0_3061E0(void *, int, int, int) __asm__("func_00213DE0");
extern float func_L00_001FF860_3061E0(float, float) __asm__("func_L00_001FF860");
extern float func_001FA748_3061E0(float, float) __asm__("func_001FA748");
extern void func_001F9BD8_3061E0(void *, void *, void *) __asm__("func_001F9BD8");
extern float func_00214358_3061E0(void *, int, float) __asm__("func_00214358");
extern void func_L00_0025C918_3061E0(float *, float *, float, float, float, float) __asm__("func_L00_0025C918");
extern void func_L00_0025A8C0_3061E0(void *, void *, int, float, void *) __asm__("func_L00_0025A8C0");
extern int func_L00_001F2BE8_3061E0(float, void *, int, void *, void *) __asm__("func_L00_001F2BE8");
extern int func_L00_001F10E0_3061E0(float, void *, int, void *) __asm__("func_L00_001F10E0");
extern void func_0020D678_3061E0(void *) __asm__("func_0020D678");
extern void func_L00_0025B040_3061E0(void *, float) __asm__("func_L00_0025B040");
extern char *D_L12_00174358_3061E0 __asm__("D_L12_00174358");
extern float D_L12_0015F660_3061E0[] __asm__("D_L12_0015F660") MACRO_ADDR;
extern float D_0015EE6C_3061E0 __asm__("D_0015EE6C") MACRO_ADDR;
extern float D_0015EE70_3061E0 __asm__("D_0015EE70") MACRO_ADDR;
extern float D_0015EE64_3061E0 __asm__("D_0015EE64") MACRO_ADDR;
extern unsigned char D_0013E633_3061E0[] __asm__("D_0013E633");

#define U8_3061E0(p, o) (*(unsigned char *)((char *)(p) + (o)))
#define I_3061E0(p, o) (*(int *)((char *)(p) + (o)))
#define F_3061E0(p, o) (*(float *)((char *)(p) + (o)))
#define P_3061E0(p, o) (*(char **)((char *)(p) + (o)))

void func_L12_003061E0(char *moby)
{
    char *data = P_3061E0(moby, 0x78);
    char *hit;
    char *pos;
    int cnt;
    int state;
    float target[4];
    float step[4];
    float probe[4];
    float dir[4];
    float sphere[4];
    float d;
    float heading;
    float turn;
    float t;
    float ground;
    float dz;

    func_L00_00260D30_3061E0(moby, target, 8.0f);
    hit = func_L00_0025B478_3061E0(moby, 0x230000, 0);
    func_L00_0025B4D0_3061E0(moby, hit, data + 0x20, 0, &cnt, (float *)0, 0, 4);
    d = func_001F9D48_3061E0(moby + 0x10, target);

    if (!(d < 0.25f)) {
        if (cnt < 2 || hit == 0 || *(short *)(P_3061E0(hit, 0x20) + 0xA6) == 0x4F5) {
            goto states;
        }
    }
    func_L00_0025F4A8_3061E0(moby, D_L12_0015F660_3061E0, 0, 0.0f, 0.0f, 3, 3, 5, 1.0f, 0.5f, 4.0f, 2, 0.7f, 7.0f,
                             0, 0, -1, 0);
    func_L00_002584A8_3061E0(moby, 0, -1);
    func_0020D678_3061E0(moby);
    return;

states:
    state = U8_3061E0(moby, 0x20);
    if (state == 0) {
        *(unsigned short *)(moby + 0x34) = *(unsigned short *)(moby + 0x34) | 0x1000;
        qcopy(data + 0x60, moby + 0x10);
        U8_3061E0(moby, 0x20) = 1;
        F_3061E0(data, 0x80) = 0.698131680f;
        if (P_3061E0(moby, 0xB8) != 0 && U8_3061E0(P_3061E0(moby, 0xB8), 0xBC) != 0) {
            F_3061E0(data, 0x80) = -0.698131680f;
        }
        I_3061E0(data, 0x84) = 0;
    } else if (state == 2) {
        func_L00_0025F4A8_3061E0(moby, D_L12_0015F660_3061E0, 0, 0.0f, 0.0f, 5, 0xF, 0x19, 2.0f, 1.0f, 4.0f, 2, 1.0f,
                                 7.0f, 0, 9, -1, 0);
        func_0020D678_3061E0(moby);
        return;
    } else if (state == 1) {
        I_3061E0(data, 0x84) = I_3061E0(data, 0x84) + 1;
        cnt = func_001F9850_3061E0(0x1E);
        if (I_3061E0(data, 0x84) < cnt) {
            int m = func_001F9850_3061E0(0x1E);
            t = func_001F9FA8_3061E0((float)I_3061E0(data, 0x84) * 3.14159274f / (float)m);
            F_3061E0(moby, 0x18) = F_3061E0(moby, 0x18) + t * 0.0700000003f;
        }
        if (func_001F9850_3061E0(0xF) < I_3061E0(data, 0x84)) {
            F_3061E0(data, 0x70) = F_3061E0(data, 0x70) + D_0015EE70_3061E0 * 30.0f;
        }
        if (func_001F9850_3061E0(0x1E) < I_3061E0(data, 0x84)) {
            if (U8_3061E0(moby, 0x53) == 0) {
                func_00213DE0_3061E0(moby, 1, 0, 2);
            }
            if (U8_3061E0(moby, 0x53) == 1 && (U8_3061E0(moby, 0x70) & 2) != 0) {
                func_00213DE0_3061E0(moby, 2, 0, 2);
            }
        }
        t = D_0015EE6C_3061E0 * 5.75f;
        if (t < F_3061E0(data, 0x70)) {
            F_3061E0(data, 0x70) = t;
        }

        pos = moby + 0x10;
        heading = func_L00_001FF860_3061E0(target[0] - F_3061E0(moby, 0x10), target[1] - F_3061E0(moby, 0x14));
        turn = heading;
        if (func_001F9D48_3061E0(pos, target) < 7.0f) {
            turn = func_001FA748_3061E0(heading, F_3061E0(data, 0x80));
        }
        step[0] = func_001F9F90_3061E0(turn) * F_3061E0(data, 0x70);
        t = func_001F9FA8_3061E0(turn);
        I_3061E0(step, 8) = 0;
        step[1] = t * F_3061E0(data, 0x70);
        func_001F9BD8_3061E0(pos, pos, step);

        if (func_001F9850_3061E0(0x1E) < I_3061E0(data, 0x84)) {
            F_3061E0(data, 0x78) = func_001FA748_3061E0(F_3061E0(data, 0x78), D_0015EE6C_3061E0 * 5.23598766f);
            F_3061E0(moby, 0x18) = F_3061E0(moby, 0x18) - F_3061E0(data, 0x74);
            F_3061E0(data, 0x74) = func_001F9FA8_3061E0(F_3061E0(data, 0x78)) * 0.100000001f;
            F_3061E0(moby, 0x18) = F_3061E0(moby, 0x18) + F_3061E0(data, 0x74);
        }
        if (func_001F9850_3061E0(0x1E) < I_3061E0(data, 0x84)) {
            ground = func_00214358_3061E0(pos, 0, 0.5f);
            if (ground < F_3061E0(D_0013E633_3061E0, 0x10F5)) {
                ground = F_3061E0(D_0013E633_3061E0, 0x10F5);
            }
            dz = ground - F_3061E0(moby, 0x18);
            if (dz < 0.5f && -3.0f < dz) {
                func_L00_0025C918_3061E0((float *)(moby + 0x18), (float *)(data + 0x7C), ground + 0.579999983f,
                                         D_0015EE64_3061E0 * 0.0199999996f, D_0015EE64_3061E0 * 0.300000012f,
                                         D_0015EE6C_3061E0 * 4.0f);
            }
        }
        if (func_001F9850_3061E0(0x1E) < I_3061E0(data, 0x84)) {
            dir[0] = func_001F9F90_3061E0(heading) * 0.300000012f;
            dir[1] = func_001F9FA8_3061E0(heading) * 0.300000012f;
            dir[3] = 5627.9248f;
            dir[2] = 1.20000005f;
            func_L00_0025A8C0_3061E0(probe, moby, 0x10001, 1.0f, dir);
            qcopy(sphere, pos);
            sphere[2] = sphere[2] + 0.400000006f;
            if (func_L00_001F2BE8_3061E0(0.400000006f, sphere, 0x10, moby, probe) != 0) {
                char *other = D_L12_00174358_3061E0;
                if (other != 0 && *(short *)(other + 0xA6) != 0x4F5 && other != P_3061E0(moby, 0xB8)) {
                    U8_3061E0(moby, 0x20) = 2;
                    goto done;
                }
            }
            qcopy(sphere, pos);
            sphere[2] = sphere[2] + 0.349999994f;
            if (func_L00_001F10E0_3061E0(0.349999994f, sphere, 0, moby) != 0) {
                U8_3061E0(moby, 0x20) = 2;
            }
        }
    }
done:
    func_L00_0025B040_3061E0(moby, 1.0f);
}
