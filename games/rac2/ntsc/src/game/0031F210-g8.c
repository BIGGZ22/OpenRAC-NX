#include "include_asm.h"
extern float D_L00_001B0464 __attribute__((sda));
extern float func_00282C48(float);
extern void func_00282F20(void *, void *, float);
extern void func_00282CC0(void *, void *, void *);
extern void func_00326EC0(void *);
extern void func_00327038(void *);
extern void func_00327380(void *);

void func_00326D90(char *object)
{
    float vector[4];
    char *data = *(char **)(object + 0x68);
    float speed = *(float *)(data + 0x30);
    if (speed != 0.0f && *(int *)(data + 0x34)) {
        float magnitude;
        float step;
        *(float *)(data + 0x18) += speed * (1.0f / 120.0f);
        magnitude = func_00282C48(*(float *)(data + 0x18));
        step = D_L00_001B0464;
        if (step * (1.0f / 60.0f) < magnitude)
            *(float *)(data + 0x18) = step * *(float *)(data + 0x30) * (1.0f / 60.0f);
    } else {
        *(float *)(data + 0x18) *= 0.85f;
    }
    *(float *)(object + 0x48) = *(float *)(data + 0x18) * 25.0f;
    if (*(int *)(data + 0x34) == 0)
        *(float *)(object + 0x48) += *(float *)(data + 0x38) * 2.0f;
    func_00282F20(vector, object + 0xC0, *(float *)(data + 0x18));
    func_00282CC0(object + 0x10, object + 0x10, vector);
    func_00326EC0(object);
    func_00327038(object);
    func_00327380(object);
}
extern char *D_L00_001B043C __attribute__((sda));
/* These scalar symbols anchor eight adjacent small-data words in each table. */
extern float D_L00_001B03F0 __attribute__((sda));
extern int D_L00_001B0410 __attribute__((sda));
extern float func_0029F530(void *);
extern int func_002E5CF8(int, int, void *);

void func_00326BD8(void)
{
    char *object = D_L00_001B043C;
    if (object) {
        char *data = *(char **)(object + 0x68);
        float current = func_0029F530(object);
        int i;
        for (i = 0; i < 8; i++) {
            float threshold = (&D_L00_001B03F0)[i];
            float delta = current - threshold;
            if ((0.0f < delta || 0.0f < *(float *)(data + 0x2C) - threshold) &&
                (delta < 0.0f || *(float *)(data + 0x2C) - threshold < 0.0f)) {
                func_002E5CF8((&D_L00_001B0410)[i], 0, D_L00_001B043C);
                break;
            }
        }
        *(float *)(data + 0x2C) = current;
    }
}
extern int ref_L00_00325B28_00 __attribute__((sda));
extern char ref_L00_00325B28_01 __attribute__((sda));
extern int ref_L00_00325B28_02;
extern void func_00281A08(void *, int, void *, void *, int, int, int, int, float);

void func_00325B28(char *object)
{
    int count = ref_L00_00325B28_00;
    int state = ref_L00_00325B28_02;
    char *data = *(char **)(object + 0x68);
    func_00281A08(&ref_L00_00325B28_01, 4, data + 0x320, data + 0x420,
        *(int *)(data + 0x2B4), *(int *)(data + 0x2B0),
        count, state, *(float *)(data + 0x2B8));
}

extern volatile int D_001AFFF0 __attribute__((sda));
extern volatile int D_001AFFF4 __attribute__((sda));
extern void func_00322138(char *);
extern void func_00322340(char *, char *, float, int, int, int);
void func_00321FE0(char *object)
{
    char *data = *(char **)(object + 0x68);
    unsigned int state = *(unsigned char *)(object + 0x20);
    if (state == 4 || state == 5 || *(unsigned char *)(object + 0x95) == 2 || state == 10)
        func_00322138(object);
    func_00322340(object, data + 0x3A0, 0.125f, D_001AFFF0, 0x20000000, 0x20000000);
    func_00322340(object, data + 0x3B0, 0.15f, D_001AFFF4, 0x30000000, 0x20000000);
    func_00322340(object, data + 0x3C0, 0.15f, D_001AFFF4, 0x10000000, 0x30000000);
}

extern unsigned short *D_001EE5A0[];
extern char *D_001B1B5C __attribute__((sda));

/* Walks the moby group list of the object's group id and raises the flag at data + 0x276
   on every peer of class 0xB0A whose class byte is not negative. */
void func_00321F60(char *self)
{
    if (*(unsigned char *)(self + 0x21) != 0xFF) {
        unsigned short *list = D_001EE5A0[*(unsigned char *)(self + 0x21)];
        if (list != 0) {
            char *base = D_001B1B5C;
            unsigned short *entry = list;
            int key;
            do {
                char *object = (char *)((unsigned int)base + ((*entry & 0x7FFF) << 8));
                if (object[0x20] >= 0 && *(short *)(object + 0xAA) == 0xB0A)
                    *(short *)(*(char **)(object + 0x68) + 0x276) = 1;
                key = *(short *)entry++;
            } while (key >= 0);
        }
    }
}

void func_00325320(char *self)
{
    if (*(unsigned char *)(self + 0x21) != 0xFF) {
        unsigned short *list = D_001EE5A0[*(unsigned char *)(self + 0x21)];
        if (list != 0) {
            char *base = D_001B1B5C;
            unsigned short *entry = list;
            int key;
            do {
                char *object = (char *)((unsigned int)base + ((*entry & 0x7FFF) << 8));
                if (object[0x20] >= 0 && *(short *)(object + 0xAA) == 0xB0B)
                    *(short *)(*(char **)(object + 0x68) + 0x276) = 1;
                key = *(short *)entry++;
            } while (key >= 0);
        }
    }
}
