#include "include_asm.h"

extern int func_00115544(const char *, const char *);

void *func_00120390(const char *first, const char *second, void *value) {
    return func_00115544(first, second) ? 0 : value;
}

void func_001203C0(void) {
}

extern void *func_00115F28(unsigned int);
extern int func_00115484(unsigned char *, int, int);
extern void func_00120368(void);

char *func_001203C8(void) {
    char *record = func_00115F28(24);
    if (record == 0) func_00120368();
    func_00115484((unsigned char *)record, 0, 24);
    *(char **)(record + 4) = record + 16;
    return record;
}

extern char *(*D_00135D38)(void);

char *func_00120420(void) {
    return D_00135D38();
}

char *func_00120448(void) {
    return D_00135D38() + 8;
}

extern char *func_00120498(void);

char *func_00120470(void) {
    D_00135D38 = func_00120498;
    return D_00135D38();
}

extern int D_00141800;
extern char D_001417F0[];
extern char D_00141808[];

char *func_00120498(void) {
    if (D_00141800 == 0) {
        D_00141800 = 1;
        func_00115484((unsigned char *)D_001417F0, 0, 16);
        *(char **)(D_001417F0 + 4) = D_00141808;
    }
    return D_001417F0;
}

char *func_00120500(void) {
    return D_00135D38() + 4;
}

INCLUDE_ASM("asm/core", func_00120528);

int func_00120800(short *record) {
    return record[3];
}

int func_00120808(short *record) {
    return record[2];
}

INCLUDE_ASM("asm/core", func_00120810);

INCLUDE_ASM("asm/core", func_001208E8);

INCLUDE_ASM("asm/core", func_00120A30);

extern void *func_00120A30(int, void *, int);
extern void *func_001153D4(void *, const void *, unsigned int);

void *func_00120AB8(int index, void *source, void *destination) {
    void *from = func_00120A30(index, source, 0);
    void *to = func_00120A30(index, destination, 0);
    unsigned int size = index < 31 ? 16 : index <= 63 ? 4 : 8;
    return func_001153D4(to, from, size);
}

INCLUDE_ASM("asm/core", func_00120B38);

void func_00120BC8(void)
{
}

INCLUDE_ASM("asm/core", func_00120BD0);

INCLUDE_ASM("asm/core", func_00120F00);

INCLUDE_ASM("asm/core", func_001210E0);

INCLUDE_ASM("asm/core", func_001212C4);
