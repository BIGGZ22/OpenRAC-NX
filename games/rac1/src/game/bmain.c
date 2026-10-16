#include "common.h"
#include "structs.h"

/*
 * bmain.cpp in the original source; text 0x1E9808-0x1E9E70.
 * Name and boundary from the NTSC split of this game, mapped to PAL by matching function
 * sizes -- see docs/DECOMP_PROGRESS.md. Compiled as C for now.
 */

extern int D_0015EE80 MACRO_ADDR;
extern int D_0015EFD8 MACRO_ADDR;
extern int D_0015F6E8 MACRO_ADDR;
extern int D_0015EF88 MACRO_ADDR;
typedef struct {
    int b;
    int a;
} LevelLoad;
typedef struct {
    char pad0[0x1A78];
    LevelLoad normal[4];
    LevelLoad alt[4];
} Globals137C80;
extern Globals137C80 D_00137C80_g __asm__("D_00137C80");
extern char D_0013E650[];
extern char D_0013D390[];
extern char D_001941C0[];
extern void func_00118D80(int);
extern void func_0022EFE8(void);
extern void func_00216D88(void);
extern int func_001F98C0(int);
extern void func_001F4E08(int);
extern void func_0012EF48(int);
extern void func_00209E68(void);
extern void func_0023B670(int, int, int, int, int);
extern void func_00120F30(int);
extern void func_00122598(int);
extern void func_00120858(int, int);
extern void func_0012F308(void);
extern void func_00123168(void (*)(void));
extern void func_00201348(int, int, int, int, int, int);

/* Start level `level`: pick its two load parameters from the level
   table in D_00137C80 (the alternate set while D_0015EE80 is set), stop
   sound and wait for the loader (func_00209E68) to go idle, then load it
   (func_0023B670) and reset the display state. */
void func_001E9808(int level) {
    int a;
    int b;
    int t;

    if (level < 0) {
        return;
    }
    if (D_0015EE80 != 0) {
        b = D_00137C80_g.alt[level].b;
        a = D_00137C80_g.alt[level].a;
    } else {
        b = D_00137C80_g.normal[level].b;
        a = D_00137C80_g.normal[level].a;
    }
    D_0015EFD8 = 2;
    D_0013E650[0x6B] |= 8;
    func_00118D80(0);
    sound_StopAllSounds();
    music_Stop();
    FadeToBlack(func_001F98C0(12));
    D_0015F6E8 = 1;
    func_00118D80(0);
    sound_StopAllSounds();
    music_Stop();
    func_0012EF48(0);
    for (;;) {
        char *ld = D_0013D390;
        if (*(int *)(ld + 0xDC) < 3 && *(int *)(ld + 0xE4) < 0) {
            break;
        }
        func_00209E68();
    }
    {
        char *g = D_001941C0;
        t = *(int *)(g + 0x1C);
    }
    func_0023B670(b, a, t + 0x100000, t + 0x400000, 0);
    func_00120F30(0);
    func_00122598(0);
    func_00120858(0, 0);
    func_00123168(func_0012F308);
    Hud_sendTexture(0x1000000, D_0015EF88, 0x1B, 6, 6, 1);
    D_0015EFD8 = 0;
    FadeToBlack(4);
    D_0015F6E8 = 0;
    D_0013E650[0x6B] |= 0x10;
}

/* Pad state, as far as this function reads it. */
struct PadState_E99D8 {
    u8 pad_0[0x1A4];
    s32 pressed;
};

extern s32 D_0015EE88_E99D8 __asm__("D_0015EE88") MACRO_ADDR;

struct DiscTable_E99D8 {
    u8 pad_0[0x14E0];
    s32 sound_bank;                        /* 0x14E0 */
};

extern struct DiscTable_E99D8 D_00137C80_E99D8 __asm__("D_00137C80");

typedef struct {
    s32 off;
    s32 size;
} LevelChunk_E99D8;

