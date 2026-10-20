#include "include_asm.h"

extern int func_00336E40(void *);
extern int *func_00336DA8(int, int);
extern void func_003361E0(void *, void *, void *);
extern void func_003369A8(void *, void *, void *);
extern void func_00336710(void *, void *, void *);
extern void func_00336A30(void *, int);
extern void func_00336A38(void *, int);
extern int *func_00335E20(void *);
extern void func_00336390(void *, void *, int);
extern void func_00335F88(void *, float, float, float, float);
extern void func_00336920(void *, int, int);
extern int func_00288FB8(int);
extern int func_0033B540(char *, int);
extern char D_001ADC10[], D_001ADC18[], D_001ADC20[], D_001ADC28[];
extern char D_001ADD58[], D_001ADD60[], D_001ADD68[], D_001ADD70[], D_001ADD78[];
extern char *D_001A8CFC;

/* Constructor of an on-screen map object: stores the owner, allocates the 16-byte anchor (255, 207)
   at +0x1C8, then names and tints three sprites ("0", "1", "2" at +0x0, +0x4C, +0x98), three more
   named "3" (+0xE4, +0x130, +0x17C), three texts (Title +0x1D8, Exit +0x268, Sel +0x220) and two
   map sprites (MapSpr +0x2B0, MapPlanetSpr +0x2EC), sets their strings and places them with
   func_0033B540. The three sprite loops keep their own counters (i, j, k): sharing one counter
   keeps the loop-invariant high half of the "3" name out of a register. */
void func_0033B170(char *self, char *owner)
{
    int i, j, k;

    *(char **)(self + 0x1CC) = owner;
    if (owner) {
        int *allocation = func_00336DA8(16, func_00336E40(owner));
        *(int **)(self + 0x1C8) = allocation;
        allocation[1] = 0;
        allocation[2] = 0;
        allocation[3] = 0;
        allocation[0] = 0;
    }
    *(int *)(self + 0x328) = 1;
    (*(float **)(self + 0x1C8))[0] = 255.0f;
    (*(float **)(self + 0x1C8))[1] = 207.0f;
    func_003361E0(self, D_001ADC10, owner);
    func_003361E0(self + 0x4C, D_001ADC18, owner);
    func_003361E0(self + 0x98, D_001ADC20, owner);
    for (i = 0; i < 3; i++)
        func_003361E0(self + 0xE4 + i * 0x4C, D_001ADC28, owner);
    func_003369A8(self + 0x1D8, D_001ADD58, owner);
    func_003369A8(self + 0x268, D_001ADD60, owner);
    func_003369A8(self + 0x220, D_001ADD68, owner);
    func_00336A30(self + 0x268, 0);
    func_00336A30(self + 0x220, 0);
    func_00336710(self + 0x2B0, D_001ADD70, owner);
    func_00336710(self + 0x2EC, D_001ADD78, owner);
    *func_00335E20(self) = 0x60442D00;
    *func_00335E20(self + 0x4C) = 0x60241700;
    *func_00335E20(self + 0x98) = 0x55F0C070;
    for (j = 0; j < 3; j++)
        *func_00335E20(self + 0xE4 + j * 0x4C) = 0x80FFDE8D;
    *func_00335E20(self + 0x2B0) = 0x70A0C0C0;
    *func_00335E20(self + 0x1D8) = 0x80F0F0F0;
    *func_00335E20(self + 0x268) = 0x80F0F0F0;
    *func_00335E20(self + 0x220) = 0x80F0F0F0;
    func_00336390(self, D_001A8CFC + 0x8710, 0x87);
    func_00336390(self + 0x4C, D_001A8CFC + 0x8710, 0x88);
    func_00336390(self + 0x98, D_001A8CFC + 0x8710, 0x89);
    for (k = 0; k < 3; k++)
        func_00336390(self + 0xE4 + k * 0x4C, D_001A8CFC + 0x8710, 0x8A);
    func_00335F88(self + 0x130, 1.0f, -1.0f, 0.0f, 0.0f);
    func_00336920(self + 0x2B0, 0xE99A, 10);
    func_00335F88(self + 0x2B0, 302.0f, 194.0f, 0.0f, 0.0f);
    func_00335F88(self + 0x2EC, 32.0f, 32.0f, 0.0f, 0.0f);
    *(int *)(self + 0x1D0) = 0;
    func_00336A38(self + 0x1D8, func_00288FB8(0x2D60));
    func_00336A38(self + 0x268, func_00288FB8(0x2D4F));
    func_00336A38(self + 0x220, func_00288FB8(0x2D75));
    func_0033B540(self, 0);
}

