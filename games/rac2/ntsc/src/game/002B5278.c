#include "include_asm.h"

/* Measured partial C views; original types and object ownership are unknown. */
typedef unsigned char QwenRecovery_84e23fd54f9f_AbiByte16;

extern int func_002B50E0(QwenRecovery_84e23fd54f9f_AbiByte16 *owner, QwenRecovery_84e23fd54f9f_AbiByte16 *input, float factor);

typedef unsigned char u8;

void func_002B5278(QwenRecovery_84e23fd54f9f_AbiByte16 *param_1, float factor)
{
    func_002B50E0(param_1, param_1 + 0x10, factor);
}

INCLUDE_ASM("asm/game", func_002B5298);

INCLUDE_ASM("asm/game", func_002B52A0);

INCLUDE_ASM("asm/game", func_002B5678);

INCLUDE_ASM("asm/game", func_002B5828);

INCLUDE_ASM("asm/game", func_002B5BC0);

extern int func_002AD0E0(void *, void *, void *, void *, void *, void *);

void func_002B5CD8(char *first, char *second) {
    void *target = *(void **)(second + 0x88);
    if (target != 0 && func_002AD0E0(first, target, first + 0x10, first + 0xF0, first + 0x10, first + 0xF0) != 0)
        *(unsigned int *)(second + 0x94) |= 0x40;
}

void *func_002B5D28(u8 *a0) { return *(void **)(*(u8 **)(a0 + 0x68) + 0x18); }

INCLUDE_ASM("asm/game", func_002B5D38);

INCLUDE_ASM("asm/game", func_002B5E48);

INCLUDE_ASM("asm/game", func_002B5EF8);

INCLUDE_ASM("asm/game", func_002B60A8);

/* func_002B60B0 is matched in 002B60B0.c. */

INCLUDE_ASM("asm/game", func_002B6118);