typedef struct {
    s32 gfx;
    s32 pad4;
    s32 gfx_alt;
    s32 padC;
    LevelChunk_E99D8 intro[6];
    LevelChunk_E99D8 loading[6];
    s32 code;
} LevelHeader_E99D8;

typedef struct {
    s32 pad[7];
    s32 bank;
} SoundSlot_E99D8;

extern s32 D_0015F6C8_E99D8 __asm__("D_0015F6C8") MACRO_ADDR;
extern u8 D_00161380_E99D8[] __asm__("D_00161380");
extern u8 D_00165530_E99D8[] __asm__("D_00165530");
extern s32 D_0015F538_E99D8 __asm__("D_0015F538") MACRO_ADDR;
extern s32 D_0015F6E8_E99D8 __asm__("D_0015F6E8") MACRO_ADDR;
extern s32 D_0015F05C_E99D8 __asm__("D_0015F05C") MACRO_ADDR;
extern s32 D_0015EFD8_E99D8 __asm__("D_0015EFD8") MACRO_ADDR;
extern s32 D_0015EE80_E99D8 __asm__("D_0015EE80") MACRO_ADDR;
extern s32 D_0015EE84_E99D8 __asm__("D_0015EE84") MACRO_ADDR;
extern u8 D_0024272F_E99D8[] __asm__("D_0024272F");
extern struct PadState_E99D8 D_0013CA40_E99D8 __asm__("D_0013CA40");
extern s32 D_00139478_E99D8[] __asm__("D_00139478");
extern s32 D_00139480_E99D8[] __asm__("D_00139480");
extern char D_001E79C0_E99D8[] __asm__("D_001E79C0");
extern SoundSlot_E99D8 D_00186200_E99D8[] __asm__("D_00186200");
extern SoundSlot_E99D8 D_001862E0_E99D8[] __asm__("D_001862E0");
extern SoundSlot_E99D8 *D_0015F714_E99D8 __asm__("D_0015F714") MACRO_ADDR;
extern s32 D_0015F710_E99D8 __asm__("D_0015F710") MACRO_ADDR;
extern u8 D_0016044C_E99D8[] __asm__("D_0016044C") MACRO_ADDR;
extern s32 D_0015F6E4_E99D8 __asm__("D_0015F6E4") MACRO_ADDR;
extern u8 D_0013E130_E99D8[] __asm__("D_0013E130");

extern void func_00201E88_E99D8(void) __asm__("func_00201E88");
extern void func_001E94A8_E99D8(void) __asm__("func_001E94A8");
extern void func_002348E8_E99D8(void) __asm__("func_002348E8");
extern void func_00235018_E99D8(void) __asm__("func_00235018");
extern void func_001F0F30_E99D8(void) __asm__("func_001F0F30");
extern s32 func_00122598_E99D8(s32) __asm__("func_00122598");
extern void func_001FB470_E99D8(void) __asm__("func_001FB470");
extern void func_001F3C10_E99D8(void) __asm__("func_001F3C10");
extern void func_001F3D00_E99D8(void) __asm__("func_001F3D00");
extern void func_001FB448_E99D8(s32, u64, u64) __asm__("func_001FB448");
extern void func_00118D80_E99D8(s32) __asm__("func_00118D80");
extern void func_0020C468_E99D8(s32, s32) __asm__("func_0020C468");
extern void func_001FB498_E99D8(void) __asm__("func_001FB498");
extern void func_001FB530_E99D8(void) __asm__("func_001FB530");
extern void func_00201AF0_E99D8(s32) __asm__("func_00201AF0");
extern void func_001F7680_E99D8(s32) __asm__("func_001F7680");
extern void func_00234C98_E99D8(s32, u64) __asm__("func_00234C98");
extern void func_001F55C0_E99D8(s32, s32, s32, s32) __asm__("func_001F55C0");
extern void func_001FB598_E99D8(void) __asm__("func_001FB598");
extern void func_001FB8A8_E99D8(void) __asm__("func_001FB8A8");
extern void func_002349B8_E99D8(void) __asm__("func_002349B8");
extern void func_00234948_E99D8(void) __asm__("func_00234948");
extern void func_00234AC8_E99D8(s32) __asm__("func_00234AC8");
extern s32 func_00209BB8_E99D8(void) __asm__("func_00209BB8");
extern void func_00218908_E99D8(void) __asm__("func_00218908");
extern void func_001F4E08_E99D8(s32) __asm__("func_001F4E08");
extern void func_0023B670_E99D8(s32, s32, s32, s32, s32) __asm__("func_0023B670");
extern s32 func_001F98C0_E99D8(s32) __asm__("func_001F98C0");
extern void func_001E9730_E99D8(char *, ...) __asm__("func_001E9730");
extern s32 func_0022EA20_E99D8(s32) __asm__("func_0022EA20");
extern void func_0012E2E8_E99D8(void) __asm__("func_0012E2E8");
extern s32 func_001EBB48_E99D8(void) __asm__("func_001EBB48");
extern s32 func_001207B8_E99D8(void) __asm__("func_001207B8");
extern void func_00121B78_E99D8(s16, s16, s16, s16) __asm__("func_00121B78");
extern void func_001F3890_E99D8(void) __asm__("func_001F3890");
extern void func_00233308_E99D8(void) __asm__("func_00233308");

