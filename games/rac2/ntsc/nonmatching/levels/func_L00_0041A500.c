#include "include_asm.h"

extern char *D_L00_001B144C __attribute__((sda));
extern int func_001163B0(void);
extern void func_L00_0041ACA0(char *object, void *entry);
extern void func_002BE278(void *entry, void *next, int zero, float scale);

void func_L00_0041A500(char *object)
{
    char *state = *(char **)(object + 0x68);
    int index = *(int *)(state + 0x48);
    int random;
    index += 2;
    index -= *(int *)(state + 0x44);


    if (index <= -1)
        index += 0x23;

    random = func_001163B0();
    if (random % 7 == 0)
        func_L00_0041ACA0(object, D_L00_001B144C + (index << 6));

    random = func_001163B0();
    if (random % 0x16 == 0) {
        char *entry = D_L00_001B144C + (index << 6);
        func_002BE278(entry, entry + 0x10, 0, -1.0f);
    }
}
