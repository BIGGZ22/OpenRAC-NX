#include "include_asm.h"

/* Frozen Batch A A18-void-store-16; acknowledged claim 4cf4f8b91176606a7af1fd853f7ccd89fa63964199d0883686e46fb6a47255a8. */
typedef unsigned char BatchA_A18_u8;

typedef int BatchA_A18_s32;

typedef short BatchA_A18_BatchAs16;

/* Frozen Batch A A09-cond-select-36; acknowledged claim 3dadb64036892dfb357f8b3f0f7b47fc608b60613dcd8c823ef8df1bc26f7e5b. */
typedef unsigned char BatchA_A09_u8;

typedef int BatchA_A09_s32;


extern BatchA_A18_u8 D_0018C0D4;

INCLUDE_ASM("asm/levels", func_L18_002C4000);

INCLUDE_ASM("asm/levels", func_L18_002C41F0);

INCLUDE_ASM("asm/levels", func_L18_002CFD68);

INCLUDE_ASM("asm/levels", func_L18_002CFF48);

INCLUDE_ASM("asm/levels", func_L18_002D0C00);

INCLUDE_ASM("asm/levels", func_L18_002D1998);

INCLUDE_ASM("asm/levels", func_L18_002DEFF8);

extern BatchA_A18_s32 ref_L18_002E4090_00(BatchA_A18_s32 a0, BatchA_A18_s32 a1);

void func_L18_002E4090(BatchA_A18_s32 a0) {
    BatchA_A18_s32 e = a0 ? 2 : 1;
    switch (D_0018C0D4) {
    case 0:
        ref_L18_002E4090_00(0, e);
        break;
    case 4:
        ref_L18_002E4090_00(0x8D, e);
        break;
    }
}

INCLUDE_ASM("asm/levels", func_L18_002E42F8);

INCLUDE_ASM("asm/levels", func_L18_002F1098);

extern void func_0027F1D8(void *a0, void *a1, const char *a2);
extern char D_L18_00296510[];

void func_L18_00303A58(void *a0, void *a1) {
    func_0027F1D8(a0, a1, D_L18_00296510);
}

INCLUDE_ASM("asm/levels", func_L18_003044B8);

INCLUDE_ASM("asm/levels", func_L18_00307C60);

INCLUDE_ASM("asm/levels", func_L18_0032B768);

INCLUDE_ASM("asm/levels", func_L18_0033B628);

INCLUDE_ASM("asm/levels", func_L18_0035D238);

extern float func_L18_0035D5F8(BatchA_A18_s32 a0, BatchA_A18_s32 a1, BatchA_A18_s32 a2,
                               BatchA_A18_s32 a3);
extern BatchA_A18_s32 D_001FE480[];

float func_L18_0035D5A8(BatchA_A18_s32 a0, BatchA_A18_s32 a1, BatchA_A18_s32 a2,
                        BatchA_A18_s32 a3) {
    if (a3 == -1) {
        return -1.0f;
    }
    a3 = D_001FE480[a3];
    if (a3 == 0) {
        return -1.0f;
    }
    return func_L18_0035D5F8(a0, a1, a2, a3);
}

INCLUDE_ASM("asm/levels", func_L18_0035D5F8);

INCLUDE_ASM("asm/levels", func_L18_003F50D8);

INCLUDE_ASM("asm/levels", func_L18_003F5198);

INCLUDE_ASM("asm/levels", func_L18_003F53D8);

INCLUDE_ASM("asm/levels", func_L18_003F5650);

INCLUDE_ASM("asm/levels", func_L18_003F6CA8);

INCLUDE_ASM("asm/levels", func_L18_003F7198);

extern float func_00283248(float a, float b);

void func_L18_003F72D8(BatchA_A18_u8 *a0, BatchA_A18_u8 *a1) {
    BatchA_A18_s32 p = *(BatchA_A18_s32 *)(a0 + 0x68);
    *(unsigned short *)((char *)p + 0x290) = 1;
    *(float *)((char *)p + 0x264) =
        func_00283248(*(float *)(a1 + 0x0) - *(float *)(a0 + 0x10),
                      *(float *)(a1 + 0x4) - *(float *)(a0 + 0x14));
}

