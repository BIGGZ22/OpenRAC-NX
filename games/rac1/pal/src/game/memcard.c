#include "common.h"
#include "callees.h"
#include "records.h"
#include "structs.h"

/*
 * memcard.cpp in the original source; text 0x209A60-0x20C210.
 * Name and boundary from the NTSC split of this game, mapped to PAL by matching function
 * sizes -- see docs/DECOMP_PROGRESS.md. Compiled as C for now.
 */

/* Declarations in scope here before the split. */
extern char D_0013E650[];
extern int D_0015F694;
extern void func_001F9A98(void *, void *, int);
extern char D_00189310[];
extern char D_001899D0[];
extern void *D_001871C0 NOT_SDA;
extern DispatchRec D_001E8F80[];
extern int D_0018A3B0[];
extern void func_001F99B0();
extern void func_001F2BC8(void);
extern int D_0018C434 NOT_SDA;
extern char D_001940C0[];
extern long D_00151888[3];
extern int D_0015F6FC;
extern short D_0015F534;
extern void func_001FB530(void);
extern void func_001F3D78(void);
extern int D_0015F564;
extern int D_0018DD40[];
extern int D_0018DC40[];
extern short D_0015F59C;
extern int func_001F65B0(unsigned char *arg0, int arg1, void *arg2);
extern unsigned char D_001DF3D0[];
extern unsigned char D_001DF770[];
extern unsigned char D_001DFB10[];
extern void func_001F6668(void *, void *, void *, void *, void *, int,
                          unsigned char *);
extern int func_001F6600(unsigned char *, int);
extern int func_001F6620(unsigned char *, int);
extern int func_001F4868(int);
extern void func_001FB498(void);
extern void func_001F3008(void);
extern void func_001F3140(void);
extern int D_0018E840[];
extern long D_00152178 NOT_SDA;
extern int func_001FE4D0(void);
extern char D_00199A68[];
extern short D_0015F780;
extern int D_001941CC NOT_SDA;
extern int D_0019A4E8 NOT_SDA;
extern int func_001FF668(int);
typedef struct {
    char b[0x13];
} Cfg13;
extern Cfg13 D_0019A540 NOT_SDA;
extern Cfg13 D_001E7DD8 NOT_SDA;
extern int func_00116810(void);
extern void func_001166FC(Cfg13 *, void *);
extern short D_0015F9D0;
extern void func_00201960(int, int, int, int, int);
extern void func_002023E0(int);
extern void func_002027C0(int);
extern void func_00204FC0(void *);
extern int D_0018CC20 NOT_SDA;
extern int D_001941C8 NOT_SDA;
extern int D_0016100C;
extern int D_001A0468[];
extern void func_00205830(int a, int b);
extern PadSlots D_001A01F0_slots __asm__("D_001A01F0");
extern int D_001A01F0[];
extern int *D_001602E0;
extern unsigned char D_0013D49C NOT_SDA;
extern unsigned char D_0013D49D NOT_SDA;
extern unsigned char D_0013D4A5 NOT_SDA;
extern short D_0015FE24;
extern unsigned char D_0013D4AC NOT_SDA;
extern unsigned char D_0013D4AD NOT_SDA;
extern unsigned char D_0013D4AE NOT_SDA;
extern unsigned char D_0013D4AF NOT_SDA;
extern unsigned char D_0013D4B5 NOT_SDA;
extern int D_001A04B4 NOT_SDA;
extern unsigned char D_0013D4C5 NOT_SDA;
extern int D_001414DC NOT_SDA;
extern unsigned char D_0013D4C0 NOT_SDA;
extern unsigned char D_0013D4C1 NOT_SDA;
extern unsigned char D_0013D4C2 NOT_SDA;
extern unsigned char D_0013D4D3 NOT_SDA;
extern unsigned char D_0013D4D4 NOT_SDA;
extern unsigned char D_0013D4D5 NOT_SDA;
extern unsigned char D_0013D4E0;
extern unsigned char D_0013D4DC NOT_SDA;
extern unsigned char D_0013D4DD NOT_SDA;
extern unsigned char D_0013D4DE NOT_SDA;
extern unsigned char D_0013D4DF NOT_SDA;
extern unsigned char D_0013D4E1 NOT_SDA;
extern unsigned char D_0013D4E9 NOT_SDA;
extern unsigned char D_0013D502 NOT_SDA;
extern unsigned char D_0013D503 NOT_SDA;
extern unsigned char D_0013D504 NOT_SDA;
extern unsigned char D_0013D505 NOT_SDA;
extern unsigned char D_0013D50F NOT_SDA;
extern int D_0013D668[];
extern void func_00209040(void);
extern int func_001FAA28(void *dst, int size, int a, int b);
extern void func_00208860(void *dst);
extern short D_0015EE84;
extern int D_0015EE84_far __asm__("D_0015EE84") NOT_SDA;
extern int D_001A0218[] NOT_SDA;
extern void func_00208458(void *, unsigned char *, int);
extern void func_00208688(void *, unsigned char *);
extern char D_0013D390[];
extern short D_0015EFB0;
extern int D_0015EFB4;

extern char D_0013D2D0[];
extern char D_0013D2E8[];
extern char D_0013D300[];
extern char D_0013D320[];
extern char D_0013D340[];
extern char D_0013D370[];
extern void func_00116B00(void *, void *, int);

/* Builds a 13-byte name/version stamp in D_0013D2D0 from the record's
   region byte (arg0[0x12]) and the record's date/version fields, then
   copies that stamp into the six on-disk name buffers. */
void func_00209A60(void *arg0) {
    unsigned char *a = (unsigned char *)arg0;
    unsigned char region = a[0x12];
    int i;

    if (region == 'E') {
        D_0013D2D0[2] = region;
    } else if (region == 'P') {
        D_0013D2D0[2] = 'I';
    }
    for (i = 3; i < 7; i++) {
        D_0013D2D0[i] = a[i + 0xD];
    }
    for (i = 8; i < 0xB; i++) {
        D_0013D2D0[i] = a[i + 0xD];
    }
    for (i = 0xB; i < 0xD; i++) {
        D_0013D2D0[i] = a[i + 0xE];
    }
    func_00116B00(D_0013D2E8, D_0013D2D0, 0xD);
    func_00116B00(D_0013D300, D_0013D2D0, 0xD);
    func_00116B00(D_0013D320, D_0013D2D0, 0xD);
    func_00116B00(D_0013D340, D_0013D2D0, 0xD);
    func_00116B00(D_0013D340 + 0x14, D_0013D2D0, 0xD);
    func_00116B00(D_0013D370, D_0013D2D0, 0xD);
}

extern int func_00124068(int, int, int *, int *, int *);
extern int func_00123F30(int, int *, int *);
extern void func_00122598(int);
extern int func_001241F0(int, int, char *, int, int, void *);
extern char D_0013D2D0[];
extern int D_0013D390_i[] __asm__("D_0013D390");

/* The comparison is bound to a local before the shift:
   `(a < b) << 1` folds into `a < b ? 2 : 0` (li/slt/movn), not retail's
   slti/sll. */
int func_00209BB8(void) {
    int type, free, format, cmd, result;

    result = func_00124068(D_0013D390_i[0], D_0013D390_i[1], &type, &free, &format);
    while (func_00123F30(1, &D_0013D390_i[0x30], &D_0013D390_i[0x31]) == 0) {
        func_00122598(0);
    }
    if (D_0013D390_i[0x31] == -5 || D_0013D390_i[0x31] < -9 || type != 2) {
        return 1;
    }
    if (D_0013D390_i[0x31] != -2 && format != 0) {
        result = func_001241F0(D_0013D390_i[0], D_0013D390_i[1], D_0013D2D0, 0, -1, 0);
        while (func_00123F30(1, &cmd, &result) == 0) {
            func_00122598(0);
        }
        if (result <= 0) {
            int t = free < 350;
            return t << 1;
        }
    }
    return 0;
}

extern int D_001A05C0[];
extern int D_001A08C0[];
extern int func_0020BAD8(int *p);
extern int func_0020BD70(void *src, int i, int *table);
extern int func_001E9730();
extern char D_001E8500[];

/* The counterpart of func_0020BA00 below: both descriptor sizes are
   recomputed and checked against the ones stored in the blob, then the
   blobs are read back in the same order they were written. The read
   cursor is the parameter, which retail keeps in $s2. */
/* memcard_RestoreGame */
void func_00209CE8(int arg0) {
    char *p = (char *)arg0;
    int a;
    int b;
    int i;

    a = memcard_GetDataSize(gGameSaveData);
    b = memcard_GetDataSize(gLevelSaveData);
    if (*(int *)p != a || *(int *)(p + 4) != b) {
        STUB_printf(D_001E8500);
        return;
    }
    p += 8;
    memcard_RestoreData(p, 0, gGameSaveData);
    p += a;
    for (i = 0; i < 0x14; i++) {
        memcard_RestoreData(p, i, gLevelSaveData);
        p += b;
    }
}

