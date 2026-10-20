#include "include_asm.h"

typedef struct { char pad[0x68]; float angle; } Slot_002B5EF8;

typedef struct {
    char pad0[0x38];
    unsigned int flags;                 /* 0x38 */
    char pad1[0x48 - 0x3C];
    Slot_002B5EF8 *left;                /* 0x48 */
    Slot_002B5EF8 *right;               /* 0x4C */
} Rec_002B5EF8;

extern float func_00283BE0(float, float);
extern float func_00283B98(float, float);

void func_002B5EF8(char *self, Rec_002B5EF8 *other, float x)
{
    float k, w1, w2, inv, d;

    if (other->left == 0 && other->right == 0)
        return;
    if (other->flags & 0x10)
        return;
    w1 = 0.0f;
    if (other->left != 0)
        w1 = 1.0f;
    w2 = 0.0f;
    if (other->right != 0)
        w2 = 1.0f;
    inv = 1.0f / (w1 + w2);
    w1 *= inv;
    w2 *= inv;
    k = 1.5707965f;
    d = func_00283BE0(x, *(float *)(self + 0xF8));
    if (k < d)
        d = k;
    else if (d < -k)
        d = -k;
    if (*(unsigned short *)(self + 0x34) & 0x8000)
        d = -d;
    if (other->left != 0) {
        other->left->angle = func_00283B98(d * w1, other->left->angle);
        if (k < other->left->angle)
            other->left->angle = k;
        else if (other->left->angle < -k)
            other->left->angle = -k;
    }
    if (other->right != 0) {
        other->right->angle = func_00283B98(d * w2, other->right->angle);
        if (k < other->right->angle)
            other->right->angle = k;
        else if (other->right->angle < -k)
            other->right->angle = -k;
    }
}
