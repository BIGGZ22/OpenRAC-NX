#include "include_asm.h"
#include "common.h"

INCLUDE_ASM("asm/levels", func_L02_003DB310);

INCLUDE_ASM("asm/levels", func_L02_003DB6D8);

void func_L02_003DB870(char *arg0, int arg1) {
    *(int *)(*(char **)(arg0 + 0x68) + 0x264) = arg1;
}

INCLUDE_ASM("asm/levels", func_L02_003DB880);

void func_L02_003DB938(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);

    if (*(short *)(state + 0x280) != 0)
        *(short *)(state + 0x280) = 0;
}

extern void func_002A7750();

int func_L02_003DB950(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    char *other = *(char **)(state + 0x264);
    unsigned char kind;
    int result = 0;

    if (other == 0)
        goto done;
    kind = *(unsigned char *)(other + 0x20);
    if (kind == 0xFE)
        goto done;
    if (kind == 0xFD)
        goto done;
    if (kind < 3)
        goto done;
    if (*(float *)(state + 0x20) <
        (float)*(short *)(state + 0x24) *
            (3.0f - (float)*(short *)(state + 0x282)) * 0.33333334f)
        result = 1;
done:
    return result;
}

INCLUDE_ASM("asm/levels", func_L02_003DBA70);

INCLUDE_ASM("asm/levels", func_L02_003DBB70);

extern char D_L02_001BA500;

extern float func_00282E48();

extern void func_002A93B0();

void func_L02_003DBE60(char *arg0) {
    if (*(unsigned char *)(arg0 + 0x31) != 0) {
        if (func_00282E48(arg0 + 0x10, &D_L02_001BA500) < 64.0f) {
            func_002A93B0(arg0);
            *(unsigned char *)(arg0 + 0x6F) = 60;
        }
    }
}

INCLUDE_ASM("asm/levels", func_L02_003DBF40);

INCLUDE_ASM("asm/levels", func_L02_003DBFD8);

INCLUDE_ASM("asm/levels", func_L02_003DEDA0);

INCLUDE_ASM("asm/levels", func_L02_003E6D70);

INCLUDE_ASM("asm/levels", func_L02_003E7370);

INCLUDE_ASM("asm/levels", func_L02_003E75A8);

INCLUDE_ASM("asm/levels", func_L02_003E77E0);

INCLUDE_ASM("asm/levels", func_L02_003E79F8);

INCLUDE_ASM("asm/levels", func_L02_003E7B58);

INCLUDE_ASM("asm/levels", func_L02_003E7F28);

INCLUDE_ASM("asm/levels", func_L02_003E80F8);

INCLUDE_ASM("asm/levels", func_L02_003E87D8);

INCLUDE_ASM("asm/levels", func_L02_003E8CE8);

INCLUDE_ASM("asm/levels", func_L02_003E8E90);

INCLUDE_ASM("asm/levels", func_L02_003E9040);

INCLUDE_ASM("asm/levels", func_L02_003E91F0);

INCLUDE_ASM("asm/levels", func_L02_003E98A8);

INCLUDE_ASM("asm/levels", func_L02_003E9BB0);

INCLUDE_ASM("asm/levels", func_L02_003EA180);

extern void func_31E6A0(char *, int, int);

/* Re-arms the actor's sub-object and clears the low bits of its flags. */
void func_L02_003EA488(char *arg0) {
    char *p = *(char **)(arg0 + 0x68);

    if (*(unsigned char *)(arg0 + 0x20) != 0)
        return;
    *(int *)(p + 0x5C) |= 4;
    func_31E6A0(arg0, 1, -1);
    *(int *)(arg0 + 0x98) = 0;
    *(unsigned short *)(arg0 + 0x34) =
        (*(unsigned short *)(arg0 + 0x34) & 0xEFFF) | 0x43;
}

INCLUDE_ASM("asm/levels", func_L02_003EA4E0);

INCLUDE_ASM("asm/levels", func_L02_003EAA68);

INCLUDE_ASM("asm/levels", func_L02_003EAE10);

/* func_L02_003EB098 is defined in L02_003DB310-g8.c (it reads D_L02_001B35B8 through $gp). */

/* func_L02_003EB138 is defined in L02_003DB310-g8.c (it reads D_L02_001B35B8 through $gp). */

INCLUDE_ASM("asm/levels", func_L02_003EB258);

INCLUDE_ASM("asm/levels", func_L02_003EB648);

INCLUDE_ASM("asm/levels", func_L02_003EE938);

INCLUDE_ASM("asm/levels", func_L02_003EEAD8);