/*
 * 3/168: the only residual is the allocator holding the loaded pointer
 * in $2 where retail uses $3. Everything else, including saving the
 * three globals across func_00209CE8 and the $at store of D_0015EE84,
 * is instruction-for-instruction retail.
 */
extern int D_00137C80[];
extern void func_001FDF10(int, void *, void *);
extern void func_00217748(int);
extern void func_002176C8(int, int, int);
extern void func_00209CE8(int);
extern int D_0015EEE8 MACRO_ADDR;
extern int D_0015EEEC MACRO_ADDR;
extern int D_0015EEF0 MACRO_ADDR;
extern int D_0015EE84_m __asm__("D_0015EE84") MACRO_ADDR;

extern int func_002176C8_i(int, int, int) __asm__("func_002176C8");

/* func_002176C8 returns int and takes three arguments (stream.c). */
void func_00209DC0(void) {
    int a;
    int b;
    int s0;
    int s1;
    int s2;

    func_001FDF10(D_00137C80[5] << 11, &a, &b);
    func_00217748(1);
    func_002176C8_i(a, D_00137C80[4], D_00137C80[5]);
    s2 = D_0015EEF0;
    s1 = D_0015EEEC;
    s0 = gStereo;
    memcard_RestoreGame(a + *(int *)(a + 0x10));
    D_0015EEF0 = s2;
    D_0015EEEC = s1;
    gStereo = s0;
    D_0015EE84_m = 0;
}

/* Adapted from Lombyte (MIT): src/storage/memory_card/memcard_update_state.c; PAL directory validation reconstructed from assembly. */
typedef struct { u8 pad0[16]; u32 size; u8 pad14[12]; char name[32]; } McDirEntry_209E68;
extern McDirEntry_209E68 mc_directory[] __asm__("D_001A0980");
extern char D_0013D315[], D_0013D335[], D_0013D355[], D_0015FF58[], D_0015FF60[];
extern void mc_memset(void *, s32, s32) __asm__("func_001153FC");
extern s32 mc_strcmp(char *, char *) __asm__("func_001165B8");
extern s32 mc_strncmp(char *, char *, s32) __asm__("func_00116948");
typedef struct {
    s32 port;
    s32 slot;
    s32 type;
    s32 free;
    s32 format;
    s32 x14;
    s32 x18;
    s32 x1C;
    u8 pad_20[0x8C];
    s32 used_blocks; /* +0xAC */
    s32 request; /* +0xB0 */
    s32 xAC; /* +0xB4 */
    s32 xB0;
    s32 xB4;
} McCard_209E68;

typedef struct {
    McCard_209E68 card[1];
    s32 cmd;
    s32 result;
    s32 xC0;
    s32 cur;
    s32 xC8;
    s32 busy;
    s32 fd;
    s32 state;
    s32 sub;
    s32 xDC;
    s32 xE0;
    s32 err;
    s32 errCard;
    void *buf;
    s32 size;
} McState_209E68;

typedef struct {
    s32 x0;
    s32 x4;
    s32 x8;
    s32 xC;
    s32 x10;
} McChunk_209E68;

typedef struct {
    u8 pad_0[0x10];
    s32 x10;
    s32 x14;
} McLevel_209E68;

extern McState_209E68 mc_data_0013D390 __asm__("D_0013D390");
extern McLevel_209E68 mc_data_00137C80 __asm__("D_00137C80");
extern s32 mc_data_00137C94[] __asm__("D_00137C94");
extern char mc_data_0013D2D0[] __asm__("D_0013D2D0");
extern char mc_data_0013D2E8[] __asm__("D_0013D2E8");
extern char mc_data_0013D300[] __asm__("D_0013D300");
extern char mc_data_0013D320[] __asm__("D_0013D320");
extern char mc_data_0013D340[] __asm__("D_0013D340");
extern char mc_data_0013D370[] __asm__("D_0013D370");
extern u8 mc_data_0014EFD0[] __asm__("D_0014EFD0");
extern u8 mc_data_001507D0[] __asm__("D_001507D0");
extern s16 mc_data_001517D8[] __asm__("D_001517D8");
extern s32 mc_data_0015EE84 __asm__("D_0015EE84") MACRO_ADDR;
extern s32 mc_data_0015EF90 __asm__("D_0015EF90") MACRO_ADDR;
extern char mc_data_0015FF68[] __asm__("D_0015FF68");
extern char mc_data_0015FF70[] __asm__("D_0015FF70");
extern u32 mc_data_001A05C0[] __asm__("D_001A05C0");
extern u32 mc_data_001A08C0[] __asm__("D_001A08C0");
extern u8 mc_data_001A0980[] __asm__("D_001A0980");
extern char mc_data_001A09A0[] __asm__("D_001A09A0");
extern char mc_data_001E8518[] __asm__("D_001E8518");

extern void mc_call_001E9730(char *fmt, ...) __asm__("func_001E9730");
extern s32 mc_call_0020BAD8(const u32 *packet) __asm__("func_0020BAD8");
extern void mc_call_001F9978() __asm__("func_001F9978");
extern s32 mc_call_00124340(s32 port, s32 slot) __asm__("func_00124340");
extern s32 mc_call_00124528(s32 port, s32 slot) __asm__("func_00124528");
extern void mc_call_001FDF10(s32, McChunk_209E68 **, s32 *) __asm__("func_001FDF10");
extern void mc_call_0020BCB0(u8 *, s32, s32) __asm__("func_0020BCB0");
extern s32 mc_call_0020BD70(u8 *, s32, u32 *) __asm__("func_0020BD70");
extern s32 mc_call_00217628(McChunk_209E68 *, s32, s32) __asm__("func_00217628");
extern s32 mc_call_00123A10(s32 fd) __asm__("func_00123A10");
extern s32 mc_call_00124410(s32 port, s32 slot, char *name) __asm__("func_00124410");
extern s32 mc_call_001241F0(s32, s32, char *, s32, s32, void *) __asm__("func_001241F0");
extern s32 mc_call_00124068(s32, s32, s32 *, s32 *, s32 *) __asm__("func_00124068");
extern s32 mc_call_001239D8(s32 port, s32 slot, char *name) __asm__("func_001239D8");
extern s32 mc_call_001238B0(s32 port, s32 slot, char *name, s32 mode) __asm__("func_001238B0");
extern s32 mc_call_00123C30(s32 fd, void *buf, s32 size) __asm__("func_00123C30");
extern s32 mc_call_00123AC8(s32 fd, s32 offset, s32 origin) __asm__("func_00123AC8");
extern s32 mc_call_00123F30(s32 mode, s32 *cmd, s32 *result) __asm__("func_00123F30");
extern s32 mc_call_00123D48(s32 fd, void *buf, s32 size) __asm__("func_00123D48");
extern s32 mc_call_00116248(char *str, const char *format, ...) __asm__("func_00116248");
extern char *mc_call_001166FC(char *, const char *) __asm__("func_001166FC");

