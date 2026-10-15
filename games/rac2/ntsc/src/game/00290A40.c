#include "include_asm.h"

extern int func_002886C8(int a, char *b);

extern void func_002883E8(int a);

/* Qwen draft (branch qwen/autonomy, d969a356), checked by tools/check.py */
void func_00278EB0(void);

/* loops 56 times calling a two-argument helper and, unless the current index is zero, a second one-argument helper with that index. */
void func_00290A40(char *a0)
{
    int i;

    for (i = 0; i < 56; i++) {
        if (func_002886C8(i, a0) == 0)
            continue;
        if (i != 0)
            func_002883E8(i);
    }
}

INCLUDE_ASM("asm/game", func_00290AA8);

typedef struct { int source, length, destination, flags; } Request_290AB0;
extern int func_0011AFE0(Request_290AB0 *, int);
extern int func_0011AFC0(int);
extern int func_0011ED08(int, int, int);

int func_00290AB0(int source, int destination, int length)
{
    Request_290AB0 request;
    int handle;
    int result = 1;
    request.source = source;
    request.destination = destination;
    request.length = length;
    request.flags = 0;
    handle = func_0011AFE0(&request, 1);
    if (handle) {
        while (func_0011AFC0(handle) >= 0) {}
        if (func_0011ED08(length, 0, 0) < 0)
            result = 0;
    }
    return result;
}

INCLUDE_ASM("asm/game", func_00290B48);

INCLUDE_ASM("asm/game", func_00290C60);

void func_00291108(void)
{
  func_00278EB0();
  return;
}

extern void func_00278F80(void);

void func_00291128(void) {
    func_00278F80();
}

INCLUDE_ASM("asm/game", func_00291148);

INCLUDE_ASM("asm/game", func_00291158);

/* func_00291268 is in 00290A40-g8.c. */

INCLUDE_ASM("asm/game", func_002912E8);

void func_002912F0(void)
{
}

INCLUDE_ASM("asm/game", func_002912F8);

INCLUDE_ASM("asm/game", func_00291308);

INCLUDE_ASM("asm/game", func_00291448);

extern void func_00291788(void);
extern void func_00282A88(void *, int);
extern char D_001C2B40[];
extern char D_001C2C40[];

void func_00291450(int index) {
    if ((unsigned int)index >= 8)
        return;
    func_00291788();
    func_00282A88((void *)(D_001C2B40 + (index << 5)), 0x20);
    func_00282A88((void *)(D_001C2C40 + index * 0x30), 0x30);
}

INCLUDE_ASM("asm/game", func_002914B8);

INCLUDE_ASM("asm/game", func_00291640);
