/* Alternative source form for func_L10_00425888: the u8 spelling of the four clamps.
   Distinguished from the primary form by its 8-bit compares - `lb`/`bgez`/`sb` where
   retail and the primary form use `lw`/`slti`/`sw`.  It reaches 160 bytes against retail's
   164 (ruler-measured with scratch/wslcmp.py; check.py not run), one instruction short,
   because it never duplicates the clamp loads into the branch delay slots.
   Naming/extraction only: this form has not been recompiled or validated
   by the cleanup. See the primary near-miss file for the recorded blocker;
   do not apply another form's verdict to this one. Recheck its file-level
   flags with tools/check.py, then tools/build.py before promotion. */
typedef unsigned char u8;
typedef signed int s32;

extern u8 D_L10_001B1390 __attribute__((sda));
extern u8 D_L10_001B1394;
extern u8 D_L10_001B1398;
extern u8 D_L10_001B139C;

extern u8 D_L10_001395B8[];

void func_L10_00425888(void) {
    if (D_L10_001B1390 >= 128) D_L10_001B1390 = 127;
    if (D_L10_001B1394 >= 128) D_L10_001B1394 = 127;
    if (D_L10_001B1398 >= 128) D_L10_001B1398 = 127;
    if (D_L10_001B139C >= 128) D_L10_001B139C = 127;

    D_L10_001395B8[0x66] = D_L10_001B1390;
    D_L10_001395B8[0x67] = D_L10_001B1394;
    D_L10_001395B8[0x68] = D_L10_001B1398;
    D_L10_001395B8[0x69] = D_L10_001B139C;
}
