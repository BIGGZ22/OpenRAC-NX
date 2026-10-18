/* functional: high. Checked against the US assembly: all 81 handlers stored into D_L00_001B1E80 are the addresses retail stores, slot for slot. */
/* Carried from the PAL decompilation's src/overlays/shared/partproc_002697A0.c (matched C there) (games/rac1) through the version
 * map (../../README.md): functional C, not matched C. Names and addresses are this version's. */
#include "types.h"
#include "qcopy.h"
#include "qzero.h"
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
/* What the PAL file this was written in declares before it. */
extern void func_L00_00276590_2697A0() __asm__("func_L00_002756F0");
extern void func_L00_002766B0_2697A0() __asm__("func_L00_00275810");
extern void func_L00_00276700_2697A0() __asm__("func_L00_00275860");
extern void func_L00_00276940_2697A0() __asm__("func_L00_00275AA0");
extern void func_L00_00276C08_2697A0() __asm__("func_L00_00275D68");
extern void func_L00_00276D50_2697A0() __asm__("func_L00_00275EB0");
extern void func_L00_0026E5A0_2697A0() __asm__("func_L00_0026D700");
extern void func_L00_0026EA80_2697A0() __asm__("func_L00_0026DBE0");
extern void func_L00_0026ECA0_2697A0() __asm__("func_L00_0026DE00");
extern void func_L00_0026EFF0_2697A0() __asm__("func_L00_0026E150");
extern void func_L00_0026F248_2697A0() __asm__("func_L00_0026E3A8");
extern void func_L00_0026F510_2697A0() __asm__("func_L00_0026E670");
extern void func_L00_0026FA28_2697A0() __asm__("func_L00_0026EB88");
extern void func_L00_0026FCE8_2697A0() __asm__("func_L00_0026EE48");
extern void func_L00_0026FEE8_2697A0() __asm__("func_L00_0026F048");
extern void func_L00_00270120_2697A0() __asm__("func_L00_0026F280");
extern void func_L00_00270500_2697A0() __asm__("func_L00_0026F660");
extern void func_L00_00270738_2697A0() __asm__("func_L00_0026F898");
extern void func_L00_002708A0_2697A0() __asm__("func_L00_0026FA00");
extern void func_L00_002709A0_2697A0() __asm__("func_L00_0026FB00");
extern void func_L00_00270AD8_2697A0() __asm__("func_L00_0026FC38");
extern void func_L00_00270DB0_2697A0() __asm__("func_L00_0026FF10");
extern void func_L00_002715B0_2697A0() __asm__("func_L00_00270710");
extern void func_L00_002717E8_2697A0() __asm__("func_L00_00270948");
extern void func_L00_00271F40_2697A0() __asm__("func_L00_002710A0");
extern void func_L00_002722F0_2697A0() __asm__("func_L00_00271450");
extern void func_L00_002725E8_2697A0() __asm__("func_L00_00271748");
extern void func_L00_00272890_2697A0() __asm__("func_L00_002719F0");
extern void func_L00_00272A88_2697A0() __asm__("func_L00_00271BE8");
extern void func_L00_00272B28_2697A0() __asm__("func_L00_00271C88");
extern void func_L00_00272BC0_2697A0() __asm__("func_L00_00271D20");
extern void func_L00_00272C98_2697A0() __asm__("func_L00_00271DF8");
extern void func_L00_00272D68_2697A0() __asm__("func_L00_00271EC8");
extern void func_L00_00272E60_2697A0() __asm__("func_L00_00271FC0");
extern void func_L00_00273090_2697A0() __asm__("func_L00_002721F0");
extern void func_L00_002732D8_2697A0() __asm__("func_L00_00272438");
extern void func_L00_00273578_2697A0() __asm__("func_L00_002726D8");
extern void func_L00_002736D8_2697A0() __asm__("func_L00_00272838");
extern void func_L00_00273788_2697A0() __asm__("func_L00_002728E8");
extern void func_L00_00273A60_2697A0() __asm__("func_L00_00272BC0");
extern void func_L00_00273F28_2697A0() __asm__("func_L00_00273088");
extern void func_L00_00274138_2697A0() __asm__("func_L00_00273298");
extern void func_L00_002742E8_2697A0() __asm__("func_L00_00273448");
extern void func_L00_002746A0_2697A0() __asm__("func_L00_00273800");
extern void func_L00_00274750_2697A0() __asm__("func_L00_002738B0");
extern void func_L00_00274908_2697A0() __asm__("func_L00_00273A68");
extern void func_L00_00274D80_2697A0() __asm__("func_L00_00273EE0");
extern void func_L00_002751A8_2697A0() __asm__("func_L00_00274308");
extern void func_L00_002753B8_2697A0() __asm__("func_L00_00274518");
extern void func_L00_002756B8_2697A0() __asm__("func_L00_00274818");
extern void func_L00_00275910_2697A0() __asm__("func_L00_00274A70");
extern void func_L00_00275C80_2697A0() __asm__("func_L00_00274DE0");
extern void func_L00_00275CE8_2697A0() __asm__("func_L00_00274E48");
extern void func_L00_00276180_2697A0() __asm__("func_L00_002752E0");
extern void func_L00_002761C0_2697A0() __asm__("func_L00_00275320");
extern void func_L00_002763B0_2697A0() __asm__("func_L00_00275510");
extern void *D_L00_001B2200_2697A0[] __asm__("D_L00_001B1E80");
extern void func_L00_00269BE8_2697A0() __asm__("func_L00_00268D48");
extern void func_L00_00269E88_2697A0() __asm__("func_L00_00268FE8");
extern void func_L00_0026A130_2697A0() __asm__("func_L00_00269290");
extern void func_L00_0026A398_2697A0() __asm__("func_L00_002694F8");
extern void func_L00_0026A910_2697A0() __asm__("func_L00_00269A70");
extern void func_L00_0026AB10_2697A0() __asm__("func_L00_00269C70");
extern void func_L00_0026ABC0_2697A0() __asm__("func_L00_00269D20");
extern void func_L00_0026B2B8_2697A0() __asm__("func_L00_0026A418");
extern void func_L00_0026B458_2697A0() __asm__("func_L00_0026A5B8");
extern void func_L00_0026B590_2697A0() __asm__("func_L00_0026A6F0");
extern void func_L00_0026B720_2697A0() __asm__("func_L00_0026A880");
extern void func_L00_0026BB18_2697A0() __asm__("func_L00_0026AC78");
extern void func_L00_0026C3A0_2697A0() __asm__("func_L00_0026B500");
extern void func_L00_0026C7B0_2697A0() __asm__("func_L00_0026B910");
extern void func_L00_0026C908_2697A0() __asm__("func_L00_0026BA68");
extern void func_L00_0026CB48_2697A0() __asm__("func_L00_0026BCA8");
extern void func_L00_0026CF28_2697A0() __asm__("func_L00_0026C088");
extern void func_L00_0026D268_2697A0() __asm__("func_L00_0026C3C8");
extern void func_L00_0026D3F8_2697A0() __asm__("func_L00_0026C558");
extern void func_L00_0026D690_2697A0() __asm__("func_L00_0026C7F0");
extern void func_L00_0026D700_2697A0() __asm__("func_L00_0026C860");
extern void func_L00_0026DB50_2697A0() __asm__("func_L00_0026CCB0");
extern void func_L00_002689B0_2697A0() __asm__("func_L00_00267B10");
extern void func_L00_0026E050_2697A0() __asm__("func_L00_0026D1B0");
extern void func_L00_0026E310_2697A0() __asm__("func_L00_0026D470");

