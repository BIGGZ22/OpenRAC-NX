/* Functions of level 13 added after the first split; stubs added by tools/overlay_variants.py, replaced by C as functions are matched. */
#include "common.h"
#include "include_asm.h"

extern void func_0022C7E0(void);
extern int D_0015EE84_m __asm__("D_0015EE84") MACRO_ADDR;
extern void func_L00_0028BBF8(void);
extern void func_L00_0028A5A8(void);
extern void func_0022C870(void);
extern void func_00234C98(int, long);
extern int D_0015EF88 MACRO_ADDR;
void func_L13_0024A0F0(void) {
    SetupSkyGifPaging();
    if (D_0015EE84_m == 13) {
        func_L00_0028BBF8();
    } else {
        UpdateSkyShellsStatic();
    }
    DoSkyGifPaging();
    VU1_addGSregister(0x47, 0x5360B);
    VU1_addGSregister(0x4E, 0x1000000 | (D_0015EF88 >> 13));
}