void func_00209E68(void) {
    union {
        char name[0x40];
        struct {
            s32 seen[5];
            char name[0x40] __attribute__((aligned(16)));
        } directory;
    } workspace;
    McState_209E68 *finished_card;

    if (mc_data_0013D390.busy) {
        mc_data_0013D390.busy = mc_call_00123F30(1, &mc_data_0013D390.cmd, &mc_data_0013D390.result) == 0;
        return;
    }
    mc_data_0013D390.busy = 1;

    switch (mc_data_0013D390.state) {
    case 0:
        if (mc_data_0013D390.cur < 0) {
            mc_data_0013D390.cur = 0;
        }
        if (mc_call_00124068(mc_data_0013D390.card[mc_data_0013D390.cur].port, mc_data_0013D390.card[mc_data_0013D390.cur].slot, &mc_data_0013D390.card[mc_data_0013D390.cur].type, &mc_data_0013D390.card[mc_data_0013D390.cur].free, &mc_data_0013D390.card[mc_data_0013D390.cur].format) == 0) {
            mc_data_0013D390.state = 1;
        }
        break;

    case 1:
        if (mc_data_0013D390.result != 0) {
            mc_data_0013D390.card[mc_data_0013D390.cur].x14 = -4;
            mc_data_0013D390.card[mc_data_0013D390.cur].x1C = mc_data_0013D390.result;
            mc_data_0013D390.card[mc_data_0013D390.cur].xAC = -1;
            mc_data_0013D390.card[mc_data_0013D390.cur].used_blocks = 0;
            if (mc_data_0013D390.card[mc_data_0013D390.cur].request == 1) {
                if (mc_data_0013D390.result == -1) mc_data_0013D390.card[mc_data_0013D390.cur].request = 2;
                else mc_data_0013D390.card[mc_data_0013D390.cur].request = 0;
            }
        } else {
            if (mc_data_0013D390.card[mc_data_0013D390.cur].request == 1) mc_data_0013D390.card[mc_data_0013D390.cur].request = 0;
        }
        if (++mc_data_0013D390.cur < 1) {
            mc_data_0013D390.state = 0;
        } else {
            mc_data_0013D390.state = 2;
        }
        mc_data_0013D390.busy = 0;
        break;

    case 2:
        if (mc_data_0013D390.xDC >= 0 && mc_data_0013D390.card[0].request != 3) {
            if (mc_data_0013D390.card[0].x1C == 0) {
                if (mc_data_0013D390.xE0 >= 0) {
                    mc_data_0013D390.cur = mc_data_0013D390.xE0;
                    mc_data_0013D390.state = mc_data_0013D390.xDC;
                    mc_data_0013D390.err = 0;
                } else {
                    mc_data_0013D390.err = 0x271A;
                }
            } else if (mc_data_0013D390.xDC == 7) {
                mc_data_0013D390.card[0].request = 0;
            }
            mc_data_0013D390.xDC = -1;
            mc_data_0013D390.xE0 = -1;
        } else {
            mc_data_0013D390.cur++;
            if (mc_data_0013D390.card[0].request == 3) mc_data_0013D390.card[0].request = 1;
            if (mc_data_0013D390.cur >= 11 || mc_data_0013D390.card[0].request == 1) {
                mc_data_0013D390.cur = 0;
                mc_data_0013D390.state = 0;
            }
        }
        mc_data_0013D390.busy = 0;
        break;

    case 3:
        if (mc_data_0013D390.card[mc_data_0013D390.cur].format == 0) {
            if (mc_call_00124340(mc_data_0013D390.card[mc_data_0013D390.cur].port, mc_data_0013D390.card[mc_data_0013D390.cur].slot) == 0) {
                mc_data_0013D390.state = 4;
            }
            break;
        }
        mc_data_0013D390.busy = 0;
        mc_data_0013D390.state = 2;
        break;

    case 4:
        if (mc_data_0013D390.result != 0) {
            mc_data_0013D390.err = 1;
            mc_data_0013D390.errCard = mc_data_0013D390.cur;
        }
        mc_data_0013D390.state = 0;
        mc_data_0013D390.cur = 0;
        mc_data_0013D390.busy = 0;
        break;

    case 5:
        if (mc_data_0013D390.card[mc_data_0013D390.cur].format == 0) {
            if (mc_call_00124528(mc_data_0013D390.card[mc_data_0013D390.cur].port, mc_data_0013D390.card[mc_data_0013D390.cur].slot) == 0) {
                mc_data_0013D390.state = 6;
            }
            break;
        }
        mc_data_0013D390.busy = 0;
        mc_data_0013D390.state = 2;
        break;

    case 6:
        if (mc_data_0013D390.result != 0) {
            mc_data_0013D390.err = 2;
            mc_data_0013D390.errCard = mc_data_0013D390.cur;
        }
        mc_data_0013D390.state = 0;
        mc_data_0013D390.cur = 0;
        mc_data_0013D390.busy = 0;
        break;

    case 7:
        if (mc_call_001241F0(mc_data_0013D390.card[mc_data_0013D390.cur].port, mc_data_0013D390.card[mc_data_0013D390.cur].slot, mc_data_0013D2E8, 0, 11, mc_data_001A0980) == 0) {
            mc_data_0013D390.sub = 0;
            mc_data_0013D390.state = 8;
        }
        break;

    case 8:
        switch (mc_data_0013D390.sub) {
        case 0:
            if (mc_data_0013D390.result > 0) {
                s32 icon0;
                s32 icon1;
                s32 icon2;
                s32 i;
                s32 blocks;
                icon0 = 0;
                mc_memset(workspace.directory.seen, 0, 20);
                icon1 = 0;
                icon2 = 0;
                blocks = 0;
                for (i = 0; i < mc_data_0013D390.result; i++) {
                    McDirEntry_209E68 *entry = &mc_directory[i];
                    char *filename = (char *)((s32)mc_directory + i * 64 + 32);
                    s32 slot;
                    blocks += (entry->size + 1023U) >> 10;
                    if (mc_strcmp(filename, D_0013D315) == 0) icon0 = 1;
                    if (mc_strcmp(filename, D_0013D335) == 0) icon1 = 1;
                    if (mc_strcmp(filename, D_0013D355) == 0) icon2 = 1;
                    if (mc_strncmp(filename, D_0015FF58, 4) == 0 &&
                        mc_strncmp(filename + 5, D_0015FF60, 4) == 0) {
                        slot = (u8)entry->name[4] - '0';
                        if (slot < 0) continue;
                        if (slot < 5) workspace.directory.seen[slot] = 1;
                    }
                }
                blocks += (mc_data_0013D390.result + 2) / 2;
                if ((u32)(blocks - 350) < 2 && mc_data_0013D390.result == 10 && icon0 && icon1 && icon2 &&
                    workspace.directory.seen[0] && workspace.directory.seen[1] && workspace.directory.seen[2] && workspace.directory.seen[3] && workspace.directory.seen[4]) {
                    mc_data_0013D390.card[mc_data_0013D390.cur].used_blocks = 0;
                    mc_data_0013D390.card[mc_data_0013D390.cur].x14 = -1;
                    mc_data_0013D390.sub = 1;
                    mc_data_0013D390.busy = 0;
                } else {
                    mc_data_0013D390.card[mc_data_0013D390.cur].used_blocks = blocks;
                    mc_data_0013D390.card[mc_data_0013D390.cur].x14 = -2;
                    goto reset_request;
                }
            } else {
                mc_data_0013D390.card[mc_data_0013D390.cur].x14 = -2;
                if (mc_data_0013D390.result == -2) {
                    mc_data_0013D390.err = 3;
                    mc_data_0013D390.errCard = mc_data_0013D390.cur;
                } else if (mc_data_0013D390.result >= 0) {
                    mc_data_0013D390.card[mc_data_0013D390.cur].x14 = -2;
                } else if (mc_data_0013D390.result != -4) {
                    mc_data_0013D390.err = 4;
                    mc_data_0013D390.errCard = mc_data_0013D390.cur;
                }
                mc_data_0013D390.state = 0;
                mc_data_0013D390.cur = 0;
                mc_data_0013D390.busy = 0;
                if (mc_data_0013D390.card[0].request == 2) mc_data_0013D390.card[0].request = 0;
            }
            break;
        case 1:
            mc_call_00116248(workspace.directory.name, mc_data_0013D370, 0);
            if (mc_call_001238B0(mc_data_0013D390.card[mc_data_0013D390.cur].port, mc_data_0013D390.card[mc_data_0013D390.cur].slot, workspace.directory.name, 1) == 0) {
                mc_data_0013D390.sub++;
            }
            break;
        case 2:
            if (mc_data_0013D390.result >= 0) {
                mc_data_0013D390.fd = mc_data_0013D390.result;
                mc_data_0013D390.sub = 3;
            } else {
                mc_data_0013D390.card[mc_data_0013D390.cur].x14 = -2;
                mc_data_0013D390.errCard = mc_data_0013D390.cur;
                if (mc_data_0013D390.result == -7) {
                    mc_data_0013D390.err = 0x2710;
                } else if (mc_data_0013D390.result == -5) {
                    mc_data_0013D390.err = 0x2711;
                } else if (mc_data_0013D390.result == -4) {
                    mc_data_0013D390.err = 0x2712;
                } else if (mc_data_0013D390.result == -3) {
                    mc_data_0013D390.err = 0x2713;
                } else if (mc_data_0013D390.result == -2) {
                    mc_data_0013D390.err = 3;
                } else {
                    mc_data_0013D390.err = 4;
                }
                mc_data_0013D390.state = 0;
                mc_data_0013D390.cur = 0;
                if (mc_data_0013D390.card[0].request == 2) mc_data_0013D390.card[0].request = 0;
            }
            mc_data_0013D390.busy = 0;
            break;
        case 3:
            mc_data_0013D390.size = 8;
            if (mc_call_00123C30(mc_data_0013D390.fd, &mc_data_0013D390.card[mc_data_0013D390.cur].xB0, 8) == 0) {
                mc_data_0013D390.sub = 4;
            }
            break;
        case 4:
            if (mc_data_0013D390.result == mc_data_0013D390.size) {
                mc_data_0013D390.card[mc_data_0013D390.cur].xAC = 0;
                if (mc_data_0013D390.card[mc_data_0013D390.cur].xB0 != mc_call_0020BAD8(mc_data_001A05C0)) {
                    mc_data_0013D390.card[mc_data_0013D390.cur].xAC++;
                }
                if (mc_data_0013D390.card[mc_data_0013D390.cur].xB4 != mc_call_0020BAD8(mc_data_001A08C0)) {
                    mc_data_0013D390.card[mc_data_0013D390.cur].xAC++;
                }
                mc_data_0013D390.busy = 0;
                mc_data_0013D390.sub = 5;
            } else {
                mc_data_0013D390.card[mc_data_0013D390.cur].x14 = -2;
                mc_data_0013D390.errCard = mc_data_0013D390.cur;
                if (mc_data_0013D390.result >= 0) {
                    mc_data_0013D390.err = 0x2716;
                    if (mc_call_00123A10(mc_data_0013D390.fd) == 0) {
                        mc_data_0013D390.state = 0;
                        mc_data_0013D390.cur = 0;
                        mc_data_0013D390.busy = 0;
                        if (mc_data_0013D390.card[0].request == 2) mc_data_0013D390.card[0].request = 0;
                    }
                    break;
                }
                if (mc_data_0013D390.result == -5) {
                    mc_data_0013D390.err = 0x2711;
                } else if (mc_data_0013D390.result == -4) {
                    mc_data_0013D390.err = 0x2714;
                } else if (mc_data_0013D390.result == -3) {
                    mc_data_0013D390.err = 0x2715;
                } else if (mc_data_0013D390.result == -2) {
                    mc_data_0013D390.err = 3;
                } else {
                    mc_data_0013D390.err = 4;
                }
                mc_data_0013D390.state = 0;
                mc_data_0013D390.cur = 0;
                mc_data_0013D390.busy = 0;
                if (mc_data_0013D390.card[0].request == 2) mc_data_0013D390.card[0].request = 0;
            }
            break;
        case 5:
            if (mc_call_00123A10(mc_data_0013D390.fd) == 0) {
                mc_data_0013D390.busy = 0;
                mc_data_0013D390.state = 21;
            }
            break;
        }
        break;

    case 9:
        mc_data_0013D390.state = 10;
        mc_data_0013D390.sub = 0;
        mc_data_0013D390.card[mc_data_0013D390.cur].x14 = 0;
    case 10:
        switch (mc_data_0013D390.sub) {
        case 0:
            if (mc_data_0013D390.card[mc_data_0013D390.cur].free + mc_data_0013D390.card[mc_data_0013D390.cur].used_blocks >= 350) {
                mc_data_0013D390.sub = 1;
            } else {
                mc_data_0013D390.err = 7;
                mc_data_0013D390.errCard = mc_data_0013D390.cur;
                mc_data_0013D390.state = 0;
                mc_data_0013D390.cur = 0;
            }
            mc_data_0013D390.busy = 0;
            break;
        case 1:
            if (mc_call_001239D8(mc_data_0013D390.card[mc_data_0013D390.cur].port, mc_data_0013D390.card[mc_data_0013D390.cur].slot, mc_data_0013D2D0) == 0) {
                mc_data_0013D390.sub = 2;
            }
            break;
        case 2:
            if (mc_data_0013D390.result == 0 || mc_data_0013D390.result == -4) {
                mc_data_0013D390.sub = 3;
                break;
            }
            mc_data_0013D390.errCard = mc_data_0013D390.cur;
            if (mc_data_0013D390.result == -3) {
                mc_data_0013D390.err = 7;
            } else if (mc_data_0013D390.result == -2) {
                mc_data_0013D390.err = 6;
            } else {
                mc_data_0013D390.err = 0xD;
            }
            mc_data_0013D390.state = 0;
            mc_data_0013D390.cur = 0;
            mc_data_0013D390.busy = 0;
            break;
        case 3: {
            McChunk_209E68 *chunk;
            s32 size;

            mc_call_001FDF10(mc_data_00137C80.x14 << 11, &chunk, &size);
            mc_call_00217628(chunk, mc_data_00137C80.x10, mc_data_00137C80.x14);
            mc_data_0013D390.sub = 4;
            mc_data_0013D390.busy = 0;
            break;
        }
        case 4:
            if (mc_data_001517D8[0] == 0) {
                mc_data_0013D390.sub = 5;
            }
            mc_data_0013D390.busy = 0;
            break;
        case 5:
        case 9:
        case 13:
        case 18:
            switch (mc_data_0013D390.sub) {
            case 5:
                mc_call_001166FC(workspace.name, mc_data_0013D300);
                break;
            case 9:
                mc_call_001166FC(workspace.name, mc_data_0013D320);
                break;
            case 18:
                mc_call_001166FC(workspace.name, mc_data_0013D340);
                break;
            case 13:
                mc_call_00116248(workspace.name, mc_data_0013D370, mc_data_0013D390.card[mc_data_0013D390.cur].x14);
                break;
            }
            if (mc_call_001238B0(mc_data_0013D390.card[mc_data_0013D390.cur].port, mc_data_0013D390.card[mc_data_0013D390.cur].slot, workspace.name, 0x203) == 0) {
                mc_data_0013D390.fd = -1;
                mc_data_0013D390.sub++;
            }
            break;
        case 6:
        case 10:
        case 14:
        case 19: {
            McChunk_209E68 *chunk;
            s32 size;

            if (mc_data_0013D390.result >= 0) {
                if (mc_data_0013D390.fd < 0) {
                    mc_data_0013D390.fd = mc_data_0013D390.result;
                }
                mc_call_001FDF10(mc_data_00137C94[0] << 11, &chunk, &size);
                switch (mc_data_0013D390.sub) {
                case 6:
                    mc_data_0013D390.size = 0x3C4;
                    mc_data_0013D390.buf = (u8 *)chunk + chunk->x0;
                    break;
                case 10:
                    mc_data_0013D390.size = chunk->xC;
                    mc_data_0013D390.buf = (u8 *)chunk + chunk->x8;
                    break;
                case 19:
                    if (mc_data_0015EF90 != 0) {
                        mc_data_0013D390.size = 0x3C00;
                    } else {
                        mc_data_0013D390.size = 0x3C04;
                    }
                    mc_data_0013D390.buf = &mc_data_0015EE84;
                    break;
                case 14: {
                    s32 n = mc_call_0020BAD8(mc_data_001A05C0);
                    mc_data_0013D390.size = n + mc_call_0020BAD8(mc_data_001A08C0) * 20 + 8;
                    mc_data_0013D390.buf = (u8 *)chunk + chunk->x10;
                    break;
                }
                }
                if (mc_call_00123D48(mc_data_0013D390.fd, mc_data_0013D390.buf, mc_data_0013D390.size) == 0) {
                    mc_data_0013D390.sub++;
                }
            } else {
                mc_data_0013D390.err = 0xA;
                mc_data_0013D390.errCard = mc_data_0013D390.cur;
                mc_data_0013D390.state = 0;
                mc_data_0013D390.cur = 0;
                mc_data_0013D390.busy = 0;
            }
            break;
        }
        case 7:
        case 11:
        case 15:
        case 20:
            if (mc_data_0013D390.result == mc_data_0013D390.size) {
                if (mc_call_00123A10(mc_data_0013D390.fd) == 0) {
                    mc_data_0013D390.sub++;
                }
                break;
            }
            mc_data_0013D390.errCard = mc_data_0013D390.cur;
            if (mc_data_0013D390.result >= 0) {
                finished_card = &mc_data_0013D390;
                finished_card->err = 0xB;
                if (mc_call_00123A10(finished_card->fd) == 0) {
                    finished_card->busy = 0;
                    goto reset_state;
                }
                break;
            }
            if (mc_data_0013D390.result == -4) {
                mc_data_0013D390.err = 8;
            } else if (mc_data_0013D390.result == -3) {
                mc_data_0013D390.err = 7;
            } else if (mc_data_0013D390.result == -2) {
                mc_data_0013D390.err = 6;
            } else {
                mc_data_0013D390.err = 0xD;
            }
            mc_data_0013D390.state = 0;
            mc_data_0013D390.cur = 0;
            mc_data_0013D390.busy = 0;
            break;
        case 8:
        case 12:
        case 16:
            if (mc_data_0013D390.result == 0) {
                mc_data_0013D390.sub++;
            } else {
                mc_data_0013D390.err = 0xC;
                mc_data_0013D390.errCard = mc_data_0013D390.cur;
                mc_data_0013D390.state = 0;
                mc_data_0013D390.cur = 0;
            }
            mc_data_0013D390.busy = 0;
            break;
        case 17:
            if (++mc_data_0013D390.card[mc_data_0013D390.cur].x14 < 5) {
                mc_data_0013D390.sub = 13;
            } else {
                mc_data_0013D390.sub++;
            }
            mc_data_0013D390.busy = 0;
            break;
        case 21:
            mc_data_0013D390.card[0].x14 = -1;
            mc_data_0013D390.state = 0;
            mc_data_0013D390.cur = 0;
            mc_data_0013D390.card[0].xAC = 0;
            break;
        }
        break;

    case 11:
        mc_data_0013D390.state = 12;
        mc_data_0013D390.sub = 0;
        mc_data_0013D390.card[mc_data_0013D390.cur].x14 = -2;
    case 12:
        switch (mc_data_0013D390.sub) {
        case 0:
            if (mc_call_001241F0(mc_data_0013D390.card[mc_data_0013D390.cur].port, mc_data_0013D390.card[mc_data_0013D390.cur].slot, mc_data_0013D2E8, 0, 1, mc_data_001A0980) == 0) {
                mc_data_0013D390.sub = 1;
            }
            break;
        case 1:
            if (mc_data_0013D390.result == 1) {
                mc_call_00116248(workspace.name, mc_data_0015FF68, mc_data_0013D2D0, mc_data_001A09A0);
                mc_call_001E9730(mc_data_0015FF70, workspace.name);
                if (mc_call_00124410(mc_data_0013D390.card[mc_data_0013D390.cur].port, mc_data_0013D390.card[mc_data_0013D390.cur].slot, workspace.name) == 0) {
                    mc_data_0013D390.sub = 2;
                }
            } else {
                if (mc_data_0013D390.result == 0) {
                    mc_data_0013D390.sub = 3;
                } else {
                    mc_data_0013D390.errCard = mc_data_0013D390.cur;
                    if (mc_data_0013D390.result == -5) {
                        mc_data_0013D390.err = 0x10;
                    } else if (mc_data_0013D390.result == -4) {
                        mc_data_0013D390.err = 0xF;
                    } else if (mc_data_0013D390.result == -2) {
                        mc_data_0013D390.err = 0xE;
                    } else {
                        mc_data_0013D390.err = 0x12;
                    }
                    mc_data_0013D390.state = 0;
                    mc_data_0013D390.cur = 0;
                }
                mc_data_0013D390.busy = 0;
            }
            break;
        case 2:
            if (mc_call_001241F0(mc_data_0013D390.card[mc_data_0013D390.cur].port, mc_data_0013D390.card[mc_data_0013D390.cur].slot, mc_data_0013D2E8, 1, 1, mc_data_001A0980) == 0) {
                mc_data_0013D390.sub = 1;
            }
            break;
        case 3:
            if (mc_call_00124410(mc_data_0013D390.card[mc_data_0013D390.cur].port, mc_data_0013D390.card[mc_data_0013D390.cur].slot, mc_data_0013D2D0) == 0) {
                mc_call_001E9730(mc_data_001E8518, mc_data_0013D2D0);
                mc_data_0013D390.sub = 4;
            }
            break;
        case 4:
            if (mc_data_0013D390.result != 0) {
                mc_data_0013D390.errCard = mc_data_0013D390.cur;
                if (mc_data_0013D390.result == -6) {
                    mc_data_0013D390.err = 0x11;
                } else if (mc_data_0013D390.result == -5) {
                    mc_data_0013D390.err = 0x10;
                } else if (mc_data_0013D390.result == -4) {
                    mc_data_0013D390.err = 0xF;
                } else if (mc_data_0013D390.result == -2) {
                    mc_data_0013D390.err = 0xE;
                } else {
                    mc_data_0013D390.err = 0x12;
                }
            }
            mc_data_0013D390.state = 0;
            mc_data_0013D390.cur = 0;
            mc_data_0013D390.busy = 0;
            break;
        }
        break;

    case 21:
        mc_data_0013D390.state = 22;
        mc_data_0013D390.card[mc_data_0013D390.cur].x18 = -1;
        mc_data_0013D390.xC8 = 0;
        break;

    case 22:
        mc_data_0013D390.card[mc_data_0013D390.cur].x18++;
        if (mc_data_0013D390.card[mc_data_0013D390.cur].x18 < 5) {
            mc_data_0013D390.sub = 0;
            mc_data_0013D390.state = 23;
        } else {
            mc_data_0013D390.state = 0;
            mc_data_0013D390.cur = 0;
            mc_data_0013D390.busy = 0;
            mc_data_0013D390.card[0].request = 0;
        }
        break;
    case 13:
        if (mc_data_0013D390.card[mc_data_0013D390.cur].x14 < 0) {
            mc_data_0013D390.err = 0x13;
            mc_data_0013D390.state = 0;
            mc_data_0013D390.cur = 0;
            mc_data_0013D390.busy = 0;
            if (mc_data_0013D390.card[0].request == 2) mc_data_0013D390.card[0].request = 0;
            break;
        }
        mc_data_0013D390.xC8 = 0;
        mc_data_0013D390.sub = 0;
        mc_data_0013D390.state = 14;
    case 14:
    case 23:
        switch (mc_data_0013D390.sub) {
        case 0:
            if (mc_data_0013D390.state == 23) {
                mc_call_00116248(workspace.name, mc_data_0013D370, mc_data_0013D390.card[mc_data_0013D390.cur].x18);
            } else {
                mc_call_00116248(workspace.name, mc_data_0013D370, mc_data_0013D390.card[mc_data_0013D390.cur].x14);
            }
            if (mc_call_001238B0(mc_data_0013D390.card[mc_data_0013D390.cur].port, mc_data_0013D390.card[mc_data_0013D390.cur].slot, workspace.name, 1) == 0) {
                mc_data_0013D390.sub++;
            }
            break;
        case 1:
            if (mc_data_0013D390.result >= 0) {
                mc_data_0013D390.fd = mc_data_0013D390.result;
                mc_data_0013D390.sub = 2;
            } else {
                mc_data_0013D390.errCard = mc_data_0013D390.cur;
                if (mc_data_0013D390.result == -7) {
                    mc_data_0013D390.err = 0x14;
                } else if (mc_data_0013D390.result == -5) {
                    mc_data_0013D390.err = 0x15;
                } else if (mc_data_0013D390.result == -4) {
                    mc_data_0013D390.err = 0x16;
                } else if (mc_data_0013D390.result == -3) {
                    mc_data_0013D390.err = 0x17;
                } else if (mc_data_0013D390.result == -2) {
                    mc_data_0013D390.err = 0x18;
                } else {
                    mc_data_0013D390.err = 0x1C;
                }
                mc_data_0013D390.state = 0;
                mc_data_0013D390.cur = 0;
                if (mc_data_0013D390.card[0].request == 2) mc_data_0013D390.card[0].request = 0;
            }
            mc_data_0013D390.busy = 0;
            break;
        case 2:
            mc_data_0013D390.size = 8;
            if (mc_call_00123C30(mc_data_0013D390.fd, &mc_data_0013D390.card[mc_data_0013D390.cur].xB0, 8) == 0) {
                mc_data_0013D390.sub = 3;
            }
            break;
        case 3:
            if (mc_data_0013D390.result == mc_data_0013D390.size) {
                mc_data_0013D390.busy = 0;
                mc_data_0013D390.sub = 4;
                break;
            }
            mc_data_0013D390.errCard = mc_data_0013D390.cur;
            if (mc_data_0013D390.result >= 0) {
                mc_data_0013D390.err = 0x1B;
                if (mc_call_00123A10(mc_data_0013D390.fd) == 0) {
                    mc_data_0013D390.state = 0;
                    mc_data_0013D390.cur = 0;
                    mc_data_0013D390.busy = 0;
                    if (mc_data_0013D390.card[0].request == 2) mc_data_0013D390.card[0].request = 0;
                }
                break;
            }
            if (mc_data_0013D390.result == -5) {
                mc_data_0013D390.err = 0x15;
            } else if (mc_data_0013D390.result == -4) {
                mc_data_0013D390.err = 0x19;
            } else if (mc_data_0013D390.result == -3) {
                mc_data_0013D390.err = 0x1A;
            } else if (mc_data_0013D390.result == -2) {
                mc_data_0013D390.err = 0x18;
            } else {
                mc_data_0013D390.err = 0x1C;
            }
            mc_data_0013D390.state = 0;
            mc_data_0013D390.cur = 0;
            mc_data_0013D390.busy = 0;
            if (mc_data_0013D390.card[0].request == 2) mc_data_0013D390.card[0].request = 0;
            break;
        case 4:
            mc_data_0013D390.size = mc_data_0013D390.card[mc_data_0013D390.cur].xB0;
            if (mc_data_0013D390.size > 0x1800) {
                mc_call_001F9978();
            }
            if (mc_call_00123C30(mc_data_0013D390.fd, mc_data_0014EFD0, mc_data_0013D390.size) == 0) {
                mc_data_0013D390.sub = 5;
            }
            break;
        case 5:
            if (mc_data_0013D390.result == mc_data_0013D390.size) {
                if (mc_data_0013D390.state == 23) {
                    mc_call_0020BCB0(mc_data_0014EFD0, mc_data_0013D390.cur, mc_data_0013D390.card[mc_data_0013D390.cur].x18);
                    mc_data_0013D390.sub = 8;
                } else {
                    mc_data_0013D390.card[mc_data_0013D390.cur].xAC = mc_call_0020BD70(mc_data_0014EFD0, 0, mc_data_001A05C0);
                    mc_data_0013D390.sub = 6;
                }
                mc_data_0013D390.busy = 0;
                break;
            }
            mc_data_0013D390.errCard = mc_data_0013D390.cur;
            if (mc_data_0013D390.result >= 0) {
                mc_data_0013D390.err = 0x1B;
                if (mc_call_00123A10(mc_data_0013D390.fd) == 0) {
                    mc_data_0013D390.state = 0;
                    mc_data_0013D390.cur = 0;
                    mc_data_0013D390.busy = 0;
                    if (mc_data_0013D390.card[0].request == 2) mc_data_0013D390.card[0].request = 0;
                }
                break;
            }
            if (mc_data_0013D390.result == -5) {
                mc_data_0013D390.err = 0x15;
            } else if (mc_data_0013D390.result == -4) {
                mc_data_0013D390.err = 0x19;
            } else if (mc_data_0013D390.result == -3) {
                mc_data_0013D390.err = 0x1A;
            } else if (mc_data_0013D390.result == -2) {
                mc_data_0013D390.err = 0x18;
            } else {
                mc_data_0013D390.err = 0x1C;
            }
            mc_data_0013D390.state = 0;
            mc_data_0013D390.cur = 0;
            mc_data_0013D390.busy = 0;
            if (mc_data_0013D390.card[0].request == 2) mc_data_0013D390.card[0].request = 0;
            break;
        case 6:
            mc_data_0013D390.size = mc_data_0013D390.card[mc_data_0013D390.cur].xB4;
            if (mc_data_0013D390.size > 0x1000) {
                mc_call_001F9978();
            }
            if (mc_call_00123C30(mc_data_0013D390.fd, mc_data_001507D0, mc_data_0013D390.size) == 0) {
                mc_data_0013D390.sub = 7;
            }
            break;
        case 7:
            if (mc_data_0013D390.result == mc_data_0013D390.size) {
                mc_data_0013D390.card[mc_data_0013D390.cur].xAC += mc_call_0020BD70(mc_data_001507D0, mc_data_0013D390.xC8, mc_data_001A08C0);
                if (++mc_data_0013D390.xC8 < 20) {
                    mc_data_0013D390.sub = 6;
                } else {
                    mc_data_0013D390.sub = 8;
                }
                mc_data_0013D390.busy = 0;
                break;
            }
            mc_data_0013D390.errCard = mc_data_0013D390.cur;
            if (mc_data_0013D390.result >= 0) {
                mc_data_0013D390.err = 0x1B;
                if (mc_call_00123A10(mc_data_0013D390.fd) == 0) {
                    mc_data_0013D390.state = 0;
                    mc_data_0013D390.cur = 0;
                    mc_data_0013D390.busy = 0;
                    if (mc_data_0013D390.card[0].request == 2) mc_data_0013D390.card[0].request = 0;
                }
                break;
            }
            if (mc_data_0013D390.result == -5) {
                mc_data_0013D390.err = 0x15;
            } else if (mc_data_0013D390.result == -4) {
                mc_data_0013D390.err = 0x19;
            } else if (mc_data_0013D390.result == -3) {
                mc_data_0013D390.err = 0x1A;
            } else if (mc_data_0013D390.result == -2) {
                mc_data_0013D390.err = 0x18;
            } else {
                mc_data_0013D390.err = 0x1C;
            }
        reset_request:
            mc_data_0013D390.state = 0;
            mc_data_0013D390.cur = 0;
            mc_data_0013D390.busy = 0;
            if (mc_data_0013D390.card[0].request == 2) mc_data_0013D390.card[0].request = 0;
            break;
        case 8:
            if (mc_call_00123A10(mc_data_0013D390.fd) == 0) {
                if (mc_data_0013D390.state == 23) {
                    mc_data_0013D390.busy = 0;
                    mc_data_0013D390.state = 22;
                } else {
                    finished_card = &mc_data_0013D390;
                    finished_card->card[0].request = 0;
                    goto reset_state;
                }
            }
            break;
        }
        break;

    case 15:
        if (mc_data_0013D390.card[mc_data_0013D390.cur].xAC != 0) {
            mc_data_0013D390.busy = 0;
            mc_data_0013D390.state = 0;
            mc_data_0013D390.cur = 0;
            mc_data_0013D390.err = 0x2717;
            break;
        }
        if (mc_data_0013D390.card[mc_data_0013D390.cur].x14 < 0) {
            mc_data_0013D390.busy = 0;
            mc_data_0013D390.state = 0;
            mc_data_0013D390.cur = 0;
            mc_data_0013D390.err = 0x1D;
            break;
        }
        mc_data_0013D390.sub = 0;
        mc_data_0013D390.state = 16;
    case 16:
        switch (mc_data_0013D390.sub) {
        case 0:
            mc_call_00116248(workspace.name, mc_data_0013D370, mc_data_0013D390.card[mc_data_0013D390.cur].x14);
            if (mc_call_001238B0(mc_data_0013D390.card[mc_data_0013D390.cur].port, mc_data_0013D390.card[mc_data_0013D390.cur].slot, workspace.name, 2) == 0) {
                mc_data_0013D390.sub = 1;
            }
            break;
        case 1:
            if (mc_data_0013D390.result >= 0) {
                mc_data_0013D390.fd = mc_data_0013D390.result;
                mc_data_0013D390.sub = 2;
            } else {
                mc_data_0013D390.errCard = mc_data_0013D390.cur;
                if (mc_data_0013D390.result == -7) {
                    mc_data_0013D390.err = 0x1E;
                } else if (mc_data_0013D390.result == -5) {
                    mc_data_0013D390.err = 0x1F;
                } else if (mc_data_0013D390.result == -4) {
                    mc_data_0013D390.err = 0x20;
                } else if (mc_data_0013D390.result == -3) {
                    mc_data_0013D390.err = 0x21;
                } else if (mc_data_0013D390.result == -2) {
                    mc_data_0013D390.err = 0x22;
                } else {
                    mc_data_0013D390.err = 0x26;
                }
                mc_data_0013D390.state = 0;
                mc_data_0013D390.cur = 0;
            }
            mc_data_0013D390.busy = 0;
            break;
        case 2:
            if (mc_call_00123AC8(mc_data_0013D390.fd, 8, 0) == 0) {
                mc_data_0013D390.sub = 3;
            }
            break;
        case 3:
            if (mc_data_0013D390.result >= 0) {
                mc_data_0013D390.sub = 4;
            } else {
                mc_data_0013D390.state = 0;
                mc_data_0013D390.cur = 0;
                mc_data_0013D390.busy = 0;
            }
            break;
        case 4:
            if (mc_call_00123D48(mc_data_0013D390.fd, mc_data_0014EFD0, mc_call_0020BAD8(mc_data_001A05C0)) == 0) {
                mc_data_0013D390.sub = 5;
            }
            break;
        case 5:
            if (mc_data_0013D390.result == mc_call_0020BAD8(mc_data_001A05C0)) {
                mc_data_0013D390.sub = 6;
                break;
            }
            mc_data_0013D390.errCard = mc_data_0013D390.cur;
            if (mc_data_0013D390.result >= 0) {
                finished_card = &mc_data_0013D390;
                finished_card->err = 0x25;
                if (mc_call_00123A10(finished_card->fd) == 0) {
                    finished_card->busy = 0;
                    goto reset_state;
                }
                break;
            }
            if (mc_data_0013D390.result == -5) {
                mc_data_0013D390.err = 0x1F;
            } else if (mc_data_0013D390.result == -4) {
                mc_data_0013D390.err = 0x23;
            } else if (mc_data_0013D390.result == -3) {
                mc_data_0013D390.err = 0x24;
            } else if (mc_data_0013D390.result == -2) {
                mc_data_0013D390.err = 0x22;
            } else {
                mc_data_0013D390.err = 0x26;
            }
            mc_data_0013D390.state = 0;
            mc_data_0013D390.cur = 0;
            mc_data_0013D390.busy = 0;
            break;
        case 6:
            if (mc_data_0013D390.xC8 == 0) {
                mc_data_0013D390.sub = 8;
                break;
            }
            if (mc_call_00123AC8(mc_data_0013D390.fd, mc_data_0013D390.xC8 * mc_call_0020BAD8(mc_data_001A08C0), 1) == 0) {
                mc_data_0013D390.sub = 7;
            }
            break;
        case 7:
            if (mc_data_0013D390.result >= 0) {
                mc_data_0013D390.sub = 8;
            } else {
                mc_data_0013D390.state = 0;
                mc_data_0013D390.cur = 0;
                mc_data_0013D390.busy = 0;
            }
            break;
        case 8:
            if (mc_call_00123D48(mc_data_0013D390.fd, mc_data_001507D0, mc_call_0020BAD8(mc_data_001A08C0)) == 0) {
                mc_data_0013D390.sub = 9;
            }
            break;
        case 9:
            if (mc_data_0013D390.result == mc_call_0020BAD8(mc_data_001A08C0)) {
                mc_data_0013D390.sub = 10;
                break;
            }
            mc_data_0013D390.errCard = mc_data_0013D390.cur;
            if (mc_data_0013D390.result >= 0) {
                finished_card = &mc_data_0013D390;
                finished_card->err = 0x25;
                if (mc_call_00123A10(finished_card->fd) == 0) {
                    finished_card->busy = 0;
                reset_state:
                    finished_card->cur = 0;
                    finished_card->state = 0;
                }
                break;
            }
            if (mc_data_0013D390.result == -5) {
                mc_data_0013D390.err = 0x1F;
            } else if (mc_data_0013D390.result == -4) {
                mc_data_0013D390.err = 0x23;
            } else if (mc_data_0013D390.result == -3) {
                mc_data_0013D390.err = 0x24;
            } else if (mc_data_0013D390.result == -2) {
                mc_data_0013D390.err = 0x18;
            } else {
                mc_data_0013D390.err = 0x1C;
            }
            mc_data_0013D390.state = 0;
            mc_data_0013D390.cur = 0;
            mc_data_0013D390.busy = 0;
            break;
        case 10:
            finished_card = &mc_data_0013D390;
            if (mc_call_00123A10(finished_card->fd) == 0) {
                finished_card->state = 0;
                finished_card->cur = 0;
            }
            break;
        }
        break;

    case 17: {
        McChunk_209E68 *chunk;
        s32 size;

        mc_call_001FDF10(mc_data_00137C80.x14 << 11, &chunk, &size);
        mc_call_00217628(chunk, mc_data_00137C80.x10, mc_data_00137C80.x14);
        mc_data_0013D390.sub = 18;
        mc_data_0013D390.busy = 0;
        break;
    }

    case 18:
        if (mc_data_001517D8[0] == 0) {
            McChunk_209E68 *chunk;
            s32 size;

            mc_call_001FDF10(mc_data_00137C94[0] << 11, &chunk, &size);
            mc_data_0013D390.sub = 19;
            mc_data_0013D390.buf = (u8 *)chunk + chunk->x10;
        }
        mc_data_0013D390.busy = 0;
        break;

    case 19:
        if (mc_data_0013D390.card[mc_data_0013D390.cur].xAC != 0) {
            mc_data_0013D390.busy = 0;
            mc_data_0013D390.state = 0;
            mc_data_0013D390.cur = 0;
            mc_data_0013D390.err = 0x2718;
            break;
        }
        if (mc_data_0013D390.card[mc_data_0013D390.cur].x14 < 0) {
            mc_data_0013D390.busy = 0;
            mc_data_0013D390.state = 0;
            mc_data_0013D390.cur = 0;
            mc_data_0013D390.err = 0x27;
            break;
        }
        mc_data_0013D390.xC8 = 0;
        mc_data_0013D390.sub = 0;
        mc_data_0013D390.state = 20;
    case 20:
        switch (mc_data_0013D390.sub) {
        case 0:
            mc_call_00116248(workspace.name, mc_data_0013D370, mc_data_0013D390.card[mc_data_0013D390.cur].x14);
            if (mc_call_001238B0(mc_data_0013D390.card[mc_data_0013D390.cur].port, mc_data_0013D390.card[mc_data_0013D390.cur].slot, workspace.name, 2) == 0) {
                mc_data_0013D390.sub = 1;
            }
            break;
        case 1:
            if (mc_data_0013D390.result >= 0) {
                s32 n;

                mc_data_0013D390.fd = mc_data_0013D390.result;
                n = mc_call_0020BAD8(mc_data_001A05C0);
                mc_data_0013D390.size = n + mc_call_0020BAD8(mc_data_001A08C0) * 20 + 8;
                if (mc_call_00123D48(mc_data_0013D390.fd, mc_data_0013D390.buf, mc_data_0013D390.size) == 0) {
                    mc_data_0013D390.sub = 2;
                }
                break;
            }
            mc_data_0013D390.err = 0x2B;
            mc_data_0013D390.errCard = mc_data_0013D390.cur;
            mc_data_0013D390.state = 0;
            mc_data_0013D390.cur = 0;
            mc_data_0013D390.busy = 0;
            break;
        case 2:
            if (mc_data_0013D390.result == mc_data_0013D390.size) {
                if (mc_call_00123A10(mc_data_0013D390.fd) == 0) {
                    mc_data_0013D390.sub = 3;
                }
                break;
            }
            mc_data_0013D390.errCard = mc_data_0013D390.cur;
            if (mc_data_0013D390.result >= 0) {
                mc_data_0013D390.err = 0xB;
                if (mc_call_00123A10(mc_data_0013D390.fd) == 0) {
                    mc_data_0013D390.state = 0;
                    mc_data_0013D390.cur = 0;
                    mc_data_0013D390.busy = 0;
                }
                break;
            }
            if (mc_data_0013D390.result == -4) {
                mc_data_0013D390.err = 0x28;
            } else if (mc_data_0013D390.result == -3) {
                mc_data_0013D390.err = 0x29;
            } else if (mc_data_0013D390.result == -2) {
                mc_data_0013D390.err = 0x2A;
            } else {
                mc_data_0013D390.err = 0x2D;
            }
            mc_data_0013D390.state = 0;
            mc_data_0013D390.cur = 0;
            mc_data_0013D390.busy = 0;
            break;
        case 3:
            if (mc_data_0013D390.result != 0) {
                mc_data_0013D390.err = 0x2C;
                mc_data_0013D390.errCard = mc_data_0013D390.cur;
            }
            mc_data_0013D390.state = 0;
            mc_data_0013D390.cur = 0;
            mc_data_0013D390.busy = 0;
            break;
        }
        break;
    }
}

