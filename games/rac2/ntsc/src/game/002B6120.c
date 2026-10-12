#include "include_asm.h"

#ifndef RAC2_T_T16_FD6E0D08A
#define RAC2_T_T16_FD6E0D08A
typedef struct { float x; float y; int pad[2]; } T16_Fd6e0d08a;
#endif

extern void *func_L00_0032C638_1(void *, void *);

extern void func_002B6228(void *, void *, int, int);

extern float func_00283248_9(float, float);

extern void func_002B50E0(void *, void *, float);

extern float func_00282E80_4(void *, void *);

typedef unsigned char u8;

typedef int s32;

extern u8 D_001A63A8[];

typedef struct { u8 before[0x68]; s32 active; u8 gap[6]; short state; } CallState;

extern void func_00133400(s32);

typedef unsigned int u32;

extern s32 func_00133230(void);

extern s32 func_00132028(void);

typedef struct {
 u8 before[0x2c]; short clear2c; u8 gap1[14]; short mode,next; u8 gap2[4];
 u32 busy44; short value48; u8 gap3[2]; short step4c,step4e; u8 gap4[24];
 u32 busy68; u8 gap5[4]; short step70,step72; u8 gap6[24];
 u32 busy8c; u8 gap7[4]; short step94,step96;
} WaitState;

extern s32 func_00133310(void);

extern s32 func_00133490(s32);

/* Frozen Batch A A05-store-rotation-52; acknowledged claim 43a7518b17439f6bdc9390be9d4164b47eec4bed94a72d581fdec3b883b0a554. */
typedef unsigned char BatchA_A05_u8;

typedef int BatchA_A05_s32;

typedef short BatchA_A05_BatchAs16;

typedef struct { BatchA_A05_u8 b[0x50]; BatchA_A05_BatchAs16 f50; BatchA_A05_BatchAs16 f52; BatchA_A05_u8 c[0x20]; BatchA_A05_BatchAs16 f74; BatchA_A05_BatchAs16 f76; BatchA_A05_u8 d[0x20]; BatchA_A05_BatchAs16 f98; BatchA_A05_BatchAs16 f9a; } BatchA_A05_BatchA52;

extern BatchA_A05_BatchA52 BatchA_A05_BatchA_D001A63A8;

/* Frozen Batch A A11-sda-3store-28; acknowledged claim 9b61d4cdd097ce0a56790793fdd6c129d3f0d822e40e788cde17e6418bdea65f. */
typedef unsigned char BatchA_A11_u8;

typedef unsigned short BatchA_A11_u16;

extern BatchA_A11_u8 BatchA_A11_D_001A63A8[];

/* Set the value of each enabled resident channel in its observed update order. */
typedef struct ResidentChannel64 {
    short flags;
    short value;
    u8 fields4[0x20];
} ResidentChannel64;

typedef struct ResidentChannels64 {
    u8 fields0[0x50];
    ResidentChannel64 channels[3];
} ResidentChannels64;

typedef struct __attribute__((packed)) { u8 mode[4]; } CdMode;

extern CdMode D_001A63E8;

extern u8 D_001A7900[] __attribute__((sda));

extern s32 D_001A7430[] __attribute__((sda));

extern s32 D_001A7434 __attribute__((sda));

extern s32 func_001334B8(s32,s32,s32,CdMode *);

extern void func_002B7CE0_1(int);

extern int func_002B7B18_1(char *, int, int);

extern s32 func_0011AEA0(s32);

extern s32 func_00133688(void);

