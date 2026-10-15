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
