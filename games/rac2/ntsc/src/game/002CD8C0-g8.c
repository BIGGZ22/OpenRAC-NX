#include "include_asm.h"

typedef struct { char prefix[0x14]; char *source; int value; char gap[0x118]; int fallback; } SelectionCallback;

typedef struct { char reserved[0x1C4]; int flags; } CallbackMenuState;

extern char *D_001A8CFC;

extern volatile unsigned char D_001A7A0A;

extern int ref_L00_002D1CB0_01;

extern int ref_L00_002D1CB0_02;

extern int ref_L00_002D1CB0_03 __attribute__((sda));

extern int ref_L00_002D1CB0_04;

extern int ref_L00_002D1CB0_05 __attribute__((sda));

extern int ref_L00_002D1CB0_06;

extern int ref_L00_002D1CB0_07 __attribute__((sda));

extern int ref_L00_002D1CB0_08 __attribute__((sda));

extern int D_00138344[];

extern int D_001ABA68 __attribute__((sda));

extern SelectionCallback D_001F2840;

extern int D_0025CCE8[];

extern int func_002E5E58(int, unsigned int, void *);

extern CallbackMenuState D_00138180;

extern int func_0029C560(int);

extern int func_0029C4F0(int);

extern char *func_003415B8(void *);

extern void func_002D5E98(int);

int func_002D1CB0(void)
{
    if (D_001A8CFC != 0) {
        if (D_001A7A0A != 0) {
            *(volatile int *)&ref_L00_002D1CB0_01 = 1;
            *(volatile int *)&ref_L00_002D1CB0_02 = 1;
        }
        if (ref_L00_002D1CB0_03 != 0)
            *(volatile int *)&ref_L00_002D1CB0_04 = 1;
        if (ref_L00_002D1CB0_05 != 0)
            *(volatile int *)&ref_L00_002D1CB0_06 = 1;
        if (ref_L00_002D1CB0_07 != 0)
            ref_L00_002D1CB0_08 = 1;
    }
    return 0;
}

int func_002D23F8(void)
{
    int flags = D_00138344[0];
    int result = 0;
    if (flags & 0x10) {
        int value;
        int status;
        D_001ABA68 = 0;
        value = *(int *)(D_001F2840.source + 0xE0);
        if (value != 0)
            D_001F2840.value = value;
        else if (D_001F2840.fallback == 0) {
            status = -1;
            goto finish;
        }
        status = 0;
finish:
        return status;
    }
    if (flags & 0x900) {
        D_001ABA68 = 0;
        result = 1;
    } else if (flags & 0x8000) {
        if (--D_001ABA68 >= 0)
            func_002E5E58(3, 0, 0);
        else
            func_002E5E58(5, 0, 0);
        if (D_001ABA68 < 0)
            D_001ABA68 = 0;
    } else if (flags & 0x2000) {
        if (++D_001ABA68 < 7)
            func_002E5E58(3, 0, 0);
        else
            func_002E5E58(5, 0, 0);
        if (D_001ABA68 >= 7)
            D_001ABA68 = 6;
    }
    D_0025CCE8[0] = D_001ABA68;
    return result;
}

int func_002CD950(void)
{
    struct { short reserved; unsigned short key; int value; } message;
    CallbackMenuState *state = &D_00138180;
    int flags = state->flags;
    int result = 0;
    if (flags & 0x10) {
        int value = *(int *)(D_001F2840.source + 0xE0);
        int status;
        if (value != 0)
            D_001F2840.value = value;
        else if (D_001F2840.fallback == 0) {
            status = -1;
            goto finish;
        }
        status = 0;
    finish:
        return status;
    }
    if (flags & 0x900) {
        result = 1;
    } else {
        func_0029C4F0(flags);
        if ((state->flags & 0x40) && D_001A8CFC != 0) {
            char *data = func_003415B8(D_001A8CFC + 0x3BE08);
            message.key = *(unsigned short *)(data + 8);
            message.value = *(int *)(data + 0xC);
            func_002D5E98((int)&message);
        }
    }
    return result;
}

int func_002CF1D0(void)
{
    struct { short reserved; unsigned short key; int value; } message;
    CallbackMenuState *state = &D_00138180;
    int flags = state->flags;
    int result = 0;
    if (flags & 0x10) {
        int value = *(int *)(D_001F2840.source + 0xE0);
        int status;
        if (value != 0)
            D_001F2840.value = value;
        else if (D_001F2840.fallback == 0) {
            status = -1;
            goto finish;
        }
        status = 0;
finish:
        return status;
    }
    if (flags & 0x900)
        result = 1;
    else {
        func_0029C560(flags);
        if ((state->flags & 0x40) && D_001A8CFC != 0) {
            char *data = func_003415B8(D_001A8CFC + 0x3BE08);
            message.key = *(unsigned short *)(data + 8);
            message.value = *(int *)(data + 0xC);
            if (message.value == 0)
                func_002E5E58(5, 0, 0);
            func_002D5E98((int)&message);
        }
    }
    return result;
}