/* Computes the x/y difference between one 16-byte record of the second handle and the record after the first handle's base, dispatches through three helpers and walks a short counter field up or down by one when the last helper's float result stays under a stored threshold. */
int func_002B6120(char *s1, char *s2)
{
    char *s0;
    float r;

    s0 = (char *)func_L00_0032C638_1(s1, s2);
    if (s0 == 0)
        return 0;
    if ((int)s2 != *(int *)(s0 + 196))
        func_002B6228(s1, s2, -1, 0);
    r = func_00283248_9(((T16_Fd6e0d08a *)(s2 + (*(short *)(s0 + 200) << 4)))[1].x - ((T16_Fd6e0d08a *)s1)[1].x,
                ((T16_Fd6e0d08a *)(s2 + (*(short *)(s0 + 200) << 4)))[1].y - ((T16_Fd6e0d08a *)s1)[1].y);
    func_002B50E0(s1, &((T16_Fd6e0d08a *)s2)[*(short *)(s0 + 202) + 1], r);
    if (func_00282E80_4(&((T16_Fd6e0d08a *)s1)[1], &((T16_Fd6e0d08a *)s2)[*(short *)(s0 + 200) + 1])
        <= *(float *)(s0 + 52)) {
        if (*(short *)(s0 + 202) == *(short *)(s0 + 200))
            return 1;
        if (*(short *)(s0 + 202) < *(short *)(s0 + 200))
            *(short *)(s0 + 200) = (unsigned short)*(short *)(s0 + 200) - 1;
        else
            *(short *)(s0 + 200) = (unsigned short)*(short *)(s0 + 200) + 1;
    }
    return 0;
}

INCLUDE_ASM("asm/game", func_002B6220);

INCLUDE_ASM("asm/game", func_002B6228);

INCLUDE_ASM("asm/game", func_002B62A0);

INCLUDE_ASM("asm/game", func_002B62F8);

INCLUDE_ASM("asm/game", func_002B6318);

INCLUDE_ASM("asm/game", func_002B63C0);

INCLUDE_ASM("asm/game", func_002B6480);

INCLUDE_ASM("asm/game", func_002B65B8);

INCLUDE_ASM("asm/game", func_002B6650);

INCLUDE_ASM("asm/game", func_002B6658);

extern char D_001A7248[] __attribute__((sda));
extern void func_0011D620(void *, int, int, int, int, int, int, int, int);
extern void func_00133250(int, int, int, int);

void func_002B66F0(void) {
    func_0011D620(D_001A7248, 3, 0, 0, 0, 0, 0, 0, 0);
    func_00133250(7, 0xA000, 0, 1);
    func_0011D620(D_001A7248, 3, 0, 0, 0, 0, 0, 0, 0);
}

void func_002B6770(int arg0, long arg1) {
    int *p = (int *)(int)arg1;

    if (p != 0) {
        *p = arg0;
    }
}

INCLUDE_ASM("asm/game", func_002B6788);

INCLUDE_ASM("asm/game", func_002B67E0);

INCLUDE_ASM("asm/game", func_002B6828);

INCLUDE_ASM("asm/game", func_002B68C0);

INCLUDE_ASM("asm/game", func_002B68C8);

extern void func_002B7D88(s32);
extern void func_001336D0(void (*)(s32));

void func_002B6938(void) {
    func_001336D0(func_002B7D88);
}

INCLUDE_ASM("asm/game", func_002B6958);

s32 func_002B6D28(void) {
    if (((CallState *)D_001A63A8)->active==0) return 0;
    if (((CallState *)D_001A63A8)->state!=3) return 0;
    func_00133400(((CallState *)D_001A63A8)->active); ((CallState *)D_001A63A8)->state=4;
    return 1;
}

INCLUDE_ASM("asm/game", func_002B6D80);

INCLUDE_ASM("asm/game", func_002B6E78);

INCLUDE_ASM("asm/game", func_002B6F70);

INCLUDE_ASM("asm/game", func_002B7088);

void func_002B7170(void) {
    ((void (*)(void))func_00133230)();
    while (((WaitState *)D_001A63A8)->busy44 == 0xffffffffU) func_00132028();
    while (((WaitState *)D_001A63A8)->busy8c == 0xffffffffU) func_00132028();
    while (((WaitState *)D_001A63A8)->busy68 == 0xffffffffU) func_00132028();
    func_00133310();
    while (func_00132028() != 0) {}
    func_00133490(1);
    ((WaitState *)D_001A63A8)->step4e = 0; ((WaitState *)D_001A63A8)->step4c = 0; ((WaitState *)D_001A63A8)->busy44 = 0;
    if (((WaitState *)D_001A63A8)->mode != -1) ((WaitState *)D_001A63A8)->value48 = ((WaitState *)D_001A63A8)->mode;
    ((WaitState *)D_001A63A8)->step72 = 0; ((WaitState *)D_001A63A8)->step70 = 0; ((WaitState *)D_001A63A8)->busy68 = 0;
    ((WaitState *)D_001A63A8)->step96 = 0; ((WaitState *)D_001A63A8)->step94 = 0; ((WaitState *)D_001A63A8)->busy8c = 0;
    ((WaitState *)D_001A63A8)->clear2c = 0; ((WaitState *)D_001A63A8)->mode = -1;
    ((WaitState *)D_001A63A8)->next = -1;
}