/* Start-up sequence before the first level: clears the level bss, shows the intro and
   loading images of the chosen language (PAL fades each in over eight frames) until they end
   or a button is pressed, plays the intro movie, shows the loading image, loads the sound
   bank and files it in the two sound slot tables, applies the video mode setting and starts
   the level load.
   The slot stores are in this order on purpose: the last store through each table, the last
   one of the bank value and the two globals are what the scheduler moves to the front.
   Adapted from Lombyte (MIT) for PAL: src/textbin/world/startlevel.c, startlevel. */
void func_001E99D8(void) {
    LevelHeader_E99D8 *hdr;
    LevelChunk_E99D8 *tbl;
    s32 n;
    s32 i;
    s32 cur;
    s32 prev;
    s32 frames;
    s32 code;
    s32 bank;
    s32 fade;
    u8 *p;

    D_0015F6C8_E99D8 = 1;
    func_00201E88_E99D8();
    func_001E94A8_E99D8();
    for (i = 0; i < D_00165530_E99D8 - D_00161380_E99D8; i++) {
        D_00161380_E99D8[i] = 0;
    }
    prev = 0;
    frames = 0;
    fade = 0x80;
    func_002348E8_E99D8();
    func_00235018_E99D8();
    func_001F0F30_E99D8();
    D_0015F538_E99D8 = 0;
    func_00122598_E99D8(0);
    D_0015F6E8_E99D8 = 0;
    func_001F3C10_E99D8();
    func_001F3D00_E99D8();
    func_001FB470_E99D8();
    func_001FB448_E99D8(0, 0, 0);
    hdr = (LevelHeader_E99D8 *)(((u32)D_0024272F_E99D8 & 0xFFFFC000) + 0x2C0000);
    func_002348E8_E99D8();
    while ((cur = func_00209BB8_E99D8()) != 0 && (frames < 11 || D_0013CA40_E99D8.pressed == 0)) {
        if (cur != prev) {
            if (cur == 1) {
                tbl = hdr->intro;
            } else {
                tbl = hdr->loading;
            }
            func_00118D80_E99D8(0);
            func_0020C468_E99D8(tbl[D_0015EE88_E99D8].off + (s32)hdr, hdr->code + (s32)hdr);
            func_00118D80_E99D8(0);
        }
        func_001FB448_E99D8(0, 0, 0);
        func_001FB498_E99D8();
        func_001FB530_E99D8();
        func_001F7680_E99D8(hdr->code + (s32)hdr);
        if (frames < 8) {
            func_00234C98_E99D8(0x42, 0x8000000044);
            func_001F55C0_E99D8(0, 0, 0, fade);
        }
        func_001FB598_E99D8();
        func_001FB8A8_E99D8();
        func_002349B8_E99D8();
        func_00234948_E99D8();
        func_00234AC8_E99D8(1);
        func_00122598_E99D8(0);
        prev = cur;
        fade -= 0x10;
        func_00218908_E99D8();
        frames++;
    }
    if (prev != 0) {
        func_001F4E08_E99D8(8);
    }
    D_0015EFD8_E99D8 = -1;
    code = hdr->code + (s32)hdr;
    D_0015F05C_E99D8 = code;
    if (D_0015EE80_E99D8 == 0) {
        func_0023B670_E99D8(D_00139478_E99D8[0], D_00139478_E99D8[1], (code + 0x3F) & ~0x3F,
                        (code + 0x2C003F) & ~0x3F, 0);
    } else {
        func_0023B670_E99D8(D_00139480_E99D8[0], D_00139480_E99D8[1], (code + 0x3F) & ~0x3F,
                        (code + 0x2C003F) & ~0x3F, 0);
    }
    D_0015EFD8_E99D8 = 0;
    func_001F4E08_E99D8(func_001F98C0_E99D8(0xC));
    func_00118D80_E99D8(0);
    if (D_0015EE80_E99D8 != 0) {
        func_0020C468_E99D8(hdr->gfx_alt + (s32)hdr, hdr->code + (s32)hdr);
    } else {
        func_0020C468_E99D8(hdr->gfx + (s32)hdr, hdr->code + (s32)hdr);
    }
    func_00118D80_E99D8(0);
    func_002348E8_E99D8();
    func_001FB448_E99D8(0, 0, 0);
    func_001FB498_E99D8();
    func_001FB530_E99D8();
    func_00201AF0_E99D8(hdr->code + (s32)hdr);
    func_001FB598_E99D8();
    func_001FB8A8_E99D8();
    func_002349B8_E99D8();
    func_00234948_E99D8();
    func_00234AC8_E99D8(1);
    func_00122598_E99D8(0);
    D_0015F538_E99D8++;
    func_001E9730_E99D8(D_001E79C0_E99D8);
    bank = func_0022EA20_E99D8(D_00137C80_E99D8.sound_bank);
    func_0012E2E8_E99D8();
    D_00186200_E99D8[0].bank = bank;
    D_00186200_E99D8[1].bank = bank;
    D_00186200_E99D8[2].bank = bank;
    D_00186200_E99D8[3].bank = bank;
    D_00186200_E99D8[4].bank = bank;
    D_00186200_E99D8[5].bank = bank;
    D_001862E0_E99D8[0].bank = bank;
    D_001862E0_E99D8[1].bank = bank;
    D_001862E0_E99D8[2].bank = bank;
    D_001862E0_E99D8[3].bank = bank;
    D_001862E0_E99D8[4].bank = bank;
    D_00186200_E99D8[6].bank = bank;
    D_0015F714_E99D8 = D_00186200_E99D8;
    D_0015F710_E99D8 = 7;
    func_001EBB48_E99D8();
    if (D_0015EE80_E99D8 != D_0016044C_E99D8[0]) {
        D_0015EE80_E99D8 = D_0016044C_E99D8[0];
        func_001207B8_E99D8();
        func_00121B78_E99D8(0, 1, D_0015EE80_E99D8 != 0 ? 3 : 2, 0);
        func_001F3890_E99D8();
        func_001FB470_E99D8();
    }
    p = D_0013E130_E99D8;
    D_0015F6E4_E99D8 = D_0015EE84_E99D8;
    *(s16 *)(p + 0x2A) = 1;
    D_0015EE84_E99D8 = -1;
    func_00233308_E99D8();
}