extern void func_00335E38(char *, float, float, float, float);
extern float *func_00335E10(char *);
extern float D_001ADD88 __attribute__((sda));
extern float D_001ADD8C;
extern float D_001ADD90;
extern float D_001ADD94;
extern float D_001ADD98;
extern float D_001ADD9C;
extern float D_001ADDA0 __attribute__((sda));
extern float D_001ADDA4;
extern float D_001ADDA8 __attribute__((sda));
extern float D_001ADDAC;
extern float D_001ADDB0 __attribute__((sda));
extern float D_001ADDB4;
extern float D_001ADDB8 __attribute__((sda));
extern float D_001ADDBC;
extern char D_001C4FA0[];

typedef struct { int first, second, third; float scale; } Entry_0033B540;
extern Entry_0033B540 D_00265140[];

/* Places the sprites of the object built by func_0033B170 around the anchor at +0x1C8 and returns
   0. While the word at D_001C4FA0 + 0x230 is below 0x15, the offset of the sprites at +0xE4 and
   +0x130 (horizontal, 5 at most per call) and of the one at +0x17C (vertical, 3 at most) is eased
   toward a cell of the 16-byte table D_00265140. g and g2 are two loads of the same base: the
   second branch rebuilds the address, as retail does. The second argument is not read. */
int func_0033B540(char *self, int unused)
{
    float dx, dy;
    float *anchor;
    char *g;

    anchor = *(float **)(self + 0x1C8);
    func_00335E38(self, anchor[0], anchor[1], 0.0f, 0.0f);
    anchor = *(float **)(self + 0x1C8);
    func_00335E38(self + 0x4C, anchor[0], anchor[1], 0.0f, 0.0f);
    anchor = *(float **)(self + 0x1C8);
    func_00335E38(self + 0x98, anchor[0], anchor[1], 0.0f, 0.0f);
    dx = dy = 0.0f;
    g = D_001C4FA0;
    if (*(int *)(g + 0x230) < 0x15) {
        float t;

        dx = func_00335E10(self + 0xE4)[0] - (D_001ADD88 + (*(float **)(self + 0x1C8))[0]);
        if (func_00335E10(self + 0xE4)[0] == 0.0f) {
            dx = 56.0f;
        } else {
            t = (float)D_00265140[*(int *)(g + 0x230)].first + D_00265140[*(int *)(g + 0x230)].scale * 0.5f - dx;
            if (t > 5.0f)
                t = 5.0f;
            else if (t < -5.0f)
                t = -5.0f;
            dx += t;
        }
        dy = func_00335E10(self + 0x17C)[1] - (D_001ADD9C + (*(float **)(self + 0x1C8))[1]);
        if (func_00335E10(self + 0x17C)[1] == 0.0f) {
            dy = -50.0f;
        } else {
            char *g2 = D_001C4FA0;
            t = (float)D_00265140[*(int *)(g2 + 0x230)].second + D_00265140[*(int *)(g2 + 0x230)].scale * 0.5f - dy;
            if (t > 3.0f)
                t = 3.0f;
            else if (t < -3.0f)
                t = -3.0f;
            dy += t;
        }
    }
    anchor = *(float **)(self + 0x1C8);
    func_00335E38(self + 0xE4, D_001ADD88 + anchor[0] + dx, D_001ADD8C + anchor[1], 0.0f, 0.0f);
    anchor = *(float **)(self + 0x1C8);
    func_00335E38(self + 0x130, D_001ADD90 + anchor[0] + dx, D_001ADD94 + anchor[1], 0.0f, 0.0f);
    func_00335E38(self + 0x17C, D_001ADD98 + (*(float **)(self + 0x1C8))[0], D_001ADD9C + (*(float **)(self + 0x1C8))[1] + dy, 0.0f, 0.0f);
    anchor = *(float **)(self + 0x1C8);
    func_00335E38(self + 0x1D8, D_001ADDA0 + anchor[0], D_001ADDA4 + anchor[1], 0.0f, 0.0f);
    anchor = *(float **)(self + 0x1C8);
    func_00335E38(self + 0x220, D_001ADDA8 + anchor[0], D_001ADDAC + anchor[1], 0.0f, 0.0f);
    anchor = *(float **)(self + 0x1C8);
    func_00335E38(self + 0x268, D_001ADDB0 + anchor[0], D_001ADDB4 + anchor[1], 0.0f, 0.0f);
    anchor = *(float **)(self + 0x1C8);
    func_00335E38(self + 0x2B0, D_001ADDB8 + anchor[0], D_001ADDBC + anchor[1], 0.0f, 0.0f);
    return 0;
}
