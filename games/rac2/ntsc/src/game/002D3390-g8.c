#include "include_asm.h"

typedef struct {
    char reserved00[0x14];
    char *source;
    int value;
    char reserved1c[0x118];
    int fallback;
} CallbackState;
typedef struct { char reserved00[0x1C4]; int flags; } MenuGameState;
typedef struct { short reserved00, key; int value; } MenuMessage;
extern MenuGameState D_00138180;
extern CallbackState D_001F2840;
extern char *D_001A8CFC[] __attribute__((sda));
extern int func_00287E70(void);
extern int func_002857B0(void);
extern void func_00287E80(void);
extern void func_00288CB0(int, int);
extern int func_0029C560(int);
extern char *func_003415B8(void *);
extern void func_002D5E98(int);
extern int D_001ABA88 __attribute__((sda));
extern int D_001ABA8C;
extern int D_001ABA90 __attribute__((sda));
extern int D_001ABA94;
extern int D_001ABA98 __attribute__((sda));
extern int D_001ABA9C;
extern float D_001B23A8;
extern void func_0027C540(long);
extern void func_0027C660(void);
extern int func_00336D50(int, int);
extern void func_00300968(char *, int, float, float, float, float);
extern char *func_00288FB8(int);
extern void func_0027FAA8(long long, long long, long long, char *, long long);
extern void func_0027F980(long long, long long, long long, char *, long long);

int func_002D4E48(void)
{
    func_0027C540(0);
    if (D_001A8CFC[0] != 0) {
        func_00300968(func_00336D50(D_001A8CFC[0] + 0x8710, 0x96), 0x60442D00,
                      (float)D_001ABA90, (float)D_001ABA94, D_001B23A8, 0.0f);
        func_00300968(func_00336D50(D_001A8CFC[0] + 0x8710, 0x97), 0x60241700,
                      (float)D_001ABA90, (float)D_001ABA94, D_001B23A8, 0.0f);
        func_00300968(func_00336D50(D_001A8CFC[0] + 0x8710, 0x98), 0x55F0C070,
                      (float)D_001ABA90, (float)D_001ABA94, D_001B23A8, 0.0f);
        func_0027FAA8(D_001ABA90 + D_001ABA88, D_001ABA94 + D_001ABA8C, 0x80F0F0F0,
                      func_00288FB8(0x2D62), -1);
        func_0027F980(D_001ABA90 + D_001ABA98, D_001ABA94 + D_001ABA9C, 0x80F0F0F0,
                      func_00288FB8(0x2D4F), -1);
    }
    func_0027C660();
    return 0;
}

int func_002D52D8(void)
{
    MenuMessage message;
    int result = 0;
    MenuGameState *state;
    int flags;
    if (func_00287E70() == 2 && func_002857B0() == 9) {
        func_00287E80();
        func_00288CB0(-1, 0);
        return 1;
    }
    func_00287E80();
    state = &D_00138180;
    flags = state->flags;
    if (flags & 0x10) {
        int value = *(int *)(D_001F2840.source + 0xE0);
        if (value != 0)
            D_001F2840.value = value;
        else if (D_001F2840.fallback == 0) {
            value = -1;
            goto return_value;
        }
        value = 0;
return_value:
        return value;
    }
    if (flags & 0x900)
        result = 1;
    else {
        func_0029C560(flags);
        if ((state->flags & 0x40) && D_001A8CFC[0] != 0) {
            char *data = func_003415B8(D_001A8CFC[0] + 0x3BE08);
            message.key = *(unsigned short *)(data + 8);
            message.value = *(int *)(data + 0xC);
            func_002D5E98((int)&message);
        }
    }
    return result;
}
