#include "include_asm.h"

/* Frozen Batch A A18-void-store-16; acknowledged claim 4cf4f8b91176606a7af1fd853f7ccd89fa63964199d0883686e46fb6a47255a8. */
typedef unsigned char BatchA_A18_u8;

typedef int BatchA_A18_s32;

typedef short BatchA_A18_BatchAs16;

/* Frozen Batch A A09-cond-select-36; acknowledged claim 3dadb64036892dfb357f8b3f0f7b47fc608b60613dcd8c823ef8df1bc26f7e5b. */
typedef unsigned char BatchA_A09_u8;

typedef int BatchA_A09_s32;

INCLUDE_ASM("asm/levels", func_L18_002C4000);

INCLUDE_ASM("asm/levels", func_L18_002C41F0);

INCLUDE_ASM("asm/levels", func_L18_002CFD68);

INCLUDE_ASM("asm/levels", func_L18_002CFF48);

INCLUDE_ASM("asm/levels", func_L18_002D0C00);

INCLUDE_ASM("asm/levels", func_L18_002D1998);

INCLUDE_ASM("asm/levels", func_L18_002DEFF8);

INCLUDE_ASM("asm/levels", func_L18_002E4090);

INCLUDE_ASM("asm/levels", func_L18_002E42F8);

INCLUDE_ASM("asm/levels", func_L18_002F1098);

INCLUDE_ASM("asm/levels", func_L18_00303A58);

INCLUDE_ASM("asm/levels", func_L18_003044B8);

INCLUDE_ASM("asm/levels", func_L18_00307C60);

INCLUDE_ASM("asm/levels", func_L18_0032B768);

INCLUDE_ASM("asm/levels", func_L18_0033B628);

INCLUDE_ASM("asm/levels", func_L18_0035D238);

INCLUDE_ASM("asm/levels", func_L18_0035D5A8);

INCLUDE_ASM("asm/levels", func_L18_0035D5F8);

INCLUDE_ASM("asm/levels", func_L18_003F50D8);

INCLUDE_ASM("asm/levels", func_L18_003F5198);

INCLUDE_ASM("asm/levels", func_L18_003F53D8);

INCLUDE_ASM("asm/levels", func_L18_003F5650);

INCLUDE_ASM("asm/levels", func_L18_003F6CA8);

INCLUDE_ASM("asm/levels", func_L18_003F7198);

INCLUDE_ASM("asm/levels", func_L18_003F72D8);

INCLUDE_ASM("asm/levels", func_L18_003F7320);

INCLUDE_ASM("asm/levels", func_L18_003F7358);

INCLUDE_ASM("asm/levels", func_L18_003F73B8);

INCLUDE_ASM("asm/levels", func_L18_003F7470);

INCLUDE_ASM("asm/levels", func_L18_003F74B8);

INCLUDE_ASM("asm/levels", func_L18_003F7590);

INCLUDE_ASM("asm/levels", func_L18_003F75E8);

INCLUDE_ASM("asm/levels", func_L18_003F76F0);

INCLUDE_ASM("asm/levels", func_L18_003F7818);

INCLUDE_ASM("asm/levels", func_L18_003F7988);

INCLUDE_ASM("asm/levels", func_L18_003F7A08);

INCLUDE_ASM("asm/levels", func_L18_003F7A88);

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

INCLUDE_ASM("asm/levels", func_L18_003FA8E0);

INCLUDE_ASM("asm/levels", func_L18_003FA908);

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

INCLUDE_ASM("asm/levels", func_L18_003FE010);

INCLUDE_ASM("asm/levels", func_L18_003FE020);

INCLUDE_ASM("asm/levels", func_L18_00426AB0);

INCLUDE_ASM("asm/levels", func_L18_00426D98);

INCLUDE_ASM("asm/levels", func_L18_00426E00);

INCLUDE_ASM("asm/levels", func_L18_00426E48);

INCLUDE_ASM("asm/levels", func_L18_00426F28);

INCLUDE_ASM("asm/levels", func_L18_00426F78);

INCLUDE_ASM("asm/levels", func_L18_00427010);

INCLUDE_ASM("asm/levels", func_L18_00427058);

INCLUDE_ASM("asm/levels", func_L18_00427130);

INCLUDE_ASM("asm/levels", func_L18_004271C0);

INCLUDE_ASM("asm/levels", func_L18_00427208);

INCLUDE_ASM("asm/levels", func_L18_004273F0);

INCLUDE_ASM("asm/levels", func_L18_00427688);

INCLUDE_ASM("asm/levels", func_L18_00427B48);
