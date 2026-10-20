#include "include_asm.h"

/* ---- shared declarations ---- */

extern int func_002ED688(unsigned char *owner);
extern void func_L05_0037AEC0(char *object, int flag);
extern void func_L05_00379C68(int arg0, int value);
extern void func_L00_002E6DB8(void *, void *);
extern void func_L00_002E6CF0(void *a0, void *a1);
extern void func_L00_002E6EF8(void *, int);
extern void func_L15_00438238(void);
extern void func_L15_00438260(void);
extern void func_L15_00438518(void);

extern void func_002FC560(int, int);
extern void func_0027DE50(int, int, int, int, long);
extern int func_00282948(void *);
extern void func_L15_00432A60(void *, int, int, float, float, float, int, int, int);
extern short D_001A6480[];
extern char D_L15_00269FE0[];

/* Sets the level's four effect blocks up: the screen context first, then, when
   the level's own record says the object is idle, the four blocks of the
   effect data, each with its own size and colour. */
void func_L15_004328E8(void *a0) {
    char *s0 = *(char **)(a0 + 0x68);

    func_002FC560(0x42, 0x48);
    func_0027DE50(0, D_001A6480[0xA9], 0, D_001A6480[0xA8], 0x40404040);
    if (func_00282948(s0 + 0x50) == 0) {
        char *q = D_L15_00269FE0;

        func_L15_00432A60(q, 0x46, 0x46, 3.2f, 6.35f, 0.2f, 0x0A, 0x007F7F7F, 0x00FF1010);
        func_L15_00432A60(q + 0xA0, 0x20, 0x20, 3.1f, 6.19f, 0.2f, 0x0A, 0x007F7F7F, 0x00FF1040);
        func_L15_00432A60(q + 0x140, 0x20, 0x20, 3.1f, 6.19f, 0.2f, 0x0A, 0x007F7F7F, 0x00FF1040);
        func_L15_00432A60(q + 0x1E0, 0x20, 0x20, 3.1f, 6.19f, 0.2f, 0x0A, 0x007F7F7F, 0x00FF1040);
    }
}

INCLUDE_ASM("asm/levels", func_L15_00432A60);

INCLUDE_ASM("asm/levels", func_L15_00434100);

INCLUDE_ASM("asm/levels", func_L15_00434610);

INCLUDE_ASM("asm/levels", func_L15_00434D28);

INCLUDE_ASM("asm/levels", func_L15_00434F90);

typedef struct GornEffectCamera {
    char pad00[0x140];
    float position[4];
} GornEffectCamera;
extern GornEffectCamera D_L15_001B9680;
extern unsigned char *D_L15_001FF4AC;
extern void func_0029FCC0(void *, int, void *);
extern float func_00283248(float, float);
extern float func_002A78D8(float, float);
extern float func_002A7878(float, float);
extern float func_00283B98(float, float);
extern void func_002AF000(void *, float, float, float);
extern unsigned char *func_002BDCE8(float, float, float, float,
                                   void *, int, void *, int, int);
extern int func_002A7820(int, int);
extern int func_00283CF0(float);
extern float func_00283CE0(int);



void func_L15_004351E0(char *object, int joint, float radius) {
    float initial[4], point[4], velocity[4];
    float heading, speed, tilt, yaw;
    unsigned char *particle;
    unsigned char *data;

    func_0029FCC0(object, joint, initial);
    heading = func_00283248(D_L15_001B9680.position[0] - *(float *)(object + 0x10),
                            D_L15_001B9680.position[1] - *(float *)(object + 0x14));
    point[0] = initial[0] + func_002A78D8(0, radius);
    point[1] = initial[1] + func_002A78D8(0, radius);
    point[2] = initial[2] + func_002A78D8(0, radius);
    speed = func_002A7878(radius * 0.008333333767950535f,
                          radius * 0.01666666753590107f);
    tilt = func_002A78D8(0, 1.396263599395752f);
    yaw = func_00283B98(heading, func_002A78D8(0, 1.5707964897155762f));
    func_002AF000(velocity, speed, yaw, tilt);
    particle = func_002BDCE8(0, 1.0f, 1.01f, radius * 210000.0f,
                            point, -1, velocity, 0x307F7F7F, 0);
    if (particle != 0) {
        particle[3] = 0x48;
        *(short *)(particle + 0xA) = func_002A7820(40, 70);
        particle[9] = func_00283CF0(4.0f) + 0x60;
        data = particle + 0x20;
        *(float *)(data + 0x1C) = func_00283CE0(func_002A7820(1, 10));
        *(int *)(data + 4) = 3;
        data[0xA] = 0x7F;
        data[0xB] = particle[0xA];
        particle[2] = *D_L15_001FF4AC;
    }
}