INCLUDE_ASM("asm/levels", func_L18_003F7320);

INCLUDE_ASM("asm/levels", func_L18_003F7358);

INCLUDE_ASM("asm/levels", func_L18_003F73B8);


INCLUDE_ASM("asm/levels", func_L18_003F7470);

INCLUDE_ASM("asm/levels", func_L18_003F74B8);

extern void func_002A7750(BatchA_A18_u8 *a0, BatchA_A18_s32 a1, BatchA_A18_s32 a2);

void func_L18_003F7590(BatchA_A18_u8 *a0) {
    BatchA_A18_s32 p = *(BatchA_A18_s32 *)(a0 + 0x68);
    *(unsigned short *)((char *)p + 0x88) = 0;
    *(BatchA_A18_s32 *)(a0 + 0x98) = 0;
    *(unsigned short *)(a0 + 0x34) =
        (unsigned short)((*(unsigned short *)(a0 + 0x34) & 0xEFFF) | 0x43);
    func_002A7750(a0, 1, 0);
    *(BatchA_A18_s32 *)(a0 + 0x98) = 0;
    *(unsigned short *)(a0 + 0x34) =
        (unsigned short)(*(unsigned short *)(a0 + 0x34) & 0xEFFF);
}

INCLUDE_ASM("asm/levels", func_L18_003F75E8);

INCLUDE_ASM("asm/levels", func_L18_003F76F0);

INCLUDE_ASM("asm/levels", func_L18_003F7818);

INCLUDE_ASM("asm/levels", func_L18_003F7988);

INCLUDE_ASM("asm/levels", func_L18_003F7A08);

extern void func_L18_003F7A08(BatchA_A18_u8 *a0);
void func_L18_003F7A88(BatchA_A18_u8 *a0) {
    if (a0[0x95] == 1) {
        func_L18_003F7A08(a0);
    }
}

INCLUDE_ASM("asm/levels", func_L18_003F7AB8);

INCLUDE_ASM("asm/levels", func_L18_003F8D28);

/* A18-void-store-16 freshly exact-verified under family reservation */
void func_L18_003F8E30(BatchA_A18_u8 *s) {
    BatchA_A18_s32 p = *(BatchA_A18_s32 *)(s + 0x68);
    *(BatchA_A18_BatchAs16 *)((char *)p + 0x768) = 1;
}

INCLUDE_ASM("asm/levels", func_L18_003F8E40);

/* A09-cond-select-36 freshly exact-verified under family reservation */
BatchA_A09_s32 func_L18_003F8E80(BatchA_A09_u8 *s) {
    BatchA_A09_s32 r = 0;
    if (s[0x20] == 6) { r = (s[0xBC] == 2); }
    return r;
}

INCLUDE_ASM("asm/levels", func_L18_003F8EA8);

INCLUDE_ASM("asm/levels", func_L18_003F8FA8);

INCLUDE_ASM("asm/levels", func_L18_003F9DA0);

INCLUDE_ASM("asm/levels", func_L18_003F9E50);

INCLUDE_ASM("asm/levels", func_L18_003F9EF0);

INCLUDE_ASM("asm/levels", func_L18_003F9FB0);

INCLUDE_ASM("asm/levels", func_L18_003FA180);

INCLUDE_ASM("asm/levels", func_L18_003FA7C0);

BatchA_A18_s32 func_L18_003FA8E0(BatchA_A18_u8 *s) {
    BatchA_A18_s32 p = *(BatchA_A18_s32 *)(s + 0x68);
    if (*(short *)((char *)p + 0xA8) > 0) {
        *(unsigned short *)((char *)p + 0xA8) =
            (unsigned short)(*(unsigned short *)((char *)p + 0xA8) - 1);
        return 1;
    }
    return 0;
}

