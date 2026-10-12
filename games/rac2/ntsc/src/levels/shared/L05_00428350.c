#include "include_asm.h"

extern void func_00282CF0_9(char *local, char *first, char *second);

extern void func_00282CC0_14(char *target, char *first, char *local);

extern char *D_L05_001B9710;

extern int func_00282978(void *);

extern void func_0029F338(void *);

/* The call target and the callback it registers are not the same function in
   every level this unit is placed in, so neither name may state an address. */
extern void ref_L05_0042E4E8_00(void);

extern int ref_L05_0042E4E8_01(void *, void *);

#ifndef RAC2_T_VEC16_FC936841D
#define RAC2_T_VEC16_FC936841D
typedef struct {
    float x;
    float y;
    float z;
    float w;
} Vec16_Fc936841d;
#endif

INCLUDE_ASM("asm/levels", func_L05_00428350);

INCLUDE_ASM("asm/levels", func_L05_00428518);

INCLUDE_ASM("asm/levels", func_L05_00428AC8);

INCLUDE_ASM("asm/levels", func_L05_00429530);

INCLUDE_ASM("asm/levels", func_L05_0042AC30);

INCLUDE_ASM("asm/levels", func_L05_0042AFE0);

INCLUDE_ASM("asm/levels", func_L05_0042B2C0);

INCLUDE_ASM("asm/levels", func_L05_0042BB10);

INCLUDE_ASM("asm/levels", func_L05_0042BCE0);

INCLUDE_ASM("asm/levels", func_L05_0042BDB0);

INCLUDE_ASM("asm/levels", func_L05_0042C928);

/* Two bound helpers at the node's +0x68; the second call registers a callback
   and the context, the other branch calls a one-argument helper. */
void func_L05_0042E4E8(char *a0)
{
    char *context = a0;
    char *node = *(char **)(context + 0x68);

    if (func_00282978(node) == 0) {
        func_00282978(node + 4);
        ref_L05_0042E4E8_01(&ref_L05_0042E4E8_00, context);
    } else {
        func_0029F338(context);
    }
}

INCLUDE_ASM("asm/levels", func_L05_0042F4E8);

INCLUDE_ASM("asm/levels", func_L05_00430318);

/* Derives a 16-byte element at index-1 from its index-2 and index-3 neighbours through two bound helpers; original game role is unknown. */
void func_L05_00430BF0(Vec16_Fc936841d *object, int index)
{
    char local[16];

    func_00282CF0_9(local, (char *)&object[index - 2], (char *)&object[index - 3]);
    func_00282CC0_14((char *)&object[index - 1], (char *)&object[index - 2], local);
}

INCLUDE_ASM("asm/levels", func_L05_00432078);

INCLUDE_ASM("asm/levels", func_L05_00432080);

INCLUDE_ASM("asm/levels", func_L05_00432158);

INCLUDE_ASM("asm/levels", func_L05_004333F8);

INCLUDE_ASM("asm/levels", func_L05_00441D40);

INCLUDE_ASM("asm/levels", func_L05_00441EA0);

INCLUDE_ASM("asm/levels", func_L05_00441F30);

INCLUDE_ASM("asm/levels", func_L05_00442398);

INCLUDE_ASM("asm/levels", func_L05_004424B8);

INCLUDE_ASM("asm/levels", func_L05_00442560);

INCLUDE_ASM("asm/levels", func_L05_00442720);

INCLUDE_ASM("asm/levels", func_L05_004427A8);

INCLUDE_ASM("asm/levels", func_L05_00442828);

INCLUDE_ASM("asm/levels", func_L05_00442880);

INCLUDE_ASM("asm/levels", func_L05_00442A58);

INCLUDE_ASM("asm/levels", func_L05_00442F90);

INCLUDE_ASM("asm/levels", func_L05_004431A0);

INCLUDE_ASM("asm/levels", func_L05_004432E0);

INCLUDE_ASM("asm/levels", func_L05_00443740);

INCLUDE_ASM("asm/levels", func_L05_00443C18);

INCLUDE_ASM("asm/levels", func_L05_00443D08);

INCLUDE_ASM("asm/levels", func_L05_004445E8);

INCLUDE_ASM("asm/levels", func_L05_00444840);

INCLUDE_ASM("asm/levels", func_L05_00444B20);

INCLUDE_ASM("asm/levels", func_L05_00444D60);

INCLUDE_ASM("asm/levels", func_L05_00444DF8);

INCLUDE_ASM("asm/levels", func_L05_00444FD0);

INCLUDE_ASM("asm/levels", func_L05_00445338);

/* Reads a state word from a fixed global; unless its +0x86 halfword is 0x19 it
   returns 0, otherwise it reports whether the signed halfword reached through
   the pointer at +0x70, offset +0xD8, is non-zero. */
int func_L05_00445980(void)
{
    char *base = D_L05_001B9710;

    if (*(short *)(base + 0x86) != 0x19)
        return 0;
    return *(short *)(*(char **)(base + 0x70) + 0xD8) != 0;
}

INCLUDE_ASM("asm/levels", func_L05_004459B0);

INCLUDE_ASM("asm/levels", func_L05_00445A98);

INCLUDE_ASM("asm/levels", func_L05_00445B58);

INCLUDE_ASM("asm/levels", func_L05_00445C38);

INCLUDE_ASM("asm/levels", func_L05_00445D60);

INCLUDE_ASM("asm/levels", func_L05_00445F90);