void FUN_L00_00268900(void) {
    D_L00_001B2200_2697A0[0] = func_L00_00269BE8_2697A0;
    D_L00_001B2200_2697A0[1] = func_L00_00269E88_2697A0;
    D_L00_001B2200_2697A0[2] = func_L00_0026A130_2697A0;
    D_L00_001B2200_2697A0[3] = func_L00_0026A398_2697A0;
    D_L00_001B2200_2697A0[4] = func_L00_0026A910_2697A0;
    D_L00_001B2200_2697A0[5] = func_L00_0026AB10_2697A0;
    D_L00_001B2200_2697A0[6] = func_L00_0026ABC0_2697A0;
    D_L00_001B2200_2697A0[7] = func_L00_0026B2B8_2697A0;
    D_L00_001B2200_2697A0[8] = func_L00_0026B458_2697A0;
    D_L00_001B2200_2697A0[9] = func_L00_0026B590_2697A0;
    D_L00_001B2200_2697A0[10] = func_L00_0026B720_2697A0;
    D_L00_001B2200_2697A0[11] = func_L00_0026BB18_2697A0;
    D_L00_001B2200_2697A0[12] = func_L00_0026C3A0_2697A0;
    D_L00_001B2200_2697A0[13] = func_L00_0026C7B0_2697A0;
    D_L00_001B2200_2697A0[14] = func_L00_0026C908_2697A0;
    D_L00_001B2200_2697A0[15] = func_L00_0026CB48_2697A0;
    D_L00_001B2200_2697A0[16] = func_L00_0026CF28_2697A0;
    D_L00_001B2200_2697A0[17] = func_L00_0026D268_2697A0;
    D_L00_001B2200_2697A0[18] = func_L00_0026D3F8_2697A0;
    D_L00_001B2200_2697A0[19] = func_L00_0026D690_2697A0;
    D_L00_001B2200_2697A0[20] = func_L00_0026D700_2697A0;
    D_L00_001B2200_2697A0[21] = func_L00_0026DB50_2697A0;
    D_L00_001B2200_2697A0[22] = func_L00_002689B0_2697A0;
    D_L00_001B2200_2697A0[23] = func_L00_0026E050_2697A0;
    D_L00_001B2200_2697A0[24] = func_L00_0026E310_2697A0;
    D_L00_001B2200_2697A0[25] = func_L00_0026E5A0_2697A0;
    D_L00_001B2200_2697A0[26] = func_L00_0026EA80_2697A0;
    D_L00_001B2200_2697A0[27] = func_L00_0026ECA0_2697A0;
    D_L00_001B2200_2697A0[28] = func_L00_0026EFF0_2697A0;
    D_L00_001B2200_2697A0[29] = func_L00_0026F248_2697A0;
    D_L00_001B2200_2697A0[30] = func_L00_0026F510_2697A0;
    D_L00_001B2200_2697A0[31] = func_L00_0026FA28_2697A0;
    D_L00_001B2200_2697A0[32] = func_L00_0026FCE8_2697A0;
    D_L00_001B2200_2697A0[33] = func_L00_0026FEE8_2697A0;
    D_L00_001B2200_2697A0[34] = func_L00_00270120_2697A0;
    D_L00_001B2200_2697A0[35] = func_L00_00270500_2697A0;
    D_L00_001B2200_2697A0[36] = func_L00_00270738_2697A0;
    D_L00_001B2200_2697A0[37] = func_L00_002708A0_2697A0;
    D_L00_001B2200_2697A0[38] = func_L00_002709A0_2697A0;
    D_L00_001B2200_2697A0[39] = func_L00_00270AD8_2697A0;
    D_L00_001B2200_2697A0[40] = func_L00_00270DB0_2697A0;
    D_L00_001B2200_2697A0[41] = func_L00_002715B0_2697A0;
    D_L00_001B2200_2697A0[42] = func_L00_002717E8_2697A0;
    D_L00_001B2200_2697A0[43] = func_L00_00271F40_2697A0;
    D_L00_001B2200_2697A0[44] = func_L00_002722F0_2697A0;
    D_L00_001B2200_2697A0[45] = func_L00_002725E8_2697A0;
    D_L00_001B2200_2697A0[46] = func_L00_00272890_2697A0;
    D_L00_001B2200_2697A0[47] = func_L00_00272A88_2697A0;
    D_L00_001B2200_2697A0[48] = func_L00_00272B28_2697A0;
    D_L00_001B2200_2697A0[49] = func_L00_00272BC0_2697A0;
    D_L00_001B2200_2697A0[50] = func_L00_00272C98_2697A0;
    D_L00_001B2200_2697A0[51] = func_L00_00272D68_2697A0;
    D_L00_001B2200_2697A0[52] = func_L00_00272E60_2697A0;
    D_L00_001B2200_2697A0[53] = func_L00_00273090_2697A0;
    D_L00_001B2200_2697A0[54] = func_L00_002732D8_2697A0;
    D_L00_001B2200_2697A0[55] = func_L00_00273578_2697A0;
    D_L00_001B2200_2697A0[56] = func_L00_002736D8_2697A0;
    D_L00_001B2200_2697A0[57] = func_L00_00273788_2697A0;
    D_L00_001B2200_2697A0[58] = func_L00_00273A60_2697A0;
    D_L00_001B2200_2697A0[59] = func_L00_00273F28_2697A0;
    D_L00_001B2200_2697A0[60] = func_L00_00274138_2697A0;
    D_L00_001B2200_2697A0[61] = func_L00_002742E8_2697A0;
    D_L00_001B2200_2697A0[62] = func_L00_002746A0_2697A0;
    D_L00_001B2200_2697A0[63] = func_L00_00274750_2697A0;
    D_L00_001B2200_2697A0[64] = func_L00_00274908_2697A0;
    D_L00_001B2200_2697A0[65] = func_L00_00274D80_2697A0;
    D_L00_001B2200_2697A0[66] = func_L00_002751A8_2697A0;
    D_L00_001B2200_2697A0[67] = func_L00_002753B8_2697A0;
    D_L00_001B2200_2697A0[68] = func_L00_002756B8_2697A0;
    D_L00_001B2200_2697A0[69] = func_L00_00275910_2697A0;
    D_L00_001B2200_2697A0[70] = func_L00_00275C80_2697A0;
    D_L00_001B2200_2697A0[71] = func_L00_00275CE8_2697A0;
    D_L00_001B2200_2697A0[72] = func_L00_00276180_2697A0;
    D_L00_001B2200_2697A0[73] = func_L00_002761C0_2697A0;
    D_L00_001B2200_2697A0[74] = func_L00_002763B0_2697A0;
    D_L00_001B2200_2697A0[75] = func_L00_00276590_2697A0;
    D_L00_001B2200_2697A0[76] = func_L00_002766B0_2697A0;
    D_L00_001B2200_2697A0[77] = func_L00_00276700_2697A0;
    D_L00_001B2200_2697A0[78] = func_L00_00276940_2697A0;
    D_L00_001B2200_2697A0[79] = func_L00_00276C08_2697A0;
    D_L00_001B2200_2697A0[80] = func_L00_00276D50_2697A0;
}