void func_002B72E8(BatchA_A05_s32 a0) {
    if (a0 != 0) { BatchA_A05_BatchA_D001A63A8.f76 = 0; BatchA_A05_BatchA_D001A63A8.f74 = -0x8000; }
    BatchA_A05_BatchA_D001A63A8.f9a = 0;
    BatchA_A05_BatchA_D001A63A8.f50 = -0x8000;
    BatchA_A05_BatchA_D001A63A8.f52 = 0;
    BatchA_A05_BatchA_D001A63A8.f98 = -0x8000;
}

void func_002B7320(void) {
    BatchA_A11_u8 *p = BatchA_A11_D_001A63A8;
    *(BatchA_A11_u16 *)(p + 0x50) = 4;
    *(BatchA_A11_u16 *)(p + 0x98) = 4;
    *(BatchA_A11_u16 *)(p + 0x74) = 4;
}

void func_002B7340(s32 value)
{
    ResidentChannels64 *state = (ResidentChannels64 *)D_001A63A8;
    if (state->channels[0].flags & 0x8000)
        state->channels[0].value = value;
    if (state->channels[2].flags & 0x8000)
        state->channels[2].value = value;
    if (state->channels[1].flags & 0x8000)
        state->channels[1].value = value;
}

INCLUDE_ASM("asm/game", func_002B7380);

INCLUDE_ASM("asm/game", func_002B7578);

extern char D_001AA1E8[];
extern void func_0026FEB8(void *);
extern int func_00133640(void);
void func_002B7AD0(void) {
    u8 *state = D_001A63A8;
    if (*(short *)(state + 4) != 0) {
        func_0026FEB8(D_001AA1E8);
        func_00133640();
        state[6] = 1;
    }
}

INCLUDE_ASM("asm/game", func_002B7B18);

INCLUDE_ASM("asm/game", func_002B7BC0);

s32 func_002B7C10(s32 a0, s32 a1, s32 a2) {
    CdMode mode = D_001A63E8;
    mode.mode[1] = D_001A7900[0];
    D_001A7430[0] = 0; D_001A7434 = 0;
    func_001334B8(a1,a2,a0,&mode);
    func_00133230(); func_00132028();
    return 1;
}

/* Calls one callee with the argument 1, then a second callee with (first, second, third argument) keeping its result, calls the first callee again with 1, and returns the kept result. */
int func_002B7C80(char *p, int x, int y)
{
    int v;

    func_002B7CE0_1(1);
    v = func_002B7B18_1(p, x, y);
    func_002B7CE0_1(1);
    return v;
}

INCLUDE_ASM("asm/game", func_002B7CE0);

void func_002B7D88(s32 a0) {
    if (a0 == 1) {
        if (func_00133688() != 0) { *(short *)(D_001A63A8+4) = 2; } else {
            void (*callback)(s32,s32);
            s32 data, flag;
            ((void (*)(s32))func_0011AEA0)(0);
            flag = D_001A63A8[6] == 0;
            callback = *(void (**)(s32,s32))(D_001A63A8+0x18);
            *(short *)(D_001A63A8+4) = 0;
            D_001A63A8[6] = 0;
            if (callback) {
                data = *(s32 *)(D_001A63A8+0x1c);
                *(void (**)(s32,s32))(D_001A63A8+0x18) = 0;
                *(s32 *)(D_001A63A8+0x1c) = 0;
                callback(data,flag);
            }
        }
    }
}

void func_002B7DF8(int arg0, long arg1) {
    short *p = (short *)(int)arg1;

    if (p != 0 && arg0 != 0 && p[5] == 2) {
        p[5] = 3;
    }
}
