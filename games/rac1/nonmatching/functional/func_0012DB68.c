/* functional: high. fcheck: unchecked (every captured state was skipped, nothing compared; OpenRAC tools/fcheck_gate.py, 2026-10-18). The NTSC decomp's matched C for the same instructions (snd_start_sound_system), carried to PAL's
   names by OpenRAC's tools/port.py. Not a PAL match: PAL's compiler gives our 616 bytes for retail's 600 bytes. */
struct StartSoundWork {
    volatile s32 read_active;
    u8 pad_4[0xC];
    volatile s32 read_error;
};
extern struct StartSoundWork D_00137C00_2DB68 __asm__("D_00137C00");
struct SifClientDataStartSound {
    u8 pad_0[0x24];
    void *volatile server;
};
extern u8 D_00133380[];
extern u8 D_00134380[];
extern u8 D_00135380[];
extern u8 D_00136380[];
extern u8 D_00137380[];
extern u8 D_001377C0[];
extern u8 D_00153D50[];
extern u8 D_00153D78[];
extern struct SifClientDataStartSound D_0015ECC0_2DB68 __asm__("D_0015ECC0");
extern struct SifClientDataStartSound D_0015ECE8_2DB68 __asm__("D_0015ECE8");
extern u8 * D_0015EDA0_2DB68[2] __asm__("D_0015EDA0");
extern short D_0015EDA8;
extern s32 D_0015EDAC MACRO_ADDR;
extern u8 * D_0015EDB0_2DB68[2] __asm__("D_0015EDB0");
extern u8 * D_0015EDB8_2DB68[2] __asm__("D_0015EDB8");
extern short D_0015EDC8;
extern short D_0015EDD0;
extern s64 D_0015EDD8 MACRO_ADDR;
extern s32 D_0015EE00_2DB68 __asm__("D_0015EE00") MACRO_ADDR;
extern void func_0011AE20(u32);
extern s32 func_0011B2F8(struct SifClientDataStartSound *, u32, s32);
extern s32 func_00116078_2DB68(const char *, ...) __asm__("func_00116078");
extern s32 func_0012E688(s32, s32, void *);

/* Adapted from Lombyte (GPL-3.0) for PAL by OpenRAC's tools/port.py: src/textbin/audio/snd_start_sound_system.c, snd_start_sound_system. */
s32 func_0012DB68(void) {
    s32 bind_result;
    s32 command_arg;
    u8 *const command_buffer1 = D_00134380;
    u8 *const command_buffer0 = D_00133380;

    D_0015EDA0_2DB68[0] = command_buffer0;
    D_0015EDA0_2DB68[1] = command_buffer1;
    D_0015EDB8_2DB68[0] = D_00137380;
    D_0015EDB8_2DB68[1] = D_001377C0;
    D_0015EDB0_2DB68[0] = D_00135380;
    D_0015EDB0_2DB68[1] = D_00136380;
    func_0011AE20(0);

    for (;;) {
        bind_result = func_0011B2F8(&D_0015ECC0_2DB68, 0x123456, 0);
        if (bind_result < 0) {
            func_00116078_2DB68((const char *)D_00153D50, D_00153D78, 0x73);
            for (;;) {
            }
        }
        command_arg = 10000;
        for (command_arg--; command_arg != -1; command_arg--) {
        }
        if (D_0015ECC0_2DB68.server != 0) {
            break;
        }
    }

    (*(s32 *)&D_0015EDC8) = 0;
    (*(s32 *)&D_0015EDD0) = 0;
    D_0015EDD8 = 0;
    D_0015EE00_2DB68 = 0;

    do {
        bind_result = func_0011B2F8(&D_0015ECE8_2DB68, 0x123457, 0);
        if (bind_result < 0) {
            func_00116078_2DB68((const char *)D_00153D50, D_00153D78, 0x88);
            for (;;) {
            }
        }
        command_arg = 10000;
        for (command_arg--; command_arg != -1; command_arg--) {
        }
    } while (D_0015ECE8_2DB68.server == 0);

    *(s32 *)D_00133380 = 0;
    D_00137C00_2DB68.read_active = 0;
    *(s32 *)D_00134380 = 0;
    D_00137C00_2DB68.read_error = 0;
    (*(s32 *)&D_0015EDA8) = 0xFFC;
    D_0015EDAC = 0xFFC;
    command_arg = (s32)(u32)&D_00137C00_2DB68.read_active;
    return func_0012E688(0, 4, &command_arg);
}