extern int D_001A05C0[];
extern int D_001A08C0[];
extern int func_0020BAD8(int *p);
extern int func_0020BBC8(void *dst, int i, int *table);

/* Serialise both descriptor tables into the caller's buffer: the two
   sizes first, then one blob for D_001A05C0 and twenty for D_001A08C0,
   each call returning how far to advance. The write cursor is the
   parameter itself -- retail keeps both in $s2. */
/* memcard_MakeWholeSave */
void func_0020BA00(char *out) {
    int i;

    *(int *)out = memcard_GetDataSize(gGameSaveData);
    *(int *)(out + 4) = memcard_GetDataSize(gLevelSaveData);
    out += 8;
    out += memcard_PrepData(out, 0, gGameSaveData);
    for (i = 0; i < 0x14; i++) {
        out += memcard_PrepData(out, i, gLevelSaveData);
    }
}

extern int func_001236F0(void);
/* Unprototyped deliberately: this is the varargs function, so callers
   legitimately pass different argument counts/types, and two agents
   declared it incompatibly. Both callers are byte-exact and must not
   be edited; an unprototyped declaration satisfies all call sites
   with identical codegen. (Only varargs *definitions* need stdarg.h;
   calling one is fine.) */
extern int func_001E9730();
extern char D_001E8690[];