INCLUDE_ASM("asm/levels", func_L15_004353E8);

INCLUDE_ASM("asm/levels", func_L15_00435838);

INCLUDE_ASM("asm/levels", func_L15_00435E80);

void func_L15_00436090(char *a0) {
    char *v1 = *(char **)(a0 + 0x68);

    *(int *)(v1 + 0x270) = 1;
    *(int *)(v1 + 0x274) = 0x1E0;
    *(char *)(a0 + 0x23) = 0x20;
    *(char *)(a0 + 0x20) = 2;
    *(unsigned short *)(a0 + 0x34) |= 8;
    *(float *)(v1 + 0x180) = 450.0f;
    *(int *)(a0 + 0x98) = 0;
}

INCLUDE_ASM("asm/levels", func_L15_004360D8);

extern int func_002E5CF8(int, int, void *);
extern void func_L05_00379FA0(void *, int, int, float, int, float, float, float,
                              int, int, int);

/* The mover's own state decides whether the shot happens: the level's record
   must exist and be ready, and the mover's own counter must be idle.  The two
   shots take the slots after the counter, and the counter wraps at three. */
void func_L15_00436298(char *a0) {
    char *s1 = a0;
    char *s0;
    char *v0;
    int v1;

    s0 = *(char **)(s1 + 0x68);
    v0 = (char *)func_002ED688(a0);
    if (v0 == 0) {
        return;
    }
    if (*(int *)(v0 + 0x120) == 0) {
        return;
    }
    if (*(int *)(s0 + 0x450) != 0) {
        return;
    }
    func_002E5CF8(2, 0, s1);
    func_L05_00379FA0(s1, 0x600000CF, 1, 1.0f, 0, 2.0f, 1.5f, 1.5f, 0x1E,
                     *(int *)(s0 + 0x454) * 2 + 4, 0);
    func_L05_00379FA0(s1, 0x600000CF, 1, 1.0f, 0, 2.0f, 1.5f, 1.5f, 0x1E,
                     *(int *)(s0 + 0x454) * 2 + 5, 0);
    *(int *)(s0 + 0x450) = func_002A7820(0x14, 0x3C);
    v1 = *(int *)(s0 + 0x454) + 1;
    *(int *)(s0 + 0x454) = v1;
    if (v1 >= 3) {
        *(int *)(s0 + 0x454) = 0;
    }
}

INCLUDE_ASM("asm/levels", func_L15_004363C8);

INCLUDE_ASM("asm/levels", func_L15_00436990);

void func_L15_00436BA8(char *a0) {
    char *s0 = a0;
    char *s1 = *(char **)(s0 + 0x68);
    char *s2 = (char *)func_002ED688((unsigned char *)a0);

    *(int *)(s2 + 0x148) = 100;
    func_L05_0037AEC0(s0, 1);
    *(float *)(s1 + 0x180) = 100.0f;
    *(unsigned char *)(s0 + 0x20) = 1;
    *(unsigned char *)(s0 + 0x30) = 0xFF;
    *(unsigned short *)(s0 + 0x32) = 0x1FF;
    *(unsigned short *)(s0 + 0x34) |= 0x100;
    *(float *)(s0 + 0x2C) = 0.20000000298023224f;
    *(float *)(s1 + 0x278) = 10.0f;
    *(int *)(s1 + 0x26C) = 0;
    *(int *)(s1 + 0x270) = 0;
    *(int *)(s1 + 0x274) = 0;
    *(int *)(s2 + 0x154) = (int)(s1 + 0x278);
    func_L05_00379C68((int)s0, 0xFFFF);
}

INCLUDE_ASM("asm/levels", func_L15_00436C68);

void func_L15_00437B18(void *a0) {
    func_L00_002E6DB8((void *)func_L15_00438238, a0);
    func_L00_002E6CF0((void *)func_L15_00438260, a0);
    func_L00_002E6EF8((void *)func_L15_00438518, a0);
}

INCLUDE_ASM("asm/levels", func_L15_00437B68);

INCLUDE_ASM("asm/levels", func_L15_00438260);

INCLUDE_ASM("asm/levels", func_L15_00438518);

INCLUDE_ASM("asm/levels", func_L15_00438B00);

INCLUDE_ASM("asm/levels", func_L15_004390A0);
