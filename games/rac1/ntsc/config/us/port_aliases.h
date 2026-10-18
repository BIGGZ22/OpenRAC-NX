/* Names the NTSC decomp's C uses for a function by another version's address, without the label
 * that gives this version's (OpenRAC's addition, OPENRAC_PATCHES.md). The native port
 * (port/game/rac1-ntsc/hostgen.json "aliases") writes each as the address name on the right before
 * it reads the sources; the decomp's own build does not read this file. */

/* create_moby, US 0x20C4F8 (src/gameplay/entities/create_moby.c, which also defines func_0020C4F8).
 * func_0020D348_m is the PAL decompilation's name, PAL's address; 45 files declare it bare, others
 * with __asm__("FUN_0020c4f8"). On NTSC-U 0x20D348 is inside
 * FUN_0020d330 (draw_moby_list): no function starts there. */
#define func_0020D348_m func_0020C4F8