/* memcard_Init */
void func_0020BAA8(void) {
    if (func_001236F0()) {
        STUB_printf(D_001E8690);
    }
}

/* memcard_GetDataSize */
int func_0020BAD8(int *p) {
    int n = 8;
    while (p[0] != 0) {
        n += 8;
        n += p[1];
        p += 4;
        n = (n + 3) & ~3;
    }
    return n + 8;
}

/* memcard_Checksum: 0 for more than 0x1800 bytes, else a 16-bit
   shift-register checksum (polynomial 0x1F45). Only the low half of
   the 0xEDB88320 seed (the CRC-32 polynomial) reaches the result. */
int func_0020BB10(void *data, int len) {
    unsigned char *p = data;
    unsigned char *end;
    int crc;
    int j;

    if (len > 0x1800) {
        return 0;
    }
    end = p + len;
    crc = 0xEDB88320;
    while (p < end) {
        crc ^= *p++ << 8;
        for (j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1F45;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc & 0xFFFF;
}

/* memcard_TestChecksum. `len` is read before the test: retail's load sits
   in the beqz slot, and a load that may trap is never taken from the
   fall-through. */
int func_0020BB88(char *buf) {
    int *p = (int *)buf;
    int len = p[0];
    int sum = p[1];
    int result = 0;
    if (sum != 0) result = memcard_Checksum(p + 2, len) == sum;
    return result;
}

extern void func_001F9A00(void *dst, void *src, int len);
extern int func_0020BB10(void *data, int len);

/* Walks a descriptor table (four words per entry: base, element size,
   tag, unused) and writes one { tag, size, data } record per entry into
   the caller's buffer, four-byte aligned, terminated by { -1, 0 }. The
   header it leaves at the front is { payload length, checksum }, and the
   return value is the total including that header. */
/*
 * Byte mismatch, correct size (0x E8), 39 of 232 bytes: the allocator puts
 * the table cursor in $s1 and the write cursor in $s0, where retail has
 * them the other way round, and it initialises the cursor before the
 * `table[0] != 0` test instead of after it. Everything else is retail's
 * instruction sequence.
 *
 * Spellings tried, all identical output: a separate `int *t = table` local
 * versus advancing the parameter itself; the source address computed at
 * the call versus in a local. That local IS load-bearing for the rest --
 * computing it before the two header stores is what makes the compiler
 * reload table[1] for them, as retail does (1807 -> 517 differing words).
 * The remaining residual is allocator destination choice, the recorded
 * dead end.
 */
struct SaveBlock {
    unsigned char *base;
    int size;
    int id;
    int pad;
};

struct SaveHeader {
    int size;
    int checksum;
};

/* memcard_PrepData: pack the save blocks (id, size, data, 4-aligned) after
   an 8-byte header, end with an id -1 record, checksum it. Adapted from
   Lombyte (MIT) for PAL. */
int func_0020BBC8(void *dst, int slot, int *table) {
    struct SaveHeader *out = dst;
    struct SaveBlock *blk = table;
    unsigned char *p;
    int total;
    unsigned char *src;

    total = 0;
    p = (unsigned char *)(out + 1);
    if (blk->base != 0) {
        struct SaveBlock *block = blk;
        int mask = -4;

        do {
            src = block->base + slot * block->size;
            total += 8;
            ((int *)p)[0] = block->id;
            ((int *)p)[1] = block->size;
            p += 8;
            func_001F9A00(p, src, block->size);
            p += block->size;
            total += block->size;
            p = (unsigned char *)(((int)p + 3) & mask);
            total = (total + 3) & mask;
            block++;
        } while (block->base != 0);
    }
    total += 8;
    ((int *)p)[1] = 0;
    ((int *)p)[0] = -1;
    out->checksum = memcard_Checksum(out + 1, total);
    out->size = total;
    return total + 8;
}

typedef struct {
    int a;        /* 0x00 */
    int b;        /* 0x04 */
    int c;        /* 0x08 */
    int d;        /* 0x0C */
    char name[8]; /* 0x10 */
    int valid;    /* 0x18 */
} McEntry;        /* 0x1C */
typedef struct {
    char hdr[0x20];
    McEntry e[5];
    char pad[0x14];
} McSlot;         /* 0xC0 */
extern McSlot D_0013D390_s[] __asm__("D_0013D390");
extern int func_0020BB88(char *); /* memcard_TestChecksum */

/* memcard_RestoreInfo(char *, int, int). Advancing the buf parameter
   itself and copying the name with memcpy both matter for retail's
   registers; re-indexing the entry per store keeps its daddu copies. */
void func_0020BCB0(char *buf, int slot, int idx) {
    D_0013D390_s[slot].e[idx].valid = memcard_TestChecksum(buf) == 0;
    buf += 0x10;
    D_0013D390_s[slot].e[idx].a = *(int *)buf;
    buf += 0xC;
    D_0013D390_s[slot].e[idx].b = *(int *)buf;
    buf += 0xC;
    D_0013D390_s[slot].e[idx].c = *(int *)buf;
    buf += 0xC;
    D_0013D390_s[slot].e[idx].d = *(int *)buf;
    buf += 0xC;
    memcpy(D_0013D390_s[slot].e[idx].name, buf, 8);
}

struct RestoreEntry {
    unsigned char *data;
    int size;
    int id;
    int status;
};
struct RestoreBlock {
    int id;
    int size;
    unsigned char data[1];
};
typedef struct {
    char pad0[0xB4];
    int errors;           /* 0xB4 */
    char padB8[8];
} RestoreCardSlot;
typedef struct {
    RestoreCardSlot slot[1];
    char padC0[0xC];
    int cur;              /* 0xCC */
} RestoreCardState;
extern RestoreCardState D_0013D390_c __asm__("D_0013D390");
extern short D_0015FF50;               /* SDA, gp -0x6DB0: mismatch count */
extern int func_001151B4(const void *, const void *, int); /* memcmp */

/* memcard_RestoreData: unpack the save blocks after buf's 8-byte header
   into the table entries with the same id (slot selects the element),
   marking each entry 1 (same size), -1 (block shorter) or -2 (longer),
   and count blocks with no entry, a wrong total and entries left
   unrestored as errors, stored in the current card slot. Adapted from
   Lombyte (MIT) for PAL. */
int func_0020BD70(void *src, int slot, int *table) {
    unsigned char *buf = src;
    struct RestoreEntry *tbl = (struct RestoreEntry *)table;
    struct RestoreEntry *e;
    int errors;
    int total;
    int i;
    int n;
    unsigned char *dst;

    if (memcard_TestChecksum((char *)buf) == 0) {
        return 1;
    }
    buf += 8;
    errors = 0;
    total = 8;
    for (i = 0; tbl[i].data != 0; i++) {
        tbl[i].status = 0;
    }
    while (((struct RestoreBlock *)buf)->id != -1) {
        i = 0;
        if (tbl[0].data == 0) {
            goto missing;
        }
        while (tbl[i].id != ((struct RestoreBlock *)buf)->id) {
            i++;
            if (tbl[i].data == 0) {
                goto missing;
            }
        }
        e = (struct RestoreEntry *)(((unsigned int)i << 4) + (unsigned int)tbl);
        if (e->data != 0) {
            int bsize = ((struct RestoreBlock *)buf)->size;

            dst = e->data + slot * e->size;
            if (bsize == e->size) {
                n = e->size;
                e->status = 1;
            } else if (bsize < e->size) {
                n = bsize;
                e->status = -1;
            } else {
                n = e->size;
                e->status = -2;
            }
            if (func_001151B4(dst, ((struct RestoreBlock *)buf)->data, n) != 0) {
                (*(int *)&D_0015FF50)++;
            }
            func_001F9A00(dst, ((struct RestoreBlock *)buf)->data, n);
            total += ((n + 3) & ~3) + 8;
        } else {
        missing:
            errors++;
        }
        buf += ((((struct RestoreBlock *)buf)->size + 3) & ~3) + 8;
    }
    total += 8;
    if (total != memcard_GetDataSize((int *)tbl)) {
        errors++;
    }
    buf += 8;
    for (i = 0; tbl[i].data != 0 && tbl[i].id != ((struct RestoreBlock *)buf)->id; i++) {
        if (tbl[i].status <= 0) {
            errors++;
        }
    }
    D_0013D390_c.slot[D_0013D390_c.cur].errors = errors;
    return errors;
}

extern void func_00121A80(void *);
extern void func_0012D818(void *);
extern void func_00208FA0(void);
extern void func_00208338(void *);
extern char D_0015EF98[] MACRO_ADDR;
extern char D_00141FC0[];
extern unsigned char D_0013DE60[];
extern int D_0015EE98 MACRO_ADDR;
extern int D_0015EF24 MACRO_ADDR;
extern int D_0015EF20 MACRO_ADDR;
extern int D_0015EFB4_m __asm__("D_0015EFB4") MACRO_ADDR;
extern char D_0014EFD0[];
extern char D_001507D0[];

/* memcard_Save(slot, flags): starts the save (func_00121A80/func_0012D818
   on D_0015EF98, func_00208FA0, func_00208338 on the D_00141FC0 page for
   D_0015EE84). With no valid card slot (D_0013D390+0xC8, its record's
   +0x14) it returns whether `slot` is 0. Otherwise it ORs `slot` into the
   pending mask (+0xFC); unless nothing is pending, a save is running
   (+0xDC >= 3) or a result is waiting (+0xE4 >= 0), it stamps +0xD0 with
   D_0015EE84 (temporarily switching D_0015EE84 to `flags` and setting that
   D_0013DE60 byte), fills entry e[+0x14] (four globals and the 8-byte
   name), serialises both tables (func_0020BBC8), restores the byte and
   D_0015EE84, and sets result 0xF. Returns whether the result is 0xF.
   D_0015EF98 is MACRO_ADDR (retail rebuilds its address at each use), the
   name field is reached from a `q + 0x30` base, and each block reads
   D_0013D390 through its own `char *` local (retail keeps only the %hi). */
int func_0020BFC8(int slot, int flags) {
    unsigned char saved;

    func_00121A80(D_0015EF98);
    func_0012D818(D_0015EF98);
    func_00208FA0();
    func_00208338(D_00141FC0 + (D_0015EE84_m << 11));
    {
        char *p = D_0013D390;
        int active = *(int *)(p + 0xC8);
        int mask;

        if (active == -1 || *(int *)(p + active * 0xC0 + 0x14) < 0) {
            return slot == 0;
        }
        mask = *(int *)(p + 0xFC) | slot;
        *(int *)(p + 0xFC) = mask;
        if (mask == 0) {
            goto done;
        }
        if (slot == 0) {
            D_0015EFB4_m |= 0x200;
        }
        if (*(int *)(p + 0xDC) >= 3) {
            goto done;
        }
        if (*(int *)(p + 0xE4) >= 0) {
            goto done;
        }
        *(int *)(p + 0xD0) = D_0015EE84_m;
        saved = 0;
        if (flags >= 0) {
            D_0015EE84_m = flags;
            saved = D_0013DE60[flags];
            if (saved == 0) {
                D_0013DE60[flags] = 1;
            }
        }
    }
    {
        char *q = D_0013D390;
        char *names = q + 0x30;

        *(int *)(q + *(int *)(q + 0x14) * 0x1C + 0x24) = gBolts;
        *(int *)(q + *(int *)(q + 0x14) * 0x1C + 0x20) = D_0015EE84_m;
        *(int *)(q + *(int *)(q + 0x14) * 0x1C + 0x2C) = D_0015EF24;
        memcpy(names + *(int *)(q + 0x14) * 0x1C, D_0015EF98, 8);
        *(int *)(q + *(int *)(q + 0x14) * 0x1C + 0x28) = D_0015EF20;
        memcard_PrepData(D_0014EFD0, 0, gGameSaveData);
        memcard_PrepData(D_001507D0, *(int *)(q + 0xD0), gLevelSaveData);
        if (flags >= 0) {
            D_0013DE60[D_0015EE84_m] = saved;
            D_0015EE84_m = *(int *)(q + 0xD0);
        }
        if (*(int *)(q + 0xE4) < 0) {
            *(int *)(q + 0xE4) = 0xF;
            *(int *)(q + 0xE8) = *(int *)(q + 0xC8);
        }
    }
done:
    {
        char *r = D_0013D390;
        return *(int *)(r + 0xE4) == 0xF;
    }
}
__asm__(".section .text\n\tnop\n");