void func_L18_003FA908(BatchA_A18_u8 *s) {
    BatchA_A18_s32 p = *(BatchA_A18_s32 *)(s + 0x68);
    *(unsigned short *)((char *)p + 0xA8) =
        (unsigned short)(*(unsigned short *)((char *)p + 0xA8) + 1);
}

INCLUDE_ASM("asm/levels", func_L18_003FA920);

INCLUDE_ASM("asm/levels", func_L18_003FAE50);

INCLUDE_ASM("asm/levels", func_L18_003FB500);

INCLUDE_ASM("asm/levels", func_L18_003FB5E8);

INCLUDE_ASM("asm/levels", func_L18_003FC958);

INCLUDE_ASM("asm/levels", func_L18_003FC9B8);

INCLUDE_ASM("asm/levels", func_L18_003FCA38);

INCLUDE_ASM("asm/levels", func_L18_003FCC48);

INCLUDE_ASM("asm/levels", func_L18_003FCD70);

INCLUDE_ASM("asm/levels", func_L18_003FD630);

INCLUDE_ASM("asm/levels", func_L18_003FD730);

/* store -1 into field 0x74 of the object at s[0x68] */
void func_L18_003FE010(BatchA_A18_u8 *s) {
    BatchA_A18_s32 p = *(BatchA_A18_s32 *)(s + 0x68);
    *(BatchA_A18_s32 *)((char *)p + 0x74) = -1;
}

INCLUDE_ASM("asm/levels", func_L18_003FE020);

INCLUDE_ASM("asm/levels", func_L18_00426AB0);

INCLUDE_ASM("asm/levels", func_L18_00426D98);

INCLUDE_ASM("asm/levels", func_L18_00426E00);

INCLUDE_ASM("asm/levels", func_L18_00426E48);

INCLUDE_ASM("asm/levels", func_L18_00426F28);

INCLUDE_ASM("asm/levels", func_L18_00426F78);

extern void func_00282948(void *a0);

void func_L18_00427010(BatchA_A18_u8 *a0) {
    BatchA_A18_s32 p = *(BatchA_A18_s32 *)(a0 + 0x68);
    if (a0[0x21] != 0xFF && *(BatchA_A18_u8 *)(p + 0x3B0) != 0) {
        func_00282948(*(void **)(p + 0x3AC));
    }
    *(BatchA_A18_u8 *)(p + 0x3B0) = 0;
}

void func_L18_00427058(BatchA_A18_u8 *a0) {
    BatchA_A18_s32 p = *(BatchA_A18_s32 *)(a0 + 0x68);
    BatchA_A18_s32 q;
    if (a0[0x21] == 0xFF) {
        return;
    }
    if (*(BatchA_A18_u8 *)(p + 0x3B0) != 0) {
        return;
    }
    q = *(BatchA_A18_s32 *)(p + 0x3AC);
    (*(BatchA_A18_s32 *)q)++;
    *(BatchA_A18_u8 *)(p + 0x3B0) = 1;
}

INCLUDE_ASM("asm/levels", func_L18_00427130);

extern BatchA_A18_s32 func_L09_0033E658(void *a0);

BatchA_A18_s32 func_L18_004271C0(BatchA_A18_u8 *a0) {
    BatchA_A18_s32 p = *(BatchA_A18_s32 *)(a0 + 0x68);
    if (*(short *)((char *)p + 0x1CA) != 0) {
        *(BatchA_A18_BatchAs16 *)((char *)p + 0x1CA) = 0;
        return 0;
    }
    *(BatchA_A18_BatchAs16 *)((char *)p + 0x1CA) =
        (BatchA_A18_BatchAs16)func_L09_0033E658(*(void **)((char *)p + 0x38C));
    return 1;
}

INCLUDE_ASM("asm/levels", func_L18_00427208);

INCLUDE_ASM("asm/levels", func_L18_004273F0);

INCLUDE_ASM("asm/levels", func_L18_00427688);

INCLUDE_ASM("asm/levels", func_L18_00427B48);