INCLUDE_ASM("asm/levels", func_L02_003EEC58);

INCLUDE_ASM("asm/levels", func_L02_003EEDD0);

typedef int TI128 __attribute__((mode(TI)));
extern char *D_L02_001FF1C0[];
extern char D_00189EA0[];
extern int func_L02_00348420(void *, char *);

/* True when D_00189EA0 is within the radius at +0x2C of the actor's position (+0x20) and also
   of one of the 16-byte points of the actor's table entry (taking every +0x40 of them); the
   point is then copied to a local vector that goes to func_L02_00348420 with the entry. */
int func_L02_003EEFB8(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    char *entry = D_L02_001FF1C0[*(int *)state];
    TI128 buf;
    int i;

    if (*(float *)(state + 0x2C) < func_00282E48(D_00189EA0, state + 0x20))
        return 0;
    for (i = 0; i < *(int *)entry; i += *(int *)(state + 0x40)) {
        if (func_00282E48(D_00189EA0, entry + (i * 16 + 0x10)) < *(float *)(state + 0x2C)) {
            buf = *(TI128 *)D_00189EA0;
            *(int *)(state + 0x30) = func_L02_00348420(&buf, entry);
            return 1;
        }
    }
    return 0;
}

INCLUDE_ASM("asm/levels", func_L02_003F0798);

INCLUDE_ASM("asm/levels", func_L02_003F1298);

extern void func_002B0990(char *, char *, float, int, int);

extern void func_002A8C28(char *, char *, int, float, float, float, int, int, int);

void func_L02_003F1578(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);

    if (*(unsigned short *)(arg0 + 0x96) < 0xFFFF)
        (*(unsigned short *)(arg0 + 0x96))++;
    func_002B0990(arg0, state + 0x220, 2.5f, 7, 1);
    func_002A8C28(arg0, arg0 + 0x10, 0x4000000, 1.3f, 1.0f, 1.0f, 0, 1, 0);
}

extern void func_002A91F0(char *, char *);

INCLUDE_ASM("asm/levels", func_L02_003F1608);

/* func_L02_003F16A8 is defined in L02_003DB310-g8.c (this unit has no $gp access for it). */

extern int func_L02_003FC0D0(int, short);

extern int func_002B3BA8(char *, int, float, float, int, int);

void func_L02_003F16F0(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);

    if (*(unsigned char *)(arg0 + 0x20) < 4) {
        *(int *)(state + 0x4B0) = 0;
        goto done;
    }

    if (*(short *)(state + 0x4EA) == 1) {
        *(short *)(state + 0x4E8) =
            func_002B3BA8(arg0, *(int *)(state + 0x4E4), 100.0f, 100.0f, 0, 0);
        *(int *)(state + 0x4B0) = *(int *)(state + 0x210);
    } else {
        if (func_L02_003FC0D0(*(int *)(arg0 + 0xB8), *(short *)(arg0 + 0xAA)) == 0)
            goto done;
        *(short *)(state + 0x4EA) = 1;
        *(short *)(state + 0x4E8) =
            func_002B3BA8(arg0, *(int *)(state + 0x4E4), 100.0f, 100.0f, 0, 0);
        *(int *)(state + 0x4B0) = *(int *)(state + 0x210);
    }
    return;

done:
    *(short *)(state + 0x4EA) = 0;
}

extern void *func_002B60B0(u8 *, float, float, float, float);

extern void func_002A0488(void *, u8, u8, u8);

/* Sets the actor up: re-registers its sub-objects and arms its effects. */
void func_L02_003F17B0(char *arg0, int arg1) {
    char *state = *(char **)(arg0 + 0x68);
    int i;

    *(float *)(state + 0x20) = (float)*(short *)(state + 0x24);
    *(u16 *)(state + 0x98) = 0;
    func_002B60B0((u8 *)arg0, 0.004444444552063942f, 0.13333334028720856f,
                  0.0011635529808700085f, 0.10471977293491364f);
    func_002A7750(arg0, 3, 0);
    *(int *)(arg0 + 0x98) = *(int *)(*(char **)(arg0 + 0x24) + 0x10);
    *(u8 *)(state + 0x7A) = 0;
    *(u8 *)(arg0 + 0x31) = 1;
    *(u8 *)(arg0 + 0x7F) = 0;
    *(u16 *)(arg0 + 0x34) = (*(u16 *)(arg0 + 0x34) | 0x1000) & 0xFFB8;
    {
        char **slot = (char **)(state + 0x4C0);

        for (i = 1; i >= 0; i--) {
            if (*slot != 0) {
                *(u16 *)(*slot + 0x34) &= 0xFFFE;
                *(u8 *)(*slot + 0x31) = 1;
                *(int *)(*slot + 0x98) = *(int *)(*(int *)(*slot + 0x24) + 0x10);
            }
            slot++;
        }
    }
    {
        short flag = *(short *)(state + 0x60);

        *(u16 *)(state + 0x4F0) = arg1;
        *(u16 *)(state + 0x4EA) = 0;
        if (flag != 0) {
            *(short *)(state + 0x60) = 0;
            func_002A0488(arg0, *(u8 *)(state + 0x64), *(u8 *)(state + 0x65),
                          *(u8 *)(state + 0x66));
            *(short *)(state + 0x62) = 1;
        }
    }
}

extern void func_31E3C0(char *, int, int, int);

extern void func_316220(char *);

extern void func_31EA58(char *, float, float);

extern void func_32B068(char *, int, char *, float, float, float, float, int);

extern void func_2F1F70(void *, void *, void *);

extern void func_3FC1B0(int, int);

extern void func_41F230(char *, int, int, int, int, int, int, int, float, float, int, int, int, int);

extern void func_366BD8(int, int, char *, int);

/* Tears the actor down: stops its effect, clears its sub-objects, resets flags. */
void func_L02_003F18E8(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    char buffer[0x10];
    int i;

    if (*(short *)(state + 0x4EA) == 1)
        func_3FC1B0(*(int *)(arg0 + 0xB8), 0xB22);
    *(short *)(state + 0x4EA) = 0;
    *(int *)(state + 0x4B0) = 0;
    *(unsigned char *)(state + 0x7A) = 0;
    *(unsigned char *)(state + 0x2E) = 1;
    if (*(unsigned char *)(arg0 + 0x31) != 0) {
        func_41F230(arg0 + 0x10, 0, 0x1E, 5, 5, 5, 0x2D, 1, 0.5f, 0.0f, 1, 0, 0, 0);
        func_366BD8(0, 0, arg0, 0xC3E);
        func_31EA58(buffer, 0.033333335071802139f, 0.083333335816860199f);
        func_2F1F70(buffer, buffer, state + 0xE0);
        func_32B068(arg0, 0x7F, buffer, 3.0f, 6.0f, 3.0f, 6.0f, 0xFF);
    }
    {
        char **slot = (char **)(state + 0x4C0);

        for (i = 1; i >= 0; i--) {
            if (*slot != 0) {
                *(unsigned short *)(*slot + 0x34) |= 1;
                *(unsigned char *)(*slot + 0x31) = 0;
                *(int *)(*slot + 0x98) = 0;
            }
            slot++;
        }
    }
    func_31E3C0(arg0, 0, 0, 3);
    func_316220(arg0);
    func_31E6A0(arg0, 1, 0);
    *(unsigned char *)(arg0 + 0x31) = 0;
    *(int *)(arg0 + 0x98) = 0;
    *(unsigned short *)(arg0 + 0x34) =
        (*(unsigned short *)(arg0 + 0x34) & 0xEFFF) | 0x47;
}

INCLUDE_ASM("asm/levels", func_L02_003F1A88);

void func_L02_003F1B70(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);

    *(short *)(state + 0x4EC) = *(unsigned char *)(arg0 + 0x20);
    *(short *)(state + 0x4EE) = *(unsigned char *)(arg0 + 0x95);
    func_002A7750(arg0, 6, 4);
}

INCLUDE_ASM("asm/levels", func_L02_003F1BA8);

INCLUDE_ASM("asm/levels", func_L02_003F52B0);

extern char *D_L02_001B3AF8 __attribute__((sda));
extern char *D_L02_001B3AFC __attribute__((sda));
extern u8 D_L02_00220730;

/* Appends to the actor's list of 16-bit indices (at +0x40, count at +0x80, at most 0x20) the
   index of every 0x20-byte record of the table whose byte at +0x1A equals the current key. */
void func_L02_003F5390(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    s16 *tbl = (s16 *)(state + 0x40);
    u8 key = D_L02_00220730;
    char *end = D_L02_001B3AFC;
    int i = 0;
    char *p;

    for (p = D_L02_001B3AF8; p < end; p += 0x20, i++) {
        if (*(u8 *)(p + 0x1A) == key) {
            s16 *q = tbl + *(int *)(state + 0x80);

            *q = i;
            if (++*(int *)(state + 0x80) >= 0x20)
                break;
        }
    }
}
