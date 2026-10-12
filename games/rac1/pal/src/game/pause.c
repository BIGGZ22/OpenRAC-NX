#include "common.h"
#include "callees.h"
#include "records.h"
#include "pad.h"
#include "moby.h"
#include "structs.h"

/*
 * pause.cpp in the original source; text 0x219C08-0x228A58.
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
/* MACRO_ADDR: lui/lw where retail has them, $gp-relative in a delay slot
   (func_0021F7D0, func_002229B0). */
extern int D_0015EE84 MACRO_ADDR;
extern int D_001A0218[] NOT_SDA;
extern void func_00208458(void *, unsigned char *, int);
extern void func_00208688(void *, unsigned char *);
extern char D_0013D390[];
extern short D_0015EFB0;
extern int D_0015EFB4;
extern int D_001A05C0[];
extern int D_001A08C0[];
extern int func_0020BAD8(int *p);
extern int func_0020BBC8(void *dst, int i, int *table);
extern int func_001236F0(void);
extern int func_001E9730();
extern char D_001E8690[];
extern int D_0013D844 NOT_SDA;
extern unsigned char D_0013D4A8 NOT_SDA;
extern int D_0013D9B4 NOT_SDA;
extern unsigned char D_0013D490[];
extern unsigned char D_0013D5CA NOT_SDA;
extern int D_0013D6B8 NOT_SDA;
extern int D_0013DAE4 NOT_SDA;
extern unsigned char D_0013D4E5 NOT_SDA;
extern int D_0013DB24 NOT_SDA;
extern unsigned char D_0013D4F1 NOT_SDA;
extern int D_0013DC34 NOT_SDA;
extern unsigned char D_0013D605 NOT_SDA;
extern int D_0013D5C8 NOT_SDA;
extern unsigned char D_0013D4B0 NOT_SDA;
extern unsigned char D_0013DE55 NOT_SDA;
extern unsigned char D_0013D5DD NOT_SDA;
extern unsigned char D_0013D5E7 NOT_SDA;
extern int D_001B2F40[];
extern void func_001FA460_2(void *, void *) __asm__("func_001FA460");
extern void func_001F9C30(void *, void *, float);
extern void func_001F9BD8(void *, void *, void *);
extern void func_001F9EC0(void *, void *, void *);
extern int D_001414D0 NOT_SDA;
extern float D_001CAE00[] NOT_SDA;
extern float func_001F9F90(float);
extern float func_001F9FA8(float);
extern void func_00118D80(int);
extern char D_00165600[];
extern int D_0015F718;
extern short D_0015F71C;
extern char D_001B3200[];
extern int func_001160D8(void);
extern float func_00214158(void);
extern float func_002140F8(float, float);
extern void func_00215C00(void *, float, float, float);
extern void func_001F9DC0(void *, void *, float);
extern void func_001FA460(void *);
extern void func_002150B0(void *, void *);
extern void func_001FA480(void *, void *);
extern float func_0020D830(void);
extern float func_00215A98(int, float);
extern unsigned char D_0014BFC0[];
extern unsigned char D_0013E620[];
extern unsigned char D_0013D510[];
extern void func_0012F068(void *);
extern void func_002177F0(int);
extern short D_001517D0[];
extern void func_0012EDE0(void *);
extern void func_0012EFE8(void);
extern char D_001E8980[];
extern int func_0012EE98(int, int, int, void *);
extern int func_00217628_v(void) __asm__("func_00217628");
extern void func_00122598(int);
extern void func_00217130(void);
extern void func_0012EC40(void);
extern void func_0012DDC0(void);
extern void func_0012EC30(void);
extern int func_0012F030(void);
extern void func_002167C0(short, short, short);
extern void func_002169B8(short, short, short);
extern short D_001517F0 NOT_SDA;
extern char D_0013CA40[];
extern int D_001CDAE0 NOT_SDA;
extern void func_00124650(void);
extern void func_00124B88(int);
extern int func_00124BC8(void *, void *);
extern void func_00217F68(void *);
extern void func_001F68E8(void *a, void *b, void *c, void *d, void *e);
extern void *func_001FE540(void);
extern int func_00205790(void);
extern void func_0020BA00(char *out);
extern void func_00217588(void);

struct S {
    u8 pad_0[0x140];
    float f140, f144, f148;
    u8 pad_14C[0x204];
    float f350;
    u8 pad_354[0x10];
    float f364;
    u8 pad_368[0x10];
    float f378, f37C;
};
extern struct S D_00187040_19C08 __asm__("D_00187040");
extern s32 D_001873A0;
extern s32 D_001873B0;

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/rendering/fun_00218d10.c, FUN_00218d10. */
void func_00219C08(void) {
    D_00187040_19C08.f140 = 256.0f;
    D_00187040_19C08.f148 = 64.0f;
    D_00187040_19C08.f144 = 256.0f;
    qzero(&D_00187040_19C08.f350);
    qzero(&D_001873A0);
    qzero(&D_001873B0);
    D_00187040_19C08.f350 = 1.0f;
    D_00187040_19C08.f364 = 1.0f;
    D_00187040_19C08.f378 = 1.0f;
    D_00187040_19C08.f37C = 1.0f;
}

extern void func_0012E528(int);
extern void func_00216EF0(int);
extern int D_0018C42C;
extern int D_0015F754 MACRO_ADDR;
extern int D_00141760;
extern unsigned char D_0014171B NOT_SDA;
extern int D_0015EFA0 MACRO_ADDR;
extern int D_0015EF20 MACRO_ADDR;
extern char D_001CE938[];
extern char D_001CEAC8[];
extern char D_001CEB18[];
extern int D_001A0414;
extern int D_001D0718;
extern int func_0020C7A0_i(void) __asm__("func_0020C7A0");
extern int func_0012DDC0_i(void) __asm__("func_0012DDC0");
extern void func_00228160(void);
extern char D_001D5F70[] NOT_SDA;
extern int D_0015F6E8 MACRO_ADDR;

/* PauseAllSounds: pause entry. Unless D_0018C42C is set (then only
   D_0015F754 = 1), it computes the pause object's flags (0x134, 0x138,
   0xD8 "loaded", 0xDC "arg0 is 0x23"), points the D_001CE938/D_001CEAC8
   records' +0x38/+0x3C at the loaded or unloaded variants, resets the
   object for arg0 (func_00219E60's body) and D_001A0414, then calls
   func_0020C7A0 and func_00228160. Each group reads D_001D5F70 through
   its own `char *` local (retail's saved %hi); func_0012DDC0 and
   func_0020C7A0 return values, which moves the next temporary to $v1. */
void func_00219C70(int arg0) {
    func_0012E528(0x1D);
    music_Pause(0);
    func_0012DDC0_i();
    if (D_0018C42C != 0) {
        D_0015F754 = 1;
        return;
    }
    if (D_00141760 == 0x24) {
        D_00141760 = 0;
    }
    {
        char *g = D_001D5F70;
        *(int *)(g + 0x134) = D_0015EE84 == 0xD || D_0014171B != 0;
    }
    {
        char *g = D_001D5F70;
        *(int *)(g + 0x138) = D_0015EE84 == 0 || D_0015EE84 == 0xE;
    }
    {
        char *g = D_001D5F70;
        *(int *)(g + 0xD8) = D_0015EFA0 != 0 || D_0015EF20 != 0
                             || *(int *)(g + 0xF8) != 0;
    }
    {
        char *g = D_001D5F70;
        char *a = D_001CE938;
        *(int *)(g + 0xDC) = arg0 == 0x23;
        *(char **)(a + 0x38) = *(int *)(g + 0xD8) ? D_001CEB18 : D_001CEAC8;
    }
    {
        char *g = D_001D5F70;
        char *b = D_001CEAC8;
        *(char **)(b + 0x3C) = *(int *)(g + 0xD8) ? D_001CEB18 : D_001CE938;
    }
    {
        char *g = D_001D5F70;
        D_0015F6E8 = 3;
        *(int *)g = arg0;
        *(int *)(g + 0xC) = 0;
        *(int *)(g + 0x10) = 0;
        *(int *)(g + 0x110) = 0;
    }
    if (D_0015EE84 < 0x13) {
        D_001A0414 = D_0015EE84;
    } else {
        D_001A0414 = 0;
    }
    func_0020C7A0_i();
    D_001D0718 = 0;
    func_00228160();
    {
        char *g = D_001D5F70;
        *(int *)(g + 0x13C) = 1;
        *(int *)(g + 0x140) = 0;
    }
}

LINKER_REMNANT("asm/remnants/text", func_00219E48);

void func_00219E60(void) {
    char *p = D_001D5F70;
    D_0015F6E8 = 3;
    *(int *)p = 0x2D;
    *(int *)(p + 0xC) = 0;
    *(int *)(p + 0x10) = 0;
    *(int *)(p + 0x110) = 0;
}

extern s32 D_0015EF78_19E90 __asm__("D_0015EF78") MACRO_ADDR;

struct MenuScreen_19E90;

struct MenuPage_19E90 {
    s32 moby_anims[14];
    struct MenuPage_19E90 *back;
    s32 state;
    struct MenuScreen_19E90 *focus;
    struct MenuScreen_19E90 *screens[14];
    u8 pad_7C[0x4];
    struct MenuScreen_19E90 *pending_focus;
    s32 confirmed;
};

struct MenuSystem_19E90 {

    s32 state;
    struct MenuPage_19E90 *current;
    struct MenuPage_19E90 *next;
    s32 close_request;
    s32 unk10;
    s32 timer;
    s32 saved_texture_start;
    s32 current_gadget;
    u8 pad_20[0x10];
    s32 equipped[4];
    u8 pad_40[0x60];

    s32 stream_buffer[2];
    s32 unkA8;
    s32 unkAC;
    s32 unkB0[3];
    s32 read_offset;
    s32 unkC0;
    u8 pad_C4[0x4];
    u8 loaded_animation[2];
    u8 read_buffer_index;
    u8 pending_buffer;
    u32 streamed_animation_base;

    struct MenuPage_19E90 *previous;
    s32 unkD4;
    s32 unkD8;
    s32 unkDC;
    s32 unkE0;
    s32 unkE4;
    u8 pad_E8[0x4];
    s32 unkEC;
    struct MenuPage_19E90 *unkF0;
    s32 unkF4;
    s32 unkF8;

    s32 unkFC;
    s32 unk100;
    s32 unk104;
    s32 help_text_buffer;
    s32 unk10C;

    s32 update_count;
    u8 pad_114[0x4];

    s32 resource_table_toggle;
    s32 unk11C;
    s32 unk120;

    s32 close_locked;
    s32 card_op_pending;
    s32 card_op_text;
    u8 pad_130[0x4];
    s32 unk134;
    s32 unk138;
    s32 unk13C;
    s32 unk140;
    s32 last_resource_table_toggle;
};

extern struct MenuSystem_19E90 D_001D5F70_19E90 __asm__("D_001D5F70");

struct S_19E90 {
    u8 pad_0[0x140];
    float f140, f144, f148;
};
struct P_19E90 {
    u8 pad_0[0x10];
    u8 f10;
};
struct T_19E90 {
    u8 pad_0[0x48];
    struct P_19E90 *tbl[1];
};
struct O2_19E90 {
    u8 pad_0[0x10];
    float f10, f14, f18;
    u8 pad_1C[8];
    struct T_19E90 *f24;
    u8 pad_28[0xC];
    u16 f34;
    u8 pad_36[0xA];
    s32 f40, f44, f48;
    u8 pad_4C[0x28];
    s32 *f74;
};
extern struct MenuPage_19E90 D_001D4948_19E90 __asm__("D_001D4948"); /* page made current and next at init */
extern u8 D_0019C250_19E90[] __asm__("D_0019C250");
extern u8 D_0019C260_19E90[] __asm__("D_0019C260");
extern s32 D_001941C0_19E90[] __asm__("D_001941C0");
extern s32 D_0015F538_19E90 __asm__("D_0015F538") MACRO_ADDR;
extern s32 D_0016100C_19E90 __asm__("D_0016100C") MACRO_ADDR;
extern void D_0023B578_19E90() __asm__("D_0023B578");
extern struct S_19E90 D_00187040_19E90 __asm__("D_00187040");
extern struct O2_19E90 *D_001D6120_19E90[] __asm__("D_001D6120");
extern u8 D_001602C0_19E90 __asm__("D_001602C0") MACRO_ADDR;
extern u8 D_001602D0_19E90 __asm__("D_001602D0") MACRO_ADDR;
extern s32 func_001F9C30_19E90(s32, s32, f32) __asm__("func_001F9C30");
extern void func_00234AC8_19E90(s32) __asm__("func_00234AC8");
extern s32 func_00122598_19E90(s32) __asm__("func_00122598");
extern void func_002348E8_19E90(void) __asm__("func_002348E8");
extern struct O2_19E90 *func_00226720_19E90() __asm__("func_00226720");
extern s32 func_00213D28_19E90() __asm__("func_00213D28");

void func_00226D50_19E90() __asm__("func_00226D50");
/* Opens the pause menu: makes the start page current, sets up the menu camera vectors and the
   texture and help text buffers behind the two stream buffers, then creates the fourteen menu
   mobys and starts each on its page animation.
   Adapted from Lombyte (MIT) for PAL: src/rendering/fun_00218f98.c, FUN_00218f98. */
void func_00219E90(void) {
    s32 i;
    s32 a, b, c, d;
    D_001D5F70_19E90.state = 2;
    D_001D5F70_19E90.current = &D_001D4948_19E90;
    D_001D5F70_19E90.next = &D_001D4948_19E90;
    D_001D5F70_19E90.close_locked = 0;
    func_001F9C30_19E90((s32)D_0019C250_19E90, (s32)&D_001602C0_19E90, 1.0f);
    qcopy(D_0019C250_19E90 - 0x10, &D_001602D0_19E90);
    qzero(D_0019C250_19E90 + 0x20);
    qzero(D_0019C260_19E90);
    func_00234AC8_19E90(1);
    func_00122598_19E90(0);
    a = D_001941C0_19E90[1] + 0xA0000;
    b = D_001941C0_19E90[2] + 0xA0000;
    c = a + 0x30000;
    d = b + 0xE0000;
    D_001D5F70_19E90.unkFC = c + 0xC1000;
    D_0015F538_19E90++;
    D_001D5F70_19E90.unk100 = d + 0x11800;
    D_0016100C_19E90 = 0xA0000;
    D_001D5F70_19E90.unk104 = a;
    D_001D5F70_19E90.unk10 = b;
    D_001D5F70_19E90.help_text_buffer = c;
    D_001D5F70_19E90.unk10C = d;
    func_00226D50_19E90(1);
    func_002348E8_19E90();
    D_001D5F70_19E90.saved_texture_start = D_0015EF78_19E90;
    if (D_001D5F70_19E90.current != 0) {
        for (i = 0; i < 14; i++) {
            struct O2_19E90 *o = (struct O2_19E90 *)func_00226720_19E90(0x472);
            D_001D6120_19E90[i] = o;
            if (o != 0) {
                s32 k;
                o->f34 &= 0xFFFD;
                D_001D6120_19E90[i]->f74 = D_0023B578_19E90;
                D_001D6120_19E90[i]->f10 = D_00187040_19E90.f140;
                D_001D6120_19E90[i]->f14 = D_00187040_19E90.f144;
                D_001D6120_19E90[i]->f18 = D_00187040_19E90.f148;
                D_001D6120_19E90[i]->f40 = 0;
                D_001D6120_19E90[i]->f44 = 0;
                D_001D6120_19E90[i]->f48 = 0;
                k = D_001D5F70_19E90.current->moby_anims[i];
                func_00213D28_19E90(D_001D6120_19E90[i], k, D_001D6120_19E90[i]->f24->tbl[k]->f10 - 1);
            }
        }
    }
}

extern int D_0015EF78 MACRO_ADDR;
extern int func_002267C0(int);
extern int D_001D6120[];

typedef struct { char pad[0x44]; char *items[14]; } ObjList;

/* Pause teardown: calls each object's +0xC handler, then refreshes the
   14 D_001D6120 handles. Retail's register copies come from gcse, so the
   base is re-read inside each block (and inside the loop body). */
void func_0021A0B0(void) {
    char *g = D_001D5F70;

    if (*(int *)(g + 0x110) < 10) {
        return;
    }
    if (*(int *)(g + 4) != 0) {
        int i;
        for (i = 0; i < 14; i++) {
            char *g2 = D_001D5F70;
            char *obj = ((ObjList *)*(char **)(g2 + 4))->items[i];
            if (obj != 0) {
                void (*fn)(void *, int) = *(void (**)(void *, int))(obj + 0xC);
                if (fn != 0) {
                    fn(obj, 0);
                }
            }
        }
        {
            char *g5 = D_001D5F70;
            *(int *)(g5 + 4) = 0;
        }
    }
    {
        char *g3 = D_001D5F70;
        int j;
        D_0015EF78 = *(int *)(g3 + 0x18);
        for (j = 0; j < 14; j++) {
            D_001D6120[j] = func_002267C0(D_001D6120[j]);
        }
    }
    {
        char *g4 = D_001D5F70;
        *(int *)g4 = 0x14;
        *(int *)(g4 + 0x14) = 2;
    }
}

extern s32 D_0015F6E8_1A1A0 __asm__("D_0015F6E8") MACRO_ADDR;

extern s32 D_0015EFB4_1A1A0 __asm__("D_0015EFB4") MACRO_ADDR;

struct MenuScreen_1A1A0;

struct MenuPage_1A1A0 {
    s32 moby_anims[14];
    struct MenuPage_1A1A0 *back;
    s32 state;
    struct MenuScreen_1A1A0 *focus;
    struct MenuScreen_1A1A0 *screens[14];
    u8 pad_7C[0x4];
    struct MenuScreen_1A1A0 *pending_focus;
    s32 confirmed;
};

struct MenuSystem_1A1A0 {

    s32 state;
    struct MenuPage_1A1A0 *current;
    struct MenuPage_1A1A0 *next;
    s32 close_request;
    s32 unk10;
    s32 timer;
    s32 saved_texture_start;
    s32 current_gadget;
    u8 pad_20[0x10];
    s32 equipped[4];
    u8 pad_40[0x60];

    s32 stream_buffer[2];
    s32 unkA8;
    s32 unkAC;
    s32 unkB0[3];
    s32 read_offset;
    s32 unkC0;
    u8 pad_C4[0x4];
    u8 loaded_animation[2];
    u8 read_buffer_index;
    u8 pending_buffer;
    u32 streamed_animation_base;

    struct MenuPage_1A1A0 *previous;
    s32 unkD4;
    s32 unkD8;
    s32 unkDC;
    s32 unkE0;
    s32 unkE4;
    u8 pad_E8[0x4];
    s32 unkEC;
    struct MenuPage_1A1A0 *unkF0;
    s32 unkF4;
    s32 unkF8;

    s32 unkFC;
    s32 unk100;
    s32 unk104;
    s32 help_text_buffer;
    s32 unk10C;

    s32 update_count;
    u8 pad_114[0x4];

    s32 resource_table_toggle;
    s32 unk11C;
    s32 unk120;

    s32 close_locked;
    s32 card_op_pending;
    s32 card_op_text;
    u8 pad_130[0x4];
    s32 unk134;
    s32 unk138;
    s32 unk13C;
    s32 unk140;
    s32 last_resource_table_toggle;
    s32 unk148;               /* 0x148: PAL only, counts down to 0 */
    u8 pad_14C[0x8];
    s32 unk154;               /* 0x154: PAL only, updates since the memory card went idle */
};

/* Memory card state at D_0013D390, as far as this function reads it (PAL offsets). */
struct MemoryCardState_1A1A0 {
    u8 pad_0[0xDC];
    s32 state;                /* 0xDC */
    s32 sub;
    s32 pending_state;        /* 0xE4 */
};
extern struct MemoryCardState_1A1A0 D_0013D390_1A1A0 __asm__("D_0013D390");

extern struct MenuSystem_1A1A0 D_001D5F70_1A1A0 __asm__("D_001D5F70");

struct MenuScreenWords_1A1A0 {
    u8 pad_30[0x4];
    s32 unk34;
    s32 unk38;
    s32 unk3C;
    u8 pad_40[0x10];
    s32 unk50;
    s32 unk54;
    u8 pad_58[0x8];
};

struct MenuScreen_1A1A0 {
    s32 (*update)(struct MenuScreen_1A1A0 *);
    u8 pad_4[0x4];
    void (*enter)(struct MenuScreen_1A1A0 *, s32);
    void (*leave)(struct MenuScreen_1A1A0 *, s32);
    s32 unk10;
    s32 moby;
    s32 x;
    s32 y;
    s32 width;
    s32 height;
    u8 pad_28[0x8];
    union {
        struct MenuScreenWords_1A1A0 raw;
    } data;
};

struct Ent3_1A1A0 {
    u8 pad0[0x10];
    u8 unk10;
};

struct Ent2_1A1A0 {
    u8 pad0[0x48];
    struct Ent3_1A1A0 *unk48[1];
};

struct Ent_1A1A0 {
    u8 pad0[0x24];
    struct Ent2_1A1A0 *unk24;
    u8 pad28[0x30];
    f32 unk58;
};

extern s16 D_001517D8_1A1A0[] __asm__("D_001517D8");
extern s32 D_0015F698_1A1A0 __asm__("D_0015F698") MACRO_ADDR;
extern s32 D_0015F6FC_1A1A0 __asm__("D_0015F6FC") MACRO_ADDR;
extern struct Ent_1A1A0 *D_001D6120_1A1A0[] __asm__("D_001D6120");

extern void func_00219C08_1A1A0(void) __asm__("func_00219C08");
extern void func_00219E90_1A1A0(void) __asm__("func_00219E90");
extern void func_00213C78_1A1A0() __asm__("func_00213C78");
extern s32 func_0022ED80_1A1A0(s32, s32, struct Ent_1A1A0 *) __asm__("func_0022ED80");
extern void func_001FBC80_1A1A0(s32, struct MenuPage_1A1A0 *, s32) __asm__("func_001FBC80");
extern void func_002348B8_1A1A0(void) __asm__("func_002348B8");
extern void func_001F2930_1A1A0(void) __asm__("func_001F2930");
extern void func_00213D28_1A1A0(struct Ent_1A1A0 *, s32, s32) __asm__("func_00213D28");
extern void func_0021A0B0_1A1A0(void) __asm__("func_0021A0B0");

/* Pause menu update: counts the update, closes down in state 0x14, opens the menu in
   states 0 and 0x2D, then runs the page switch (leave handlers, the mobys' page animations,
   enter handlers after the timer), every screen's update and the focus hand-over.
   Adapted from Lombyte (MIT) for PAL: src/ui/menus/fun_002192a8.c, FUN_002192a8. */
void func_0021A1A0(void) {
    s32 i;
    s32 j;
    s32 k;
    s32 flag;
    struct MenuScreen_1A1A0 *obj;
    struct MenuScreen_1A1A0 **objs;
    struct MenuScreen_1A1A0 *m;

    D_001D5F70_1A1A0.update_count = D_001D5F70_1A1A0.update_count + 1;
    if (D_001D5F70_1A1A0.update_count > 0x7D00) {
        D_001D5F70_1A1A0.update_count = 0x7D00;
    }
    func_00219C08_1A1A0();
    if (D_001D5F70_1A1A0.unk148 != 0) {
        D_001D5F70_1A1A0.unk148 = D_001D5F70_1A1A0.unk148 - 1;
    }
    if (D_0013D390_1A1A0.state < 3 && D_0013D390_1A1A0.pending_state < 0) {
        D_001D5F70_1A1A0.unk154 = D_001D5F70_1A1A0.unk154 + 1;
    } else {
        D_001D5F70_1A1A0.unk154 = 0;
    }
    if (D_001D5F70_1A1A0.state == 0x14) {
        if (D_001D5F70_1A1A0.timer != 0) {
            D_001D5F70_1A1A0.timer = D_001D5F70_1A1A0.timer - 1;
            if (D_001D5F70_1A1A0.timer != 0) {
                return;
            }
        }
        if (D_001517D8_1A1A0[0] != 0) {
            return;
        }
        D_0015F698_1A1A0 = 0x1E000;
        func_002348B8_1A1A0();
        func_001F2930_1A1A0();
        D_0015F6FC_1A1A0 = 1;
        D_001D5F70_1A1A0.help_text_buffer = 0;
        D_001D5F70_1A1A0.unk10C = 0;
        D_001D5F70_1A1A0.unk104 = 0;
        D_001D5F70_1A1A0.unk10 = 0;
        D_0015F6E8_1A1A0 = 0;
        return;
    }
    if (D_001D5F70_1A1A0.state == 0 || D_001D5F70_1A1A0.state == 0x2D) {
        func_00219E90_1A1A0();
    }
    if (D_0015EFB4_1A1A0 & 1) {
        func_001FBC80_1A1A0(3, D_001D5F70_1A1A0.current, 0);
        return;
    }
    if (D_001D5F70_1A1A0.state == 1) {
        D_001D5F70_1A1A0.timer = (D_001D5F70_1A1A0.timer < 1) ? 0 : D_001D5F70_1A1A0.timer - 1;
        if (D_001D5F70_1A1A0.timer == 0) {
            struct MenuPage_1A1A0 *o8 = D_001D5F70_1A1A0.next;
            D_001D5F70_1A1A0.next = 0;
            D_001D5F70_1A1A0.current = o8;
            D_001D5F70_1A1A0.state = o8->state;
            for (i = 0; i < 14; i++) {
                obj = D_001D5F70_1A1A0.current->screens[i];
                if (obj != 0 && obj->enter != 0) {
                    obj->enter(obj, 0);
                }
            }
        }
    } else if (D_001D5F70_1A1A0.next != 0) {
        if (D_001D5F70_1A1A0.current == D_001D5F70_1A1A0.next) {
            func_0022ED80_1A1A0(3, 0x11, D_001D6120_1A1A0[0]);
        } else {
            func_0022ED80_1A1A0(4, 0x11, D_001D6120_1A1A0[0]);
        }
        for (i = 0; i < 14; i++) {
            obj = D_001D5F70_1A1A0.current->screens[i];
            if (obj != 0 && obj->leave != 0) {
                obj->leave(obj, 0);
            }
        }
        flag = (D_001D5F70_1A1A0.next == D_001D5F70_1A1A0.current->back);
        if (D_001D5F70_1A1A0.current == D_001D5F70_1A1A0.next) {
            flag = flag ^ 1;
        }
        for (j = 0; j < 14; j++) {
            if (D_001D5F70_1A1A0.next->screens[j] != 0) {
                D_001D5F70_1A1A0.next->screens[j]->moby = (s32)D_001D6120_1A1A0[j];
            }
            if (flag) {
                s32 n = D_001D5F70_1A1A0.current->moby_anims[j];
                func_00213D28_1A1A0(D_001D6120_1A1A0[j], n, D_001D6120_1A1A0[j]->unk24->unk48[n]->unk10 - 1);
                D_001D6120_1A1A0[j]->unk58 = -1.0f;
            } else {
                func_00213D28_1A1A0(D_001D6120_1A1A0[j], D_001D5F70_1A1A0.next->moby_anims[j], 0);
                D_001D6120_1A1A0[j]->unk58 = 1.0f;
            }
        }
        D_001D5F70_1A1A0.state = 1;
        D_001D5F70_1A1A0.timer = 12;
        D_001D5F70_1A1A0.previous = D_001D5F70_1A1A0.current;
        D_001D5F70_1A1A0.current = 0;
    }
    if (D_001D5F70_1A1A0.current != 0) {
        objs = D_001D5F70_1A1A0.current->screens;
        for (k = 0; k < 14; k++) {
            if (objs != 0 && objs[k] != 0 && objs[k]->update != 0) {
                if (objs[k]->update(objs[k]) != 0) {
                    D_001D5F70_1A1A0.close_request = 1;
                }
            }
        }
        if (D_001D5F70_1A1A0.current != 0 && D_001D5F70_1A1A0.current->pending_focus != 0) {
            m = D_001D5F70_1A1A0.current->focus;
            if (m->leave != 0) {
                m->leave(m, 1);
            }
            D_001D5F70_1A1A0.current->focus = D_001D5F70_1A1A0.current->pending_focus;
            D_001D5F70_1A1A0.current->pending_focus = 0;
            m = D_001D5F70_1A1A0.current->focus;
            if (m->enter != 0) {
                m->enter(m, 1);
            }
        }
    }
    func_00213C78_1A1A0();
    if (D_001D5F70_1A1A0.close_request != 0) {
        func_0021A0B0_1A1A0();
    }
}

INCLUDE_ASM("asm/nonmatchings/text", func_0021A610);

/*
 * Pad handler for the pause page arg0 (g->x4->x40): 1 on the 0xD00
 * buttons while g->x124 is 0; on 0x10, switch to the page's +0x38
 * target, or return -1 when it has none and g->x124 is 0. Written in
 * its family's shape (func_0021F7D0, func_00222DB0): early returns and
 * one `char *` local per block that reads a global.
 */
int func_0021ACD8(int arg0) {
    char *g = D_001D5F70;

    if (*(int *)(*(char **)(g + 4) + 0x40) != arg0) {
        return 0;
    }
    {
        char *pad = D_0013CA40;
        if (*(int *)(pad + 0x1C4) & 0xD00) {
            if (*(int *)(g + 0x124) == 0) {
                return 1;
            }
        }
    }
    {
        char *pad2 = D_0013CA40;
        if (*(int *)(pad2 + 0x1C4) & 0x10) {
            char *g2 = D_001D5F70;
            int t = *(int *)(*(char **)(g2 + 4) + 0x38);
            if (t != 0) {
                *(int *)(g2 + 8) = t;
                return 0;
            }
            if (*(int *)(g2 + 0x124) == 0) {
                return -1;
            }
        }
    }
    return 0;
}

extern int D_001D6094;
extern void func_0022ED80(int, int, int);

/* Pause list handler (func_0021ACD8's family: one `char *` pad local per
   block). The cursor at arg0+0x3C moves on 0x1000/0x4000 within
   arg0+0x40, with a func_0022ED80 notify on change; then the scroll
   (arg0+0x60) is clamped around the cursor and the bar position at
   arg0+0x5C derived from it, or pinned to 0x52. The re-reads of 0x3C and
   0x60 are how retail's reloads come out; the page size q - 2 is formed
   after the clamp, and each arm stores the bar position itself. */
int func_0021AD68(char *arg0) {
    {
        char *pad = D_0013CA40;
        if (*(int *)(pad + 0x1C4) & 0xD00) {
            if (D_001D6094 == 0) {
                return 1;
            }
        }
    }
    {
        char *pad2 = D_0013CA40;
        if (*(int *)(pad2 + 0x1C4) & 0x10) {
            char *g = D_001D5F70;
            int t = *(int *)(*(char **)(g + 4) + 0x38);
            if (t != 0) {
                *(int *)(g + 8) = t;
                return 0;
            }
            if (*(int *)(g + 0x124) == 0) {
                return -1;
            }
        }
    }
    {
        char *pad3 = D_0013CA40;
        int v = *(int *)(pad3 + 0x1C4);
        int old = *(int *)(arg0 + 0x3C);
        if ((v & 0x1000) && old != 0) {
            *(int *)(arg0 + 0x3C) = old - 1;
        }
        if (v & 0x4000) {
            int c = *(int *)(arg0 + 0x3C) + 1;
            if (c < *(int *)(arg0 + 0x40)) {
                *(int *)(arg0 + 0x3C) = c;
            }
        }
        if (*(int *)(arg0 + 0x3C) != old) {
            func_0022ED80(1, 0x11, *(int *)(arg0 + 0x14));
        }
    }
    {
        int t = *(int *)(arg0 + 0x24) * 16;
        if (t / 0x252 >= *(int *)(arg0 + 0x40)) {
            *(int *)(arg0 + 0x5C) = 0x52;
        } else {
            int q = (t - 0x28) / 0x252;
            int n;
            if (*(int *)(arg0 + 0x60) >= *(int *)(arg0 + 0x3C)) {
                *(int *)(arg0 + 0x60) = *(int *)(arg0 + 0x3C) - 1;
                if (*(int *)(arg0 + 0x60) < 0) {
                    *(int *)(arg0 + 0x60) = 0;
                }
            }
            n = q - 2;
            if (*(int *)(arg0 + 0x60) < *(int *)(arg0 + 0x3C) - n) {
                *(int *)(arg0 + 0x60) = *(int *)(arg0 + 0x3C) - n;
            }
            *(int *)(arg0 + 0x5C) = 0x182 - *(int *)(arg0 + 0x60) * 0x252;
        }
    }
    return 0;
}

extern short D_001602B0;              /* SDA, gp -0x6A50 */
extern void func_00201640(int, int, int, int, long, long);
extern int func_00200198(int, int);
extern void func_00200468(int, int, int, int, int, int);
extern void func_00200650(int, int, int, int, int, int);
extern void func_002008B8(int, int, int, int, int, int);
extern int D_0015F538 MACRO_ADDR;
extern int func_001F9B70(int); /* abs */
extern void func_001F4630(int);
extern void func_001F4748(void);

/* Vertical picture list: each entry (10 bytes at +0x48, count at +0x40)
   is a 0x200-square image at x, stepping 0x252 down from the scroll
   position at +0x5C; the selected one (+0x3C) gets a pulsing frame. Up
   and down arrows show when the list runs off the top or the bottom. */
int func_0021AEF8(char *arg0) {
    int x = (*(int *)(arg0 + 0x20) * 16 - 0x200) >> 1;
    int y = *(int *)(arg0 + 0x5C);
    int top = y;
    int i;

    SetupGifPaging(0);
    for (i = 0; i < *(int *)(arg0 + 0x40); i++) {
        char *e = *(char **)(arg0 + 0x48) + i * 10;

        if (*(int *)(arg0 + 0x3C) == i) {
            func_00201640(x - 0x30, y - 0x30, x + 0x230, y + 0x230,
                          (unsigned int)((func_001F9B70((D_0015F538 & 0x3F) - 0x20) + 0x40)
                                         * 0x10202 | 0x80000000), 1);
            func_00201640(x - 0x10, y - 0x10, x + 0x210, y + 0x210,
                          *(int *)&D_001602B0, 1);
        }
        func_002008B8(GetIconFrame(*(unsigned short *)e, *(short *)(e + 2)), x, y,
                      0x200, 0x200, 0x80);
        y += 0x252;
    }
    if (top < 0) {
        func_00201640(0, 0, *(int *)(arg0 + 0x20), 0x14, *(int *)&D_001602B0, 0);
        HudSprite(GetIconFrame(0xE99E, 6), x >> 4, 2, 0x20, 0x10, 0x80);
    }
    if (*(int *)(arg0 + 0x24) * 16 < y) {
        func_00201640(0, *(int *)(arg0 + 0x24) - 0x14, *(int *)(arg0 + 0x20),
                      *(int *)(arg0 + 0x24), *(int *)&D_001602B0, 0);
        func_00200650(GetIconFrame(0xE99E, 6), x >> 4, *(int *)(arg0 + 0x24) - 0x12,
                      0x20, 0x10, 0x80);
    }
    DoGifPaging();
    return 2;
}

extern int D_0015EF90;
extern char D_001D4B90[];
extern char D_001D4BC0[];

extern int D_0015EF90_m __asm__("D_0015EF90") MACRO_ADDR;

/* D_0015EF90 is read through a MACRO_ADDR alias: retail's one-register
   load. */
int func_0021B108(void *arg0) {
    *(char **)((char *)arg0 + 0x34) =
        (D_0015EF90_m != 0) ? D_001D4B90 : D_001D4BC0;
    return 0;
}

/* The two initializer tables of the local arrays below. The first is
   copied as a char block (retail's ldl/ldr for all 32 bytes), the second
   as an int block (ldl/ldr, then lw/sw for the last word). */
typedef struct { char b[0x20]; } Blk32;
typedef struct { int w[7]; } Blk28;
extern Blk32 D_001E8A58;
extern Blk28 D_001E8A78;
/* MACRO_ADDR: retail builds each address in one register (la). */
extern unsigned char D_0015EEC0[] MACRO_ADDR;
extern unsigned char D_0015EEB0[] MACRO_ADDR;
typedef struct {
    int val;
    void *addr;
    int c1;
    int c2;
    int zero;
} Entry14;
extern Entry14 D_001D3E90[];

/* Builds the D_001D3E90 list: for each id in {1, 3, 0, 7, 4, 6, 2} (up
   to the -1) whose D_0015EEC0 flag is set, an entry with its text id
   (0x501A...), &D_0015EEB0[id] and the constants 0x4F5A/0x4F5B; a zero
   val ends the list. A plain indexed for loop: strength reduction gives
   retail's pointer walk, its end test against ids + 12, and the id read
   twice per iteration (exit test and body). */
int func_0021B138(void) {
    int ids[8];
    int names[7];
    int i, n;

    *(Blk32 *)ids = D_001E8A58;
    *(Blk28 *)names = D_001E8A78;
    n = 0;
    for (i = 0; i < 12 && ids[i] != -1; i++) {
        if (D_0015EEC0[ids[i]] != 0) {
            D_001D3E90[n].val = names[i];
            D_001D3E90[n].addr = &gCheats[ids[i]];
            D_001D3E90[n].c1 = 0x4F5A;
            D_001D3E90[n].c2 = 0x4F5B;
            D_001D3E90[n].zero = 0;
            n++;
        }
    }
    D_001D3E90[n].val = 0;
    return 0;
}

int func_0021B278(void) {
    return 0;
}

int func_0021B280(void) {
    return 0;
}

int func_0021B288(void *arg0) {
    *(int *)((char *)arg0 + 0x44) = -1;
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/text", func_0021B298);


struct MenuTextItem_1BB90 {
    s16 text;
    s16 action;
    union {
        s32 value;
        struct {
            u16 lo;
            s16 hi;
        } half;
    } param;
    s16 subtext;
    s16 fade_timer;
};

struct MenuTextListData_1BB90 {
    s32 flags;
    struct MenuTextItem_1BB90 *items;
    struct MenuScreen_1BB90 *up;
    struct MenuScreen_1BB90 *down;
    s32 selected;
    s32 scroll;
};

struct MenuScreen_1BB90 {
    s32 (*update)(struct MenuScreen_1BB90 *);
    u8 pad_4[0x4];
    void (*enter)(struct MenuScreen_1BB90 *, s32);
    void (*leave)(struct MenuScreen_1BB90 *, s32);
    s32 unk10;
    s32 moby;
    s32 x;
    s32 y;
    s32 width;
    s32 height;
    u8 pad_28[0x8];
    union {
        struct MenuTextListData_1BB90 list;
    } data;
};

struct MenuScreen_1BB90;

struct MenuPage_1BB90 {
    s32 moby_anims[14];
    struct MenuPage_1BB90 *back;
    s32 state;
    struct MenuScreen_1BB90 *focus;
    struct MenuScreen_1BB90 *screens[14];
    u8 pad_7C[0x4];
    struct MenuScreen_1BB90 *pending_focus;
    s32 confirmed;
};

struct MenuSystem_1BB90 {

    s32 state;
    struct MenuPage_1BB90 *current;
    struct MenuPage_1BB90 *next;
    s32 close_request;
    s32 unk10;
    s32 timer;
    s32 saved_texture_start;
    s32 current_gadget;
    u8 pad_20[0x10];
    s32 equipped[4];
    u8 pad_40[0x60];

    s32 stream_buffer[2];
    s32 unkA8;
    s32 unkAC;
    s32 unkB0[3];
    s32 read_offset;
    s32 unkC0;
    u8 pad_C4[0x4];
    u8 loaded_animation[2];
    u8 read_buffer_index;
    u8 pending_buffer;
    u32 streamed_animation_base;

    struct MenuPage_1BB90 *previous;
    s32 unkD4;
    s32 unkD8;
    s32 unkDC;
    s32 unkE0;
    s32 unkE4;
    u8 pad_E8[0x4];
    s32 unkEC;
    struct MenuPage_1BB90 *unkF0;
    s32 unkF4;
    s32 unkF8;

    s32 unkFC;
    s32 unk100;
    s32 unk104;
    s32 help_text_buffer;
    s32 unk10C;

    s32 update_count;
    u8 pad_114[0x4];

    s32 resource_table_toggle;
    s32 unk11C;
    s32 unk120;

    s32 close_locked;
    s32 card_op_pending;
    s32 card_op_text;
    u8 pad_130[0x4];
    s32 unk134;
    s32 unk138;
    s32 unk13C;
    s32 unk140;
    s32 last_resource_table_toggle;
    u8 pad_148[0x8];
    s32 unk150;               /* 0x150: PAL only: 0 after action 4, 1 after action 5 */
    s32 unk154;               /* 0x154: PAL only */
};

extern struct MenuSystem_1BB90 D_001D5F70_1BB90 __asm__("D_001D5F70");

extern struct PadState D_0013CA40_1BB90 __asm__("D_0013CA40");
/* gp-relative here, so not the plain rnc/globals.h spellings. */
extern s32 D_0015EE84_1BB90 __asm__("D_0015EE84") MACRO_ADDR;
extern s32 D_0015EE88_1BB90 __asm__("D_0015EE88") MACRO_ADDR;
extern s32 D_001602B4_1BB90 SDATA(D_001602B4);
extern s32 *D_001602E0_1BB90 SDATA(D_001602E0);
extern s32 D_00199578_1BB90[] __asm__("D_00199578");
extern s32 D_001A0414_1BB90[] __asm__("D_001A0414");

extern s32 func_001F98C0_1BB90(s32) __asm__("func_001F98C0");
extern void func_001FBC80_1BB90(s32, s32, s32) __asm__("func_001FBC80");
extern s32 func_0022ED80_1BB90(s32, s32, s32) __asm__("func_0022ED80");

/*
 * Per-frame update of a menu list: fades the entry highlights, handles
 * cancel/back, runs the selected entry's action on accept, and moves the
 * selection up/down (with wrap or hand-off to the neighbouring list).
 */

/* Text list screen update: runs each entry's highlight fade timer, then for the focused
   list the pad: close, back, confirm (the selected entry's action: open a page, freeze the
   game for a prompt, close with a value, pick a level or language) and up/down selection.
   Adapted from Lombyte (MIT) for PAL: src/ui/menus/fun_0021abf8.c, update_menu_entry_actions. */
s32 func_0021BB90(struct MenuScreen_1BB90 *menu) {
    s32 focused;
    s32 entry_index;
    s32 entry_count;
    s32 previous_selection;
    s32 flags;
    s32 buttons;
    s16 fade_timer;
    s16 message_index;

    focused = D_001D5F70_1BB90.current->focus == menu;
    for (entry_index = 0; menu->data.list.items[entry_index].text != 0; entry_index++) {
        if (focused && menu->data.list.selected == entry_index) {
            menu->data.list.items[entry_index].fade_timer =
                menu->data.list.items[entry_index].fade_timer + 1;
            if (menu->data.list.items[entry_index].fade_timer > 0x7D00) {
                menu->data.list.items[entry_index].fade_timer = 0x7D00;
            }
        } else {
            fade_timer = menu->data.list.items[entry_index].fade_timer;
            if (func_001F98C0_1BB90(D_001602B4_1BB90) < fade_timer) {
                menu->data.list.items[entry_index].fade_timer =
                    func_001F98C0_1BB90(D_001602B4_1BB90);
            }
            menu->data.list.items[entry_index].fade_timer =
                menu->data.list.items[entry_index].fade_timer > 0
                    ? menu->data.list.items[entry_index].fade_timer - 1
                    : 0;
        }
    }
    if (!focused) {
        return 0;
    }
    if (D_0013CA40_1BB90.pressed_unmasked & 0xD00) {
        if (menu->data.list.flags & 0x20) {
            D_001A0414_1BB90[0] = D_0015EE84_1BB90;
        }
        return -1;
    }
    if (D_0013CA40_1BB90.pressed_unmasked & 0x10) {
        if (menu->data.list.flags & 0x20) {
            D_001A0414_1BB90[0] = D_0015EE84_1BB90;
        }
        if (D_001D5F70_1BB90.current->back != 0) {
            D_001D5F70_1BB90.next = D_001D5F70_1BB90.current->back;
            return 0;
        } else if (D_001D5F70_1BB90.close_locked == 0) {
            return -1;
        }
    }
    if ((D_0013CA40_1BB90.pressed_unmasked & 0x40) && ((D_0015EFB4_1A1A0 ^ 1) & 1)) {
        /* Index through menu-> directly: locals for items/selected change
           which register keeps the copy of selected that actions 6 and 9 reuse. */
        switch (menu->data.list.items[menu->data.list.selected].action) {
        case 0:
            break;
        /* No case 1 in PAL: retail's table sends action 1 to the default. */
        case 3:
            D_001D5F70_1BB90.next =
                (struct MenuPage_1BB90 *)menu->data.list.items[menu->data.list.selected].param.value;
            break;
        case 4:
            func_0022ED80_1BB90(0, 0x11, menu->moby);
            D_001D5F70_1BB90.unk150 = 0;
            D_0015EFB4_1A1A0 = (D_0015EFB4_1A1A0 | 2) & ~4;
            func_001FBC80_1BB90(3, menu->data.list.items[menu->data.list.selected].param.value, 0);
            break;
        case 5:
            func_0022ED80_1BB90(0, 0x11, menu->moby);
            D_0015EFB4_1A1A0 = (D_0015EFB4_1A1A0 | 4) & ~2;
            func_001FBC80_1BB90(3, menu->data.list.items[menu->data.list.selected].param.value, 0);
            D_001D5F70_1BB90.unk150 = 1;
            break;
        case 6:
            /* This action passes the low halfword; the other indexed actions pass the full word. */
            message_index = menu->data.list.items[menu->data.list.selected].param.half.hi;
            if (message_index != 0) {
                D_001D5F70_1BB90.unkEC = D_00199578_1BB90[message_index];
            }
            D_001D5F70_1BB90.close_request = 5;
            D_001D5F70_1BB90.unkF0 = D_001D5F70_1BB90.current;
            D_001D5F70_1BB90.unkF4 = 0;
            D_001D5F70_1BB90.unkE4 =
                menu->data.list.items[menu->data.list.selected].param.half.lo;
            func_0022ED80_1BB90(0, 0x11, menu->moby);
            return 0;
        case 7:
            D_001D5F70_1BB90.unkF4 = 2;
            D_001D5F70_1BB90.unkF0 = D_001D5F70_1BB90.current;
            D_001D5F70_1BB90.close_request = 3;
            D_001D5F70_1BB90.unkE4 = menu->data.list.items[menu->data.list.selected].param.value;
            func_0022ED80_1BB90(0, 0x11, menu->moby);
            return 0;
        case 8:
            D_001D5F70_1BB90.unkF4 = 2;
            D_001D5F70_1BB90.unkF0 = D_001D5F70_1BB90.current;
            D_001D5F70_1BB90.close_request = 4;
            D_001D5F70_1BB90.unkE4 = menu->data.list.items[menu->data.list.selected].param.value;
            func_0022ED80_1BB90(0, 0x11, menu->moby);
            return 0;
        case 10:
            D_001D5F70_1BB90.unkF4 = 2;
            D_001D5F70_1BB90.unkF0 = D_001D5F70_1BB90.current;
            D_001D5F70_1BB90.close_request = 6;
            D_001D5F70_1BB90.unkE4 = menu->data.list.items[menu->data.list.selected].param.value;
            func_0022ED80_1BB90(0, 0x11, menu->moby);
            return 0;
        case 11:
            D_001D5F70_1BB90.unkF0 = D_001D5F70_1BB90.current;
            D_001D5F70_1BB90.unkF4 = 2;
            D_001D5F70_1BB90.close_request = 7;
            func_0022ED80_1BB90(0, 0x11, menu->moby);
            return 0;
        case 9:
            D_0015EE88_1BB90 = menu->data.list.items[menu->data.list.selected].param.value;
            return 0;
        case 2:
            func_0022ED80_1BB90(2, 0x11, menu->moby);
            break;
        }
    }
    /* Count first, then read selected/flags: in the other order (or as a
       do-while) the allocator shifts every register of the tail. */
    for (entry_count = 0; menu->data.list.items[entry_count].text != 0; entry_count++) {
    }
    previous_selection = menu->data.list.selected;
    flags = menu->data.list.flags;
    if (flags & 1) {
        buttons = D_0013CA40_1BB90.raw_pressed;
    } else {
        buttons = D_0013CA40_1BB90.pressed_unmasked;
    }
    if ((buttons & 0x1000) || ((flags & 0x100) && (buttons & 4))) {
        if (menu->data.list.selected != 0) {
            menu->data.list.selected--;
        } else if (flags & 0x1000) {
            menu->data.list.selected = entry_count - 1;
        } else {
            D_001D5F70_1BB90.current->pending_focus = menu->data.list.up;
        }
    }
    if ((buttons & 0x4000) || ((menu->data.list.flags & 0x100) && (buttons & 8))) {
        if (menu->data.list.items[menu->data.list.selected + 1].text != 0 &&
            menu->data.list.items[menu->data.list.selected + 1].action != 0) {
            menu->data.list.selected++;
        } else if (menu->data.list.flags & 0x1000) {
            menu->data.list.selected = 0;
        } else {
            D_001D5F70_1BB90.current->pending_focus = menu->data.list.down;
        }
    }
    if (menu->data.list.selected != previous_selection || D_001D5F70_1BB90.current->pending_focus != 0) {
        func_0022ED80_1BB90(1, 0x11, menu->moby);
        if (menu->data.list.flags & 0x20) {
            D_001A0414_1BB90[0] = D_001602E0_1BB90[menu->data.list.selected];
        }
    }
    return 0;
}

typedef struct {
    s16 text_id;
    s16 enabled;
    s32 action_value;
    s16 secondary_text_id;
    s16 fade_timer;
} MenuItem_1C1B0;
typedef struct {
    u8 pad0[0x20];
    s32 width;
    s32 height;
    u8 pad28[8];
    s32 flags;
    MenuItem_1C1B0 *items;
    u8 pad38[8];
    s32 selected_entry;
} MenuDescriptor;
typedef struct {
    u8 pad0[0x40];
    MenuDescriptor *focus;
} MenuPage;
extern MenuPage * D_001D5F74_1C1B0[] __asm__("D_001D5F74");
extern u8 D_001DF3D0[];
extern u8 D_001DF770[];
extern u8 D_001DFB10[];
extern short D_001602B8;
extern short D_001602BC;
extern void func_00234C98_1C1B0(s32, long) __asm__("func_00234C98");
extern void func_001F4630(s32);
extern s32 func_001F4868(s32);
extern void func_001F6598(void);
extern void func_001F65A8(void);
extern s32 func_001F65B0_1C1B0(char *, s32, u8 *) __asm__("func_001F65B0");
extern void func_001F6668_1C1B0(s32, s32, long, char *, s32, s32, u8 *) __asm__("func_001F6668");
extern char *func_001FE540_1C1B0(s32) __asm__("func_001FE540");
extern s32 func_0021C6C0(s32, s32, s32);

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/textbin/render_localized_ui_entry_list.c, render_localized_ui_entry_list. */
s32 func_0021C1B0(MenuDescriptor *menu) {
    s32 focused;
    u8 *glyphs;
    s32 font_texture_index;
    s32 row_height;
    s32 half_width;
    s32 maximum_text_width;
    s32 selection_index;
    s32 font_height;
    s32 entry_count;
    MenuItem_1C1B0 *entry;
    MenuItem_1C1B0 *scan_entry;
    s32 entry_index;
    s32 selected_entry;
    s32 enabled;
    s32 color;
    char *text_id;
    char *secondary_text;
    s32 x;
    s32 y;
    s32 text_width;
    s32 flags;
    s32 menu_width;
    MenuItem_1C1B0 *selected_item;
    s32 shadow_y;
    s32 shadow_x;
    font_height = 12;
    font_texture_index = 1;
    glyphs = D_001DF3D0;
    focused = D_001D5F74_1C1B0[0]->focus == menu;
    if (menu->flags & 4) {
        font_height = 14;
        font_texture_index = 3;
        glyphs = D_001DFB10;
    }
    if (menu->flags & 8) {
        font_height = 10;
        font_texture_index = 2;
        glyphs = D_001DF770;
    }
    func_00234C98_1C1B0(0x42, 0x44);
    func_00234C98_1C1B0(0x47, 0x2004B);
    SetupGifPaging(0);
    entry_count = 0;
    entry = menu->items;
    while (entry->text_id != 0) {
        entry++;
        entry_count++;
    }

    if (menu->flags & 0x10) {
        row_height = font_height + 3;
    } else {
        row_height = menu->height / (entry_count + 1);
    }
    half_width = menu->width >> 1;
    maximum_text_width = 0;
    y = (row_height - (font_height / 2)) - 1;
    if (menu->flags & 0x4000) {
        scan_entry = menu->items;
        if (scan_entry[0].text_id != 0) {
            entry_index = 0;
            selection_index = 0;
            do {
                text_width = func_001F65B0_1C1B0(
                    func_001FE540_1C1B0(scan_entry[entry_index].text_id), -1, glyphs);
                entry_index++;
                selection_index++;
                maximum_text_width =
                    (maximum_text_width < text_width) ? (text_width) : (maximum_text_width);
                scan_entry = menu->items;
            } while (menu->items[selection_index].text_id != 0);
        }
    }
    flags = menu->flags;
    menu_width = menu->width;
    if (flags & 0x20000) {
        if (menu_width < (maximum_text_width + 6)) {
            if (!(flags & 8)) {
                menu->flags = flags | 8;
                return 1;
            }
        }
    }
    entry_index = (selection_index = 0);
    maximum_text_width = (menu_width < maximum_text_width) ? (menu_width) : (maximum_text_width);
    if (menu->items[0].text_id != 0) {
        do {
            selected_entry = 0;
            if (focused) {
                selected_entry = menu->selected_entry == selection_index;
            }
            selected_item = (MenuItem_1C1B0 *)((u32)(entry_index * sizeof(MenuItem_1C1B0)) + (u32)menu->items);
            enabled = selected_item->enabled != 0;
            if (menu->flags & 2) {
                color = 0x80FFA888;
            } else if (selected_entry) {
                if (enabled) {
                    goto fade_timer;
                }
                color = 0x80006060;
            } else if (enabled) {
            fade_timer:
                color = func_0021C6C0(selected_item->fade_timer, -1, -1);

            } else {
                color = 0x80303030;
            }
            text_id = func_001FE540_1C1B0(menu->items[entry_index].text_id);
            if (menu->items[entry_index].enabled == 2) {
                text_id = func_001FE540_1C1B0(0x4F54);
            }
            x = half_width - (func_001F65B0_1C1B0(text_id, -1, glyphs) >> 1);
            if (menu->flags & 0x40) {
                x = 4;
            } else if (menu->flags & 0x4000) {
                x = half_width - (maximum_text_width >> 1);
            }
            func_001F65A8();
            func_001F6668_1C1B0(x + (*(s32 *)&D_001602B8), y + (*(s32 *)&D_001602BC), 0x80000000L, text_id, -1,
                       GetEffectTex(font_texture_index), glyphs);
            func_001F6598();
            if (menu->flags & 0x80) {
                func_001F65A8();
            }
            func_001F6668_1C1B0(x, y, color, text_id, -1, GetEffectTex(font_texture_index), glyphs);
            y += row_height;
            if (menu->items[entry_index].secondary_text_id != 0) {
                func_001F65A8();
                shadow_x = x + (*(s32 *)&D_001602B8);
                shadow_y = y + (*(s32 *)&D_001602BC);
                secondary_text = func_001FE540_1C1B0(menu->items[entry_index].secondary_text_id);
                func_001F6668_1C1B0(shadow_x, shadow_y, 0x80000000L, secondary_text, -1,
                           GetEffectTex(font_texture_index), glyphs);
                if (!(menu->flags & 0x80)) {
                    func_001F6598();
                }
                func_001F6668_1C1B0(x, y, color,
                           func_001FE540_1C1B0(menu->items[entry_index].secondary_text_id), -1,
                           GetEffectTex(font_texture_index), glyphs);
                y += row_height;
            }
            if (menu->flags & 0x80) {
                func_001F6598();
            }
            entry_index++;
            selection_index++;
        } while (menu->items[entry_index].text_id != 0);
    }
    DoGifPaging();
    return 2;
}

/* SDA, gp -0x6A4C: declared 2 bytes so the assembler reaches it through
   $gp, and read as the int it is. */
extern short D_001602B4;
extern int func_001FA8A8(int, int, float);

/* Fade colour for the pause menus: -1 picks the default colours
   0x80FFA888 / 0x8020FFFF and a negative start becomes 0. While the
   D_001602B4 timer (func_001F98C0) is still below start the blend factor
   is 1; after that it is 1 - (elapsed - start) / total, and the two
   colours are blended by func_001FA8A8. The parameters are updated in
   place; the timer is read again for each value. */
int func_0021C6C0(int start, int col1, int col2) {
    float t;

    if (start < 0) {
        start = 0;
    }
    if (col1 == -1) {
        col1 = 0x80FFA888;
    }
    if (col2 == -1) {
        col2 = 0x8020FFFF;
    }
    if (func_001F98C0(*(int *)&D_001602B4) >= start) {
        int a = func_001F98C0(*(int *)&D_001602B4);
        t = 1.0f - (float)(a - start) / (float)func_001F98C0(*(int *)&D_001602B4);
    } else {
        t = 1.0f;
    }
    return FastTweenColor(col1, col2, t);
}

/* Pause page link walk: with arg1 clear, follow arg0's +0x4C chain for
   as long as the current page is skippable (bit 8 of its +0x30 flags
   while D_001D5F70+0x134 is set, or bit 4 while +0x138 is set), and if
   it moved at all, make the page it stopped on the current one's +0x80.
   Each block reads D_001D5F70 through its own `char *` local: the loop's
   own copy is why GCSE leaves its +0x138 load in the loop (only +0x134,
   read first in the body, is hoisted by loop.c), and the last block's
   copy is retail's kept %hi. */
int func_0021C790(char *arg0, int arg1) {
    int ok = 0;
    int found = 0;

    if (arg1 != 0) {
        return 0;
    }
    {
        char *g = D_001D5F70;
        if (*(int *)(g + 0x134) != 0 && (*(int *)(arg0 + 0x30) & 8)) {
            ok = 1;
        }
        if (*(int *)(g + 0x138) != 0 && (*(int *)(arg0 + 0x30) & 4)) {
            ok = 1;
        }
    }
    while (ok) {
        char *g = D_001D5F70;
        found = 1;
        arg0 = *(char **)(arg0 + 0x4C);
        ok = 0;
        if (*(int *)(g + 0x134) != 0 && (*(int *)(arg0 + 0x30) & 8)) {
            ok = 1;
        }
        if (*(int *)(g + 0x138) != 0 && (*(int *)(arg0 + 0x30) & 4)) {
            ok = 1;
        }
    }
    if (found) {
        char *g2 = D_001D5F70;
        *(char **)(*(char **)(g2 + 4) + 0x80) = arg0;
    }
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/text", func_0021C840);

int func_0021CD98(void) {
    return 0;
}


/* D_001D5F70 + 0xCB. Retail addresses this one byte BOTH ways -- through
   its own %hi/%lo here and as 0xCB($16) off the D_001D5F70 base a few
   instructions later -- so it really is two names on one address in the
   original source, and splat has the second as its own linker symbol. */
extern char D_001D603B[];
extern short D_001517D0[];

/*
 * Pause/menu teardown. When the D_001517D0+8 state word is clear, drop
 * the active slot: clear the 0xCB flag byte, toggle 0x1000 in that
 * slot's flag word and mark no slot active. When it is SET instead and
 * the 0xCB byte is still set, run func_00217588 first, then clear the
 * byte and invalidate the slot outright (flags = -1, not a toggle).
 *
 * The two tests read D_001517D0[4] twice rather than being one if/else,
 * and that IS the source: the indirect store into the slot table between
 * them kills gcc 2.95's memory, so retail reloads the halfword and
 * re-materialises the base. Written as if/else it collapses to a single
 * test.
 *
 * D_001A01F0 is the struct from func_00205790 above; `sel` at +0x2A0
 * sits between the flag array and the size array, which is why flags is
 * 5 entries and not 6.
 *
 * `char *g = D_001D5F70;` is load-bearing: writing the two accesses as
 * D_001D5F70[0xCB] lets gcc fold 0xCB into the symbol addend, giving
 * %hi/%lo(D_001D5F70+203) and a base register already at +0xCB. Retail
 * keeps the UNOFFSET base in $16 and puts 0xCB in both displacements.
 * Same instruction count, but it also changes what the branch delay
 * slot gets filled with, so it is a byte difference, not just cosmetic.
 */
int func_0021CDA0(void) {
    int cur;

    if (D_001517D0[4] == 0) {
        cur = D_001A01F0_slots.sel;
        if (cur != -1) {
            D_001D603B[0] = 0;
            D_001A01F0_slots.flags[cur] ^= 0x1000;
            D_001A01F0_slots.sel = -1;
        }
    }
    if (D_001517D0[4] != 0) {
        char *g = D_001D5F70;
        if (*(unsigned char *)(g + 0xCB) != 0) {
            request_audio_stream_break();
            g[0xCB] = 0;
            D_001A01F0_slots.flags[D_001A01F0_slots.sel] = -1;
            D_001A01F0_slots.sel = -1;
        }
    }
    return 0;
}

struct MapIcon_1CE60;
struct MapMarker_1CE60;
struct MapHdr_1CE60;

struct MapState_1CE60 {
    u8 pad0[0x8];
    s32 unk8;
    u8 *mask;
    u8 pad10[0x4];
    s32 unk14;
    s32 z;
    struct MapMarker_1CE60 *markers;
    struct MapIcon_1CE60 *icons;
    s32 unk24;
    s32 unk28;
    s32 unk2C;
    struct {
        s32 unk0;
        u8 pad4[0xC];
    } unk30[8];
    s32 marker_count;
    f32 zoom[20];
    s32 pan_x[20];
    s32 pan_y[20];
    u8 pad1A4[0x80];
    s32 level;
    s32 loaded;
    s32 unk22C;
    s32 unk230;
    s32 unk234;
    u8 pad238[0x4];
    struct MapHdr_1CE60 *hdr;
    s32 map_image_vram;
    s32 tex_clut_vram;
    s32 tex0_vram;
    s32 tex1_vram;
    s32 tex2_vram;
    u8 pad254[0x4];
    s64 tex0;
    s64 tex1;
    s64 tex2;
    u8 pad270[0x8];
    s32 slot[5];
    s32 slot_id[5];
    s32 sel;
    s32 slot_size[5];
};

extern struct MapState_1CE60 D_001A01F0_1CE60 __asm__("D_001A01F0");


struct MenuScreen_1CE60;

struct MenuPage_1CE60 {
    s32 moby_anims[14];
    struct MenuPage_1CE60 *back;
    s32 state;
    struct MenuScreen_1CE60 *focus;
    struct MenuScreen_1CE60 *screens[14];
    u8 pad_7C[0x4];
    struct MenuScreen_1CE60 *pending_focus;
    s32 confirmed;
};

struct MenuSystem_1CE60 {

    s32 state;
    struct MenuPage_1CE60 *current;
    struct MenuPage_1CE60 *next;
    s32 close_request;
    s32 unk10;
    s32 timer;
    s32 saved_texture_start;
    s32 current_gadget;
    u8 pad_20[0x10];
    s32 equipped[4];
    u8 pad_40[0x60];

    s32 stream_buffer[2];
    s32 unkA8;
    s32 unkAC;
    s32 unkB0[3];
    s32 read_offset;
    s32 unkC0;
    u8 pad_C4[0x4];
    u8 loaded_animation[2];
    u8 read_buffer_index;
    u8 pending_buffer;
    u32 streamed_animation_base;

    struct MenuPage_1CE60 *previous;
    s32 unkD4;
    s32 unkD8;
    s32 unkDC;
    s32 unkE0;
    s32 unkE4;
    u8 pad_E8[0x4];
    s32 unkEC;
    struct MenuPage_1CE60 *unkF0;
    s32 unkF4;
    s32 unkF8;

    s32 unkFC;
    s32 unk100;
    s32 unk104;
    s32 help_text_buffer;
    s32 unk10C;

    s32 update_count;
    u8 pad_114[0x4];

    s32 resource_table_toggle;
    s32 unk11C;
    s32 unk120;

    s32 close_locked;
    s32 card_op_pending;
    s32 card_op_text;
    u8 pad_130[0x4];
    s32 unk134;
    s32 unk138;
    s32 unk13C;
    s32 unk140;
    s32 last_resource_table_toggle;
};

extern struct MenuSystem_1CE60 D_001D5F70_1CE60 __asm__("D_001D5F70");

struct MenuScreenWords_1CE60 {
    u8 pad_30[0x4];
    s32 unk34;
    s32 unk38;
    s32 unk3C;
    u8 pad_40[0x10];
    s32 unk50;
    s32 unk54;
    u8 pad_58[0x8];
};

struct MenuScreen_1CE60 {
    s32 (*update)(struct MenuScreen_1CE60 *);
    u8 pad_4[0x4];
    void (*enter)(struct MenuScreen_1CE60 *, s32);
    void (*leave)(struct MenuScreen_1CE60 *, s32);
    s32 unk10;
    s32 moby;
    s32 x;
    s32 y;
    s32 width;
    s32 height;
    u8 pad_28[0x8];
    union {
        struct MenuScreenWords_1CE60 raw;
    } data_1CE60;
};


struct MapHdr_1CE60 {
    s32 unk0;
    s32 unk4;
    s32 unk8;
    s32 unkC;
    s32 unk10;
    s32 unk14;
    s32 unk18;
};

struct MapEntry_1CE60 {
    s32 a;
    s32 b;
};

struct Hdr16_1CE60 {
    u8 pad0[0x8];
    s16 unk8;
};

extern struct Hdr16_1CE60 D_001517D0_1CE60 __asm__("D_001517D0");
extern u8 D_001CF9F8_1CE60[] __asm__("D_001CF9F8");
extern u8 D_001CF798_1CE60[] __asm__("D_001CF798");
extern s32 *D_001602E0_1CE60 __asm__("D_001602E0") MACRO_ADDR;
extern u8 D_0013D5E9_1CE60[] __asm__("D_0013D5E9");
extern u8 D_0013DE60_1CE60[] __asm__("D_0013DE60");
extern u8 D_00141FC0_1CE60[] __asm__("D_00141FC0");
extern struct MapEntry_1CE60 D_001384A0_1CE60[] __asm__("D_001384A0");
extern struct MapEntry_1CE60 D_00138538_1CE60[] __asm__("D_00138538");

extern s32 func_00205C70_1CE60() __asm__("func_00205C70");
extern s32 func_00205A50_1CE60(s32 id) __asm__("func_00205A50");
extern s32 func_002058D0_1CE60() __asm__("func_002058D0");
extern s32 func_00205AA8_1CE60() __asm__("func_00205AA8");
extern void func_00205830_1CE60() __asm__("func_00205830");
extern s32 func_00205790_1CE60() __asm__("func_00205790");
extern s32 func_00205918_1CE60() __asm__("func_00205918");
extern void func_00205C08_1CE60() __asm__("func_00205C08");
extern void func_00206F40_1CE60() __asm__("func_00206F40");
extern s32 func_0020C7A0_1CE60() __asm__("func_0020C7A0");
extern s32 func_00226EA8_1CE60() __asm__("func_00226EA8");
extern s32 func_00226F68_1CE60() __asm__("func_00226F68");
extern s32 func_00217628_1CE60() __asm__("func_00217628");
extern s32 func_00234238_1CE60() __asm__("func_00234238");
extern s32 func_0020C468_1CE60() __asm__("func_0020C468");
extern s64 func_00205660_1CE60() __asm__("func_00205660");
extern void func_002083E0_1CE60() __asm__("func_002083E0");
extern void func_0020C2F8_1CE60() __asm__("func_0020C2F8");
extern void func_0020CDE0_1CE60() __asm__("func_0020CDE0");
extern void func_0022ED80_1CE60() __asm__("func_0022ED80");

/* Map screen of the pause menu: pad input (close, back, the mission page, warp to the shown
   level, previous/next visited level), then decodes the shown level's map into its textures
   when it changed and keeps the next map streaming into a free slot.
   Adapted from Lombyte (MIT) for PAL: src/ui/map/draw_map_screen.c, draw_map_screen. */
s32 func_0021CE60(struct MenuScreen_1CE60 *screen) {
    s32 prev;
    s32 idx;
    s32 t;
    s32 id;
    s32 slot;
    s32 n;
    struct MapHdr_1CE60 *hdr;
    u8 *pal;
    u8 *pix;
    s32 buf;
    u8 *base;
    u8 *a;
    u8 *tex;
    u8 *tbase;
    u8 *b;
    u8 *c;
    s32 pick;
    s32 next;
    s32 k;
    struct MapEntry_1CE60 *e;

    func_00205C70_1CE60();
    if (!(screen->data_1CE60.raw.unk34 & 0x40)) {
        prev = D_001A01F0_1CE60.level;
        if (D_0013CA40_1BB90.pressed_unmasked & 0xD00) {
            if (D_001D5F70_1CE60.close_locked == 0) {
                return 1;
            }
        }
        if (D_0013CA40_1BB90.pressed_unmasked & 0x10) {
            if (D_001D5F70_1CE60.current->back != 0) {
                D_001D5F70_1CE60.next = D_001D5F70_1CE60.current->back;
                return 0;
            } else if (D_001D5F70_1CE60.close_locked == 0) {
                return -1;
            }
        }
        if (D_0013CA40_1BB90.pressed_unmasked & 0x40) {
            D_001D5F70_1CE60.next = (struct MenuPage_1CE60 *)D_001CF9F8_1CE60;
        }
        if ((D_0013CA40_1BB90.pressed_unmasked & 0x20) && D_001A01F0_1CE60.level != 0) {
            D_001D5F70_1CE60.unkF0 = (struct MenuPage_1CE60 *)D_001CF798_1CE60;
            D_001D5F70_1CE60.unkF4 = 0xB;
            D_001D5F70_1CE60.close_request = 3;
            D_001D5F70_1CE60.unkE4 = D_001A01F0_1CE60.level;
            func_0022ED80_1CE60(0, 0x11, screen->moby);
            return 0;
        }
        idx = func_00205A50_1CE60(D_001A01F0_1CE60.level);
        if (idx >= 0) {
            if (D_0013CA40_1BB90.pressed_unmasked & 8) {
                if (idx < 0x13) {
                    t = D_001602E0_1CE60[idx + 1];
                    if (t != 0) {
                        D_001A01F0_1CE60.level = t;
                    }
                }
            }
            if ((D_0013CA40_1BB90.pressed_unmasked & 4) && idx != 0) {
                if (D_001602E0_1CE60[idx - 1] != 0) {
                    D_001A01F0_1CE60.level = D_001602E0_1CE60[idx - 1];
                }
            }
        }
        if (D_001A01F0_1CE60.level != prev) {
            func_0022ED80_1CE60(1, 0x11, screen->moby);
            func_0020C7A0_1CE60();
        }
    }

    id = D_001A01F0_1CE60.level;
    if (D_0013D5E9_1CE60[0] != 0) {
        id += 0x100;
    }
    slot = func_002058D0_1CE60(id);
    if (D_001A01F0_1CE60.level != D_001A01F0_1CE60.loaded && slot != -1) {
        n = func_00205AA8_1CE60();
        if (D_001A01F0_1CE60.slot[0] != 0 && n > 0) {
            func_00205830_1CE60(n, 0);
            if (slot == 0) {
                slot = n;
            }
        }
        func_0020C468_1CE60(D_001A01F0_1CE60.slot[slot], D_001A01F0_1CE60.hdr);
        hdr = D_001A01F0_1CE60.hdr;
        pix = (u8 *)hdr + hdr->unk8;
        pal = (u8 *)hdr + hdr->unkC;
        buf = func_00226EA8_1CE60(0);
        if (buf != 0) {
            base = (u8 *)D_001A01F0_1CE60.hdr;
            a = base + hdr->unk0 + 8;
            if (D_001A01F0_1CE60.level == D_0015EE84_1BB90) {
                func_00205C08_1CE60(pal, pix, pal, D_001A01F0_1CE60.mask);
            } else {
                if (D_0013DE60_1CE60[D_001A01F0_1CE60.level] != 0) {
                    func_002083E0_1CE60(buf, D_00141FC0_1CE60 + (D_001A01F0_1CE60.level << 11), base + hdr->unk4);
                } else {
                    func_00206F40_1CE60(buf, a, a, base);
                }
                func_00205C08_1CE60(pal, pix, pal, buf);
            }
        }
        if (!(screen->data_1CE60.raw.unk34 & 0x80)) {
            tbase = (u8 *)D_001A01F0_1CE60.hdr;
            tex = tbase + hdr->unk10;
            b = tbase + hdr->unk14 + 0x420;
            c = tbase + hdr->unk18 + 0x420;
            D_001A01F0_1CE60.tex0 =
                func_00205660_1CE60(7, 7, tex + 0x20, tex + 0x420, D_001A01F0_1CE60.tex_clut_vram, D_001A01F0_1CE60.tex0_vram);
            D_001A01F0_1CE60.tex1 =
                func_00205660_1CE60(7, 7, tex + 0x20, b, D_001A01F0_1CE60.tex_clut_vram, D_001A01F0_1CE60.tex1_vram);
            D_001A01F0_1CE60.tex2 =
                func_00205660_1CE60(7, 7, tex + 0x20, c, D_001A01F0_1CE60.tex_clut_vram, D_001A01F0_1CE60.tex2_vram);
            if (buf != 0) {
                func_00205660_1CE60(9, 9, pal, pal, 0x3FF000, D_001A01F0_1CE60.map_image_vram);
            }
            func_0020C2F8_1CE60();
        }
        if (buf != 0) {
            func_00226F68_1CE60(buf);
        }
        D_001A01F0_1CE60.slot[0] = (s32)pix;
        D_001A01F0_1CE60.loaded = D_001A01F0_1CE60.level;
        func_0020CDE0_1CE60(D_001A01F0_1CE60.level, 0);
    }

    if (D_001517D0_1CE60.unk8 == 0) {
        if (D_001A01F0_1CE60.sel != -1) {
            D_001D5F70_1CE60.pending_buffer = 0;
            D_001A01F0_1CE60.slot_id[D_001A01F0_1CE60.sel] ^= 0x1000;
            D_001A01F0_1CE60.sel = -1;
        }
    }
    pick = -1;
    if (D_001517D0_1CE60.unk8 == 0) {
        pick = func_00205790_1CE60();
        if (pick != -1) {
            next = func_00205918_1CE60();
            if (next != -1) {
                k = (D_0015EE84_1BB90 + D_001A01F0_1CE60.unk230 == 0) ? 0 : 0x100;
                if (k == next) {
                    func_00234238_1CE60(D_001A01F0_1CE60.slot[pick], D_001A01F0_1CE60.unk22C, 0,
                                       D_001A01F0_1CE60.unk234, 0);
                    D_001A01F0_1CE60.slot_size[pick] = D_001A01F0_1CE60.unk234;
                } else {
                    if (next & 0x100) {
                        e = &D_00138538_1CE60[next ^ 0x100];
                    } else {
                        e = &D_001384A0_1CE60[next];
                    }
                    func_00217628_1CE60(D_001A01F0_1CE60.slot[pick], e->a, e->b);
                    D_001D5F70_1CE60.pending_buffer = 1;
                    D_001A01F0_1CE60.slot_size[pick] = (e->b << 11) >> 4;
                }
                D_001A01F0_1CE60.slot_id[pick] = next | 0x1000;
                D_001A01F0_1CE60.sel = pick;
            }
        }
    }
    slot = func_002058D0_1CE60(id);
    if (D_001A01F0_1CE60.level != D_001A01F0_1CE60.loaded) {
        if (D_001517D0_1CE60.unk8 == 0 && slot == -1 && pick == slot) {
            func_00205AA8_1CE60(D_001A01F0_1CE60.loaded);
        }
    }
    return 0;
}

extern int D_001A0414;
extern int D_001CFBF4;
extern int D_001CFAD8;
extern void func_0020C7A0(void *);
extern int func_0020CA50(void *, void *, void *, int);

int func_0021D420(void *arg0) {
    int *p;

    func_0020C7A0(arg0);
    *(int *)((char *)arg0 + 0x7C) =
        func_0020CA50((void *)0x70000000, (void *)0, (void *)0x70000100, 1);
    p = (int *)((char *)arg0 + 0x30);
    p += D_001A0414;
    if (*p != -1) {
        D_001CFBF4 = ((int *)0x70000000)[*p];
        D_001CFAD8 = ((int *)0x70000100)[*p];
    }
    return 0;
}

struct Menu;
struct Owner {
    char pad0[0x38];
    int unk38;
    char pad3C[4];
    struct Menu *owner;
};
struct Menu {
    char pad0[0x14];
    int unk14;
    char pad18[0x18];
    int choice[19];
    int count;
};
typedef struct {
    char pad0[4];
    struct Owner *cur;
    int unk8;
    char padC[0x118];
    int unk124;
} PauseG;
typedef struct {
    char pad0[0x1C4];
    unsigned int pressed;
} PadG;
typedef struct {
    char pad0[0x224];
    int level;
} LevelG;
extern PauseG D_001D5F70_g __asm__("D_001D5F70") NOT_SDA;
extern PadG D_0013CA40_g __asm__("D_0013CA40");
extern LevelG D_001A01F0_lv __asm__("D_001A01F0");
extern unsigned char D_0013DE48[];

/* Pause slot-select tick. Adapted from Lombyte (MIT) for PAL: src/ui/menus/fun_0021c4c0.c, FUN_0021c4c0. */
int func_0021D4C0(struct Menu *m) {
    int old;
    int i;
    int prev;
    unsigned int pad;
    int *p;
    int *ch;

    {
        struct Menu *owner = D_001D5F70_g.cur->owner;
        old = D_001A01F0_lv.level;
        if (owner != m) {
            if (old < 20) {
                m->choice[old] = -1;
            }
            return 0;
        }
    }
    if ((D_0013CA40_g.pressed & 0xD00) && D_001D5F70_g.unk124 == 0) {
        return 1;
    }
    if (D_0013CA40_g.pressed & 8) {
        for (i = D_001A01F0_lv.level + 1; i < 20; i++) {
            if (D_0013DE48[i] != 0 || D_0015EE84 == i) {
                D_001A01F0_lv.level = i;
                break;
            }
        }
    }
    if (D_0013CA40_g.pressed & 4) {
        for (i = D_001A01F0_lv.level - 1; i >= 0; i--) {
            if (D_0013DE48[i] != 0 || D_0015EE84 == i) {
                D_001A01F0_lv.level = i;
                break;
            }
        }
    }
    if (D_001A01F0_lv.level != old) {
        func_0022ED80(1, 0x11, m->unk14);
        func_0020C7A0_i();
    }
    if (m->count != 0) {
        pad = D_0013CA40_g.pressed;
        ch = m->choice;
        prev = m->choice[D_001A01F0_lv.level];
        if (pad & 0x1000) {
            m->choice[D_001A01F0_lv.level] = (prev + m->count - 1) % m->count;
        }
        if (pad & 0x4000) {
            m->choice[D_001A01F0_lv.level] = (m->choice[D_001A01F0_lv.level] + 1) % m->count;
        }
        if (m->choice[D_001A01F0_lv.level] != prev) {
            func_0022ED80(1, 0x11, m->unk14);
        }
        if ((pad & 0x5000) || D_001A01F0_lv.level != old) {
            m->count = func_0020CA50((void *)0x70000000, (void *)0, (void *)0x70000100, 1);
            p = &ch[D_001A01F0_lv.level];
            D_001CFBF4 = ((unsigned int *)0x70000000)[*p];
            D_001CFAD8 = ((unsigned int *)0x70000100)[*p];
        }
    }
    if (D_0013CA40_g.pressed & 0x10) {
        if (D_001D5F70_g.cur->unk38 != 0) {
            D_001D5F70_g.unk8 = D_001D5F70_g.cur->unk38;
        } else if (D_001D5F70_g.unk124 == 0) {
            return -1;
        }
    }
    return 0;
}

extern char *D_001D5F74 NOT_SDA;
extern int D_0015EFA4 MACRO_ADDR;
extern void func_0022ED80(int, int, int);
typedef struct {
    char pad[0xA8];
    unsigned short count;  /* 0xA8 */
    unsigned short best;   /* 0xAA */
    unsigned int mask;     /* 0xAC */
} Stats_0021D7A0;
extern Stats_0021D7A0 D_00141948;
typedef struct {
    char pad[0x14];
    int id;      /* 0x14 */
    char pad18[0x18];
    int list[8]; /* 0x30 */
    int idx;     /* 0x50 */
} SelList_0021D7A0;

/* Quick-select slot handler: 8 and 4 on the pad step the slot index at
   +0x50 forward and back (mod 8), with a func_0022ED80 notify when it
   moved. With 0x40, if the highlighted item (the current page's entry
   id) is owned, it is recorded: the use count at D_00141948+0xA8 (to
   0xFFFF), the best time +0xAA and the level mask +0xAC; then the item
   leaves any slot it held and is put in the current slot, and the index
   advances. The three conditions are one `&&` (one shared exit) and the
   time is compared call-first, as retail evaluates them. */
int func_0021D7A0(SelList_0021D7A0 *arg0) {
    char *q = *(char **)(D_001D5F74 + 0x40);
    int item = *(short *)(*(int *)(q + 0x3C) * 10 + *(char **)(q + 0x48) + 6);
    int old = arg0->idx;
    char *pad = D_0013CA40;
    int i;

    if (*(int *)(pad + 0x1C4) & 8) {
        arg0->idx = (old + 1) % 8;
    }
    if (*(int *)(pad + 0x1C4) & 4) {
        arg0->idx = (arg0->idx + 7) % 8;
    }
    if (arg0->idx != old) {
        func_0022ED80(1, 0x11, arg0->id);
    }
    if (item != 0 && ((unsigned char *)&D_0013D5C8)[item] != 0
        && (*(int *)(pad + 0x1C4) & 0x40)) {
        Stats_0021D7A0 *s = &D_00141948;

        if (s->count <= 0xFFFE) {
            s->count++;
        }
        if (func_001F98C0(D_0015EFA4) / 600 > s->best) {
            s->best = func_001F98C0(D_0015EFA4) / 600;
        }
        s->mask = s->mask | (1 << D_0015EE84) | 0x80000000;
        for (i = 0; i < 8; i++) {
            if (arg0->list[i] == item) {
                break;
            }
        }
        if (i < 8) {
            arg0->list[i] = 0;
        }
        arg0->list[arg0->idx] = item;
        arg0->idx = (arg0->idx + 1) % 8;
    }
    return 0;
}

extern int D_00141FA0[];

typedef struct {
    char pad[0x30];
    int list[8]; /* 0x30 */
    int idx;     /* 0x50 */
} SelList_0021D9C8;

/* Reload the 8-entry list at +0x30 from D_00141FA0 (the reverse of
   func_0021DA60 below), then leave idx at +0x50 on the first empty entry,
   wrapped to 0..7. Indexing arg0->list directly in both loops (no local
   list pointer) is what gives retail's hoisted base; the scan is a do-while
   guarded by list[0] (a plain while/for rotated differently). */
int func_0021D9C8(SelList_0021D9C8 *arg0) {
    int i;
    for (i = 0; i < 8; i++) {
        arg0->list[i] = D_00141FA0[i];
    }
    arg0->idx = 0;
    if (arg0->list[0] != 0) {
        do {
            arg0->idx++;
        } while (arg0->list[arg0->idx] != 0 && arg0->idx < 8);
    }
    arg0->idx %= 8;
    return 0;
}

extern int D_00141FA0[];

int func_0021DA60(void *arg0) {
    int *src = (int *)((char *)arg0 + 0x30);
    int *dst = D_00141FA0;
    int i = 7;
    do {
        *dst++ = *src++;
    } while (--i >= 0);
    return 0;
}

extern char D_001D0A50[];
extern char D_001D0A88[];

int func_0021DA98(void *arg0) {
    *(char **)((char *)arg0 + 0x34) =
        (gHaveHeliPack != 0) ? D_001D0A50 : D_001D0A88;
    return 0;
}

extern int D_001A0418 NOT_SDA;

int func_0021DAC8(void) {
    D_001A0418 = -1;
    return 0;
}


int func_0021DAE0(void) {
    func_00226D50(1);
    return 0;
}

extern int D_0015EEF0 MACRO_ADDR;
extern int D_0013E6A0;

int func_0021DB00(void) {
    D_0013E6A0 = (D_0015EEF0 * 8) / 10;
    return 0;
}

struct MenuScreen_1DB30;

struct MenuPage_1DB30 {
    s32 moby_anims[14];
    struct MenuPage_1DB30 *back;
    s32 state;
    struct MenuScreen_1DB30 *focus;
    struct MenuScreen_1DB30 *screens[14];
    u8 pad_7C[0x4];
    struct MenuScreen_1DB30 *pending_focus;
    s32 confirmed;
};

struct MenuSystem_1DB30 {

    s32 state;
    struct MenuPage_1DB30 *current;
    struct MenuPage_1DB30 *next;
    s32 close_request;
    s32 unk10;
    s32 timer;
    s32 saved_texture_start;
    s32 current_gadget;
    u8 pad_20[0x10];
    s32 equipped[4];
    u8 pad_40[0x60];

    s32 stream_buffer[2];
    s32 unkA8;
    s32 unkAC;
    s32 unkB0[3];
    s32 read_offset;
    s32 unkC0;
    u8 pad_C4[0x4];
    u8 loaded_animation[2];
    u8 read_buffer_index;
    u8 pending_buffer;
    u32 streamed_animation_base;

    struct MenuPage_1DB30 *previous;
    s32 unkD4;
    s32 unkD8;
    s32 unkDC;
    s32 unkE0;
    s32 unkE4;
    u8 pad_E8[0x4];
    s32 unkEC;
    struct MenuPage_1DB30 *unkF0;
    s32 unkF4;
    s32 unkF8;

    s32 unkFC;
    s32 unk100;
    s32 unk104;
    s32 help_text_buffer;
    s32 unk10C;

    s32 update_count;
    u8 pad_114[0x4];

    s32 resource_table_toggle;
    s32 unk11C;
    s32 unk120;

    s32 close_locked;
    s32 card_op_pending;
    s32 card_op_text;
    u8 pad_130[0x4];
    s32 unk134;
    s32 unk138;
    s32 unk13C;
    s32 unk140;
    s32 last_resource_table_toggle;
};

extern struct MenuSystem_1DB30 D_001D5F70_1DB30 __asm__("D_001D5F70");

struct SoundMenuInput_1DB30 {
    u8 pad_0[0x1C0];
    s32 held_buttons;
    s32 pressed_buttons;
};

struct SoundMenuMixer_1DB30 {
    u8 pad_0[0x48];
    s32 group_0_volume;
    s32 group_1_volume;
    s32 group_2_volume;
    s32 group_3_volume;
    s32 group_4_volume;
    s32 group_5_volume;
};

struct SoundMenu_1DB30 {
    u8 pad_0[0x14];
    s32 sound_owner;
    u8 pad_18[0x18];
    s32 flags;
    u8 pad_34[0xC];
    s32 selected_option;
};

extern struct SoundMenuInput_1DB30 D_0013CA40_1DB30 __asm__("D_0013CA40");
extern struct SoundMenuMixer_1DB30 D_0013E650_1DB30 __asm__("D_0013E650");
extern s32 D_001D6094_1DB30 __asm__("D_001D6094") NOT_SDA;
extern s32 D_0015EEEC_1DB30 __asm__("D_0015EEEC") MACRO_ADDR;
extern s32 D_0015EEF0_1DB30 __asm__("D_0015EEF0") MACRO_ADDR;
extern s32 D_0015EEE8_1DB30 __asm__("D_0015EEE8") MACRO_ADDR;
extern s32 D_001A0414_1DB30 __asm__("D_001A0414") NOT_SDA;
extern s32 func_0022ED80_1DB30(s32 flags, s32 sound_index,
                                           s32 sound_owner) __asm__("func_0022ED80");
void func_0012E380_1DB30(s32 menu) __asm__("func_0012E380");

/* Sound options page input: close and back, up/down over the three rows, left/right change
   the sound or music volume by 3 (0..0x400) and push the new levels to the mixer groups,
   confirm on the third row toggles mono/stereo.
   Adapted from Lombyte (MIT) for PAL: src/textbin/ui/menus/audio/sound_options_menu.c, sound_options_menu. */
s32 func_0021DB30(struct SoundMenu_1DB30 *menu) {
    s32 previous_selection;
    s32 *sound_volume_ptr;
    s32 previous_music_volume;
    s32 previous_sound_volume;
    s32 scaled_sound_volume;

    if ((D_0013CA40_1DB30.pressed_buttons & 0xD00) && (D_001D6094_1DB30 == 0)) {
        return 1;
    }
    if (D_0013CA40_1DB30.pressed_buttons & 0x10) {
        struct MenuPage_1DB30 *back_page = D_001D5F70_1DB30.current->back;

        if (back_page != 0) {
            D_001D5F70_1DB30.next = back_page;
            return 0;
        } else if (D_001D5F70_1DB30.close_locked == 0) {
            return -1;
        }
    }
    previous_selection = menu->selected_option;
    if (D_0013CA40_1DB30.pressed_buttons & 0x1000) {
        menu->selected_option = (previous_selection + 2) % 3;
    }
    if (D_0013CA40_1DB30.pressed_buttons & 0x4000) {
        menu->selected_option = (menu->selected_option + 1) % 3;
    }
    if ((menu->selected_option != previous_selection) || (D_001D5F70_1DB30.current->pending_focus != 0)) {
        func_0022ED80_1DB30(1, 0x11, menu->sound_owner);
        if (menu->flags & 0x20) {
            D_001A0414_1DB30 = D_001602E0_1CE60[menu->selected_option];
        }
    }
    previous_music_volume = D_0015EEEC_1DB30;
    previous_sound_volume = D_0015EEF0_1DB30;
    sound_volume_ptr = &D_0015EEF0_1DB30;
    if (D_0013CA40_1DB30.held_buttons & 0x2000) {
        if (menu->selected_option == 0) {
            D_0015EEF0_1DB30 = (previous_sound_volume + 3 < 0x401) ? previous_sound_volume + 3 : 0x400;
        }
        if (menu->selected_option == 1) {
            D_0015EEEC_1DB30 = (previous_music_volume + 3 < 0x401) ? previous_music_volume + 3 : 0x400;
        }
    }
    if (D_0013CA40_1DB30.held_buttons & 0x8000) {
        if (menu->selected_option == 0) {
            D_0015EEF0_1DB30 = (D_0015EEF0_1DB30 - 3 <= 0) ? 0 : D_0015EEF0_1DB30 - 3;
        }
        if (menu->selected_option == 1) {
            D_0015EEEC_1DB30 = (D_0015EEEC_1DB30 - 3 <= 0) ? 0 : D_0015EEEC_1DB30 - 3;
        }
    }
    if ((previous_music_volume != D_0015EEEC_1DB30) || (previous_sound_volume != *sound_volume_ptr)) {
        D_0013E650_1DB30.group_0_volume = *sound_volume_ptr * 8 / 10;
        D_0013E650_1DB30.group_2_volume = D_0013E650_1DB30.group_1_volume = D_0015EEEC_1DB30;
        D_0013E650_1DB30.group_3_volume = *sound_volume_ptr * 7 / 10;
        scaled_sound_volume = *sound_volume_ptr * 7 / 10;
        D_0013E650_1DB30.group_4_volume = scaled_sound_volume;
        D_0013E650_1DB30.group_5_volume = *sound_volume_ptr;
    }
    if (D_0013CA40_1DB30.pressed_buttons & 0x40) {
        if (menu->selected_option == 2) {
            D_0015EEE8_1DB30 = !D_0015EEE8_1DB30;
        }
        func_0012E380_1DB30(!D_0015EEE8_1DB30);
        func_0022ED80_1DB30(0, 0x11, menu->sound_owner);
    }
    return 0;
}

typedef struct {
    u8 pad0[0x20];
    s32 width;
    s32 height;
    u8 pad28[0x18];
    s32 selected_option;
} SoundMenu_1DE08;


extern void func_001F4630_1DE08(s32) __asm__("func_001F4630");
extern void func_001F4748_1DE08(void) __asm__("func_001F4748");
extern void *func_001FE540_1DE08(s32) __asm__("func_001FE540");
extern void func_001F6CF8_1DE08(s32, s32, u64, void *, s32) __asm__("func_001F6CF8");
extern void func_001F68E8_1DE08(s32, s32, u64, void *, s32) __asm__("func_001F68E8");
extern s32 func_00200198_1DE08(s32, s32) __asm__("func_00200198");
extern void func_00201190_1DE08(s32, s32, s32, s32, s32, s32, s32, s32, s32,
                                s32) __asm__("func_00201190");
extern void func_00201640_1DE08(s32, s32, s32, s32, u64, s32) __asm__("func_00201640");

/* Draws the sound options page: the sound and music volume rows, each a right-aligned label
   and a slider bar, then the playback mode row; the selected row's label is yellow.
   Adapted from Lombyte (MIT) for PAL: src/textbin/ui/menus/audio/draw_sound_menu.c, draw_sound_menu. */
s32 func_0021DE08(SoundMenu_1DE08 *menu) {
    s32 center_x;
    s32 row_height;
    s32 label_right;
    s32 border_left;
    s32 slider_left;
    s32 slider_padding;
    s32 slider_length;
    s32 second_row;

    row_height = menu->height >> 2;
    center_x = menu->width >> 1;
    func_001F4630_1DE08(0);
    label_right = center_x - 8;
    border_left = center_x + 7;
    slider_left = center_x + 9;
    slider_padding = center_x + 0x4A;

    func_001F6CF8_1DE08(label_right, row_height - 8,
                        menu->selected_option == 0 ? 0x8020FFFF : 0x80FFA888,
                        func_001FE540_1DE08(0x5217), -1);
    func_00201640_1DE08(border_left, row_height - 8, menu->width - 0x3F, row_height + 8,
                        0x80696969, 0);
    func_00201640_1DE08(slider_left, row_height - 6, menu->width - 0x41, row_height + 6,
                        0x80383838, 0);
    slider_length = (menu->width - slider_padding) * D_0015EEF0_1DB30 / 1024;
    func_00201190_1DE08(func_00200198_1DE08(0xE99E, 8), slider_left << 4,
                        (row_height - 6) << 4, (center_x + 8 + slider_length) << 4,
                        (row_height + 5) << 4, 0, 0xA0, 0x1F0, 0x150, 0x80);

    second_row = row_height + row_height;
    func_001F6CF8_1DE08(label_right, second_row - 8,
                        menu->selected_option == 1 ? 0x8020FFFF : 0x80FFA888,
                        func_001FE540_1DE08(0x5218), -1);
    func_00201640_1DE08(border_left, second_row - 8, menu->width - 0x3F, second_row + 8,
                        0x80696969, 0);
    func_00201640_1DE08(slider_left, second_row - 6, menu->width - 0x41, second_row + 6,
                        0x80383838, 0);
    slider_length = (menu->width - slider_padding) * D_0015EEEC_1DB30 / 1024;
    func_00201190_1DE08(func_00200198_1DE08(0xE99E, 9), slider_left << 4,
                        (second_row - 6) << 4, (center_x + 8 + slider_length) << 4,
                        (second_row + 5) << 4, 0, 0xA0, 0x1F0, 0x150, 0x80);

    second_row += row_height;
    func_001F6CF8_1DE08(label_right, second_row - 8,
                        menu->selected_option == 2 ? 0x8020FFFF : 0x80FFA888,
                        func_001FE540_1DE08(0x5219), -1);
    func_001F68E8_1DE08(center_x + 8, second_row - 8, 0x80FFA888,
                        func_001FE540_1DE08(D_0015EEE8_1DB30 != 0 ? 0x521B : 0x521A), -1);
    func_001F4748_1DE08();
    return 2;
}

typedef struct {
    unsigned short v;
    short pad;
} Half4;
typedef struct {
    unsigned short a;
    short b;
    char pad[8];
} Rec0C;
extern int D_0015EF30 MACRO_ADDR;
extern Rec0C D_001CFFC0[];
extern unsigned char D_00141F08[];
extern Half4 D_00199812[];

int func_0021E170(void) {
    int i;
    for (i = 0; i < D_0015EF30;) {
        int j = i + 1;
        unsigned char k;
        D_001CFFC0[i].b = 1;
        k = D_00141F08[D_0015EF30 - j];
        D_001CFFC0[i].a = D_00199812[k].v;
        i = j;
    }
    D_001CFFC0[D_0015EF30].a = 0;
    return 0;
}

int func_0021E1F8(void) {
    return 0;
}

typedef struct {
    int key;
    int flags;
} PadBind;

/* Aliased rather than renamed: func_00227018 further down still walks
   the same table as a flat int array. */
extern PadBind D_001D6448_t[] __asm__("D_001D6448");

extern int D_001D6078;
extern int D_00137C80[];
extern int func_00217628_3(int, int, int) __asm__("func_00217628");

int func_0021E200(char *arg0) {
    int i;
    char *g;
    func_00226D50(1);
    *(int *)(arg0 + 0x54) = 0;
    *(int *)(arg0 + 0x38) = 0;
    g = D_001D5F70;
    for (i = 0; i < 5; i++) {
        if (D_001D6448_t[i].key != 0
            && (unsigned int)D_001D6448_t[i].key
                   < *(unsigned int *)(g + 0x10C)) {
            D_001D6448_t[i].flags |= 2;
        }
    }
    *(int *)(arg0 + 0x50) = 0;
    if (D_001517D0[4] == 0) {
        if (func_00217628_3(D_001D6078, D_00137C80[0x1528 / 4],
                            D_00137C80[0x152C / 4]) != 0) {
            *(int *)(arg0 + 0x50) = 1;
        } else {
            *(int *)(arg0 + 0x50) = 3;
        }
    }
    *(int *)(arg0 + 0x10) |= 4;
    return 0;
}

/* D_0015F780 is SDA elsewhere; pause.cpp stores it through $at. */
extern int D_0015F780_m __asm__("D_0015F780") MACRO_ADDR;
extern int D_001997FC;

int func_0021E2D0(int arg0) {
    if (D_001517D0[4] != 0 && *(int *)(arg0 + 0x50) == 1) {
        request_audio_stream_break();
    }
    func_00226D50(1);
    {
        int v = *(int *)(arg0 + 0x54);
        if (v != 0) {
            int w = *(int *)(arg0 + 0x38);
            D_0015F780_m = v;
            D_001997FC = w;
        }
    }
    return 0;
}

extern int D_0015EE88 MACRO_ADDR;
extern char D_001997D0[];
extern void func_001F9A00(void *, void *, int);

/* The slot count at D_001997D0+0x2C, read through its own base pointer:
   retail's loop keeps a separate copy of the D_001997D0 address. */
static inline int pauseSlotCount(void) {
    char *b = D_001997D0;
    return *(int *)(b + 0x2C);
}

/* Pause sub-state machine at arg0+0x50 (func_00221688's family). State 0
   runs func_00217628 and moves to 1 or 3; state 1, once D_001517D0[4]
   clears, pulls the controller's record out of the table at
   D_001D5F70+0x108 (func_001F9A00 moves it to the table start), swaps it
   in as D_0015F780 and the D_001997D0+0x2C count (old values kept in
   arg0+0x54/+0x38), rebases the entries' first words and moves to 2.
   Cases 2 and 3 are empty; they make gcc's case tree test 1 first, as
   retail does. The reloaded table gets its own variable so that the
   first one stays block-local, and the rebase delta is computed in the
   loop so that loop.c hoists it. */
int func_0021E340(char *arg0) {
    switch (*(int *)(arg0 + 0x50)) {
    case 0:
        if (D_001517D0[4] == 0) {
            if (func_00217628_3(D_001D6078, D_00137C80[0x1528 / 4],
                                D_00137C80[0x152C / 4]) != 0) {
                *(int *)(arg0 + 0x50) = 1;
            } else {
                *(int *)(arg0 + 0x50) = 3;
            }
        }
        break;
    case 1:
        if (D_001517D0[4] == 0) {
            char *g = D_001D5F70;
            int *tbl = *(int **)(g + 0x108);
            int *p = (int *)((char *)tbl + tbl[D_0015EE88]);
            int n = *p++;
            int sz = *p++;
            char *b;
            int *t;
            int i;

            func_001F9A00(tbl, p, ((sz + 3) & ~3) - 8);
            b = D_001997D0;
            *(int *)(arg0 + 0x54) = D_0015F780_m;
            *(int *)(arg0 + 0x38) = *(int *)(b + 0x2C);
            *(int *)(b + 0x2C) = n;
            t = *(int **)(g + 0x108);
            D_0015F780_m = (int)t;
            for (i = 0; i < pauseSlotCount(); i++) {
                int d = (int)t - 8;
                *(int *)((char *)t + i * 0x10) += d;
            }
            *(int *)(arg0 + 0x10) &= ~4;
            *(int *)(arg0 + 0x50) = 2;
        }
        break;
    case 2:
    case 3:
        break;
    }
    return 0;
}

extern void func_001F7070_a(void *, long, char *, int, int, unsigned char *) __asm__("func_001F7070");
extern void func_00234C98_l(int, long) __asm__("func_00234C98");
extern void *func_001FE540_id(int) __asm__("func_001FE540");
extern void func_001153FC(void *, int, int); /* memset */
extern void func_001F65A8(void);
extern void func_001F6598(void);
extern void func_002208F8(int x, int y, int flag);

typedef struct {
    short text;
    short enabled;
    int id;
    short subtext;
    short pad0A;
} MenuItem;

typedef struct {
    char pad00[0x20];
    int width;
    int height;
    char pad28[8];
    int flags;
    MenuItem *items;
    char pad38[8];
    int sel;
    int scroll;
} Menu;

typedef struct {
    short s[12];
} MenuBox;

/* Draws a pause-menu list (rows of text with optional subtext, cursor highlight, scrolling). Adapted from Lombyte (MIT) for PAL: ui/menus/fun_0021d4a8.c, FUN_0021d4a8. */
int func_0021E4B0(Menu *menu) {
    int size;
    int kind;
    int focused;
    unsigned char *font;
    int n;
    MenuItem *p;
    int rowh;
    int glyphs;
    int i;
    int sel;
    int en;
    int color;
    char *text;
    int end;
    int y;

    size = 12;
    kind = 1;
    font = D_001DF3D0;
    focused = *(Menu **)(D_001D5F74 + 0x40) == menu;
    if (menu->flags & 4) {
        size = 14;
        kind = 3;
        font = D_001DFB10;
    }
    if (menu->flags & 8) {
        size = 10;
        kind = 2;
        font = D_001DF770;
    }
    func_00234C98_l(0x42, 0x44);
    func_00234C98_l(0x47, 0x2004B);
    SetupGifPaging(0);

    n = 0;
    p = menu->items;
    while (p->text != 0) {
        p++;
        n++;
    }
    if (menu->flags & 0x10) {
        rowh = size + 3;
    } else {
        rowh = menu->height / (n + 1);
    }
    y = rowh - size / 2;
    {
        MenuBox box = { { 4, menu->height - 4, 0, menu->width - 2, 0,
                          y - menu->scroll, 0, 0, size + 2 } };

        glyphs = GetEffectTex(kind);
        for (i = 0; menu->items[i].text != 0; i++) {
            sel = 0;
            if (focused && menu->sel == i) {
                sel = 1;
            }
            en = menu->items[i].enabled != 0;
            if (menu->flags & 2) {
                color = 0x80FFA888;
            } else if (sel) {
                color = en ? 0x8020FFFF : 0x80006060;
            } else {
                color = en ? 0x80FFA888 : 0x80303030;
            }
            if (!(menu->flags & 0x10000) && sel && box.s[5] < 4) {
                menu->scroll -= 4;
            }
            text = func_001FE540_id(menu->items[i].text);
            box.s[4] = (menu->flags & 0xA00) ? 0x20 : 4;
            if (menu->flags & 0x400) {
                box.s[9] = 1;
                box.s[4] = menu->width >> 1;
            }
            if (sel) {
                func_001F65A8();
            }
            func_001F7070_a(&box, color, text, -1, glyphs, font);
            if (sel) {
                func_001F6598();
            }
            if (menu->flags & 0x200) {
                draw_menu_selection_marker(0xF, box.s[5] + 9, gSkillPoints[i] != 0);
            }
            if (menu->flags & 0x800) {
                draw_menu_selection_marker(0xF, box.s[5] + 9, D_0015EE88 == menu->items[i].id);
            }
            box.s[5] += box.s[7];
            if (menu->items[i].subtext != 0) {
                text = func_001FE540_id(menu->items[i].subtext);
                box.s[4] = 0x14;
                func_001F7070_a(&box, color, text, -1, glyphs, font);
                box.s[5] += rowh;
            }
            box.s[5] += 8;
            if (!(menu->flags & 0x10000) && sel) {
                end = box.s[5] + box.s[7];
                if (box.s[1] < end) {
                    if (menu->flags & 0x8000) {
                        menu->scroll += end - box.s[1];
                    } else {
                        menu->scroll += 4;
                    }
                }
            }
        }
    }
    DoGifPaging();
    if (menu->flags & 0x8000) {
        menu->flags ^= 0x8000;
        return 1;
    }
    return 2;
}

typedef struct {
    u8 pad_0[0x40];
    f32 w;
    f32 h;
} Font;
typedef struct {
    u8 pad_0[0x78];
    Font *font;
} FontHolder;
typedef struct {
    u8 pad_0[0x40];
    void *owner;
} MenuFocus;
typedef struct {
    u8 pad_0[0x4];
    MenuFocus *focus;
    u8 pad_8[0x28];
    s32 slot[65];
    s32 unk134;
    s32 unk138;
} MenuState;
typedef struct {
    s32 slot;
    u8 pad_4[0x48];
} ItemInfo;
typedef struct {
    u16 icon;
    s16 frame;
    s16 kind;
    s16 id;
    u16 pad_8;
} MenuGridCell;
typedef struct {
    u8 pad_0[0x14];
    FontHolder *holder;
    u8 pad_18[0x8];
    s32 w;
    s32 h;
    u8 pad_28[0x8];
    s32 flags;
    f32 margin_x;
    f32 margin_y;
    s32 selected_cell;
    s32 rows;
    s32 cols;
    MenuGridCell *cells;
} MenuItemGrid;
extern MenuState D_001D5F70_1E950 __asm__("D_001D5F70");
extern ItemInfo D_001864D8[];
extern u8 D_0013D490[];
extern u8 D_0013D5C8_1E950[] __asm__("D_0013D5C8");
extern u8 D_0013E620[];
extern s32 D_0015F538 MACRO_ADDR;
extern short D_001602B0;
extern short D_00160390;
extern short D_00160394;
extern s32 func_001F9B70(s32);
extern void func_001F4630(s32);
extern s32 func_00200198(s32, s32);
extern void func_002008B8(s32, s32, s32, s32, s32, s32);
extern void func_00201640_1E950(s32, s32, s32, s32, u64, s32) __asm__("func_00201640");
extern s32 func_00234C98_1E950(s32, s64) __asm__("func_00234C98");
s32 func_0021E950(MenuItemGrid *grid);

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/ui/menus/fun_0021d948.c, draw_menu_item_grid. */
s32 func_0021E950(MenuItemGrid *grid) {
    Font *font;
    MenuGridCell *cell;
    s32 focused;
    f32 start_x, column_step, start_y, y, row_step, x;
    f32 scale;
    s32 largest_dimension;
    s32 icon_width, icon_height;
    s32 i, j;
    s32 left, top, right, bottom;
    s32 id, frame_offset;
    u32 color;

    font = grid->holder->font;
    focused = D_001D5F70_1E950.focus->owner == grid;
    cell = grid->cells;
    func_00234C98_1E950(0x42, 0x8000000044L);
    func_00234C98_1E950(0x47, 0xB);
    SetupGifPaging(0);

    if (grid->cols >= 2) {
        start_x = grid->margin_x;
        column_step = (*(f32 *)&D_00160390) + (font->w - (start_x + start_x) - (*(f32 *)&D_00160390) * grid->cols) / (grid->cols - 1);
    } else {
        column_step = 0.0f;
        start_x = (font->w - (*(f32 *)&D_00160390)) * 0.5f;
    }

    if (grid->flags & 2) {
        row_step = (*(f32 *)&D_00160394) + 0.15f;
        start_y = grid->margin_y;
    } else if (grid->rows >= 2) {
        start_y = grid->margin_y;
        row_step = (*(f32 *)&D_00160394) + (font->h - (start_y + start_y) - (*(f32 *)&D_00160394) * grid->rows) / (grid->rows - 1);
    } else {
        row_step = 0.0f;
        start_y = (font->h - (*(f32 *)&D_00160394)) * 0.5f;
    }

    largest_dimension = grid->h;
    if (largest_dimension < grid->w) {
        largest_dimension = grid->w;
    }
    scale = (f32)(largest_dimension << 4) / (font->h < font->w ? font->w : font->h);
    icon_width = scale * (*(f32 *)&D_00160390);
    icon_height = scale * (*(f32 *)&D_00160394);

    y = start_y;
    for (i = 0; i < grid->rows; i++) {
        x = start_x;
        for (j = 0; j < grid->cols; j++) {
            top = scale * y;
            bottom = top + icon_height;
            left = scale * x;
            right = left + icon_width;
            if (focused && grid->selected_cell == cell - grid->cells) {
                color = ((func_001F9B70((D_0015F538 & 0x3F) - 0x20) + 0x40) * 0x10202) | 0x80000000;
                func_00201640_1E950(left - 0x30, top - 0x30, right + 0x30, bottom + 0x30, color, 1);
                func_00201640_1E950(left - 0x10, top - 0x10, right + 0x10, bottom + 0x10, (*(s32 *)&D_001602B0), 1);
            }
            if (cell->kind == 0 ? D_0013D5C8_1E950[cell->id] : gSpecialItems[cell->id]) {
                frame_offset = 0;
                if ((u16)cell->kind == 0) {
                    id = cell->id;
                    if (D_001D5F70_1E950.slot[D_001864D8[id].slot] == id && !(grid->flags & 0x20)) {
                        frame_offset = 1;
                    }
                    if (frame_offset == 0) {
                        frame_offset = D_0013E620[id] ? 4 : 0;
                    }
                    if (D_001D5F70_1E950.unk134 != 0 && (grid->flags & 8)) {
                        frame_offset = 2;
                    }
                    if (D_001D5F70_1E950.unk138 != 0 && (grid->flags & 4)) {
                        frame_offset = 2;
                    }
                }
                func_002008B8(GetIconFrame(cell->icon, cell->frame + frame_offset), left, top, icon_width, icon_height, 0x80);
            }
            cell++;
            x += column_step;
        }
        y += row_step;
    }
    DoGifPaging();
    return 2;
}

extern unsigned char D_001414F4 NOT_SDA;

/* A short-returning inline keeps retail's branch: the select happens in
   HImode, which has no movcc pattern, so jump.c cannot turn it into the
   xori/movz a promoted `short` local gets. */
static inline short pauseFlagState(void) {
    if (D_001414F4 != 1) {
        return 3;
    }
    return 0;
}

/* The select sits in a `static inline short` helper so that it stays a
   branch: jump.c turns an if into movz/movn only when its arm sets a
   full register, and the inline's short return value is a subreg. */
int func_0021EDD8(char *arg0) {
    char *p = *(char **)(arg0 + 0x34);
    *(short *)(p + 2) = pauseFlagState();
    return 0;
}

extern int D_001D53A0[];
extern void func_0020E180(int, int);

/* Tears down the 24 handles at arg0+0x44, skipping the ones in use.
   The switch's table is jtbl_001E8AD0; `off` before `slots` and a second
   slots2/off2 pair give retail's copies. */
int func_0021EE00(char *arg0) {
    int i;

    for (i = 0; i < 24; i++) {
        int off = i * 4;
        char *slots = arg0 + 0x44;
        if (*(int *)(slots + off) == 0) {
            continue;
        }
        if (*(unsigned char *)(arg0 + i + 0xA4) != 0) {
            continue;
        }
        if (D_001D53A0[i] == 0) {
            continue;
        }
        if (i == 7 && *(short *)(*(char **)(arg0 + 0x60) + 0xA6) == 0x4A) {
            unsigned char *o = *(unsigned char **)(arg0 + 0x44);
            if (o[0x52] != o[0x53]) {
                continue;
            }
        }
        switch (i) {
        case 1:
        case 2:
        case 3:
        case 5:
        case 6:
        case 10:
        case 11:
        case 12: {
            unsigned char *o = *(unsigned char **)(arg0 + 0x44);
            if ((*(long *)(o + 0x50) & 0xFFFF0000L) == 0x99990000L
                && o[0x50] >= 0x4D && o[0x50] < 0x92) {
                continue;
            }
            break;
        }
        }
        {
            char *slots2 = arg0 + 0x44;
            int off2 = i * 4;
            DrawMobyList(*(int *)(slots2 + off2), 1);
        }
    }
    return 4;
}

int func_0021EF30(void) {
    return 0;
}

int func_0021EF38(void *arg0) {
    char *p = (char *)arg0;
    *(float *)(p + 0x38) = 3.14159274f;
    *(int *)(p + 0x34) = 0;
    *(int *)(p + 0x44) = 0;
    *(int *)(p + 0x48) = 0;
    return 0;
}

int func_0021EF60(char *arg0) {
    *(int *)(arg0 + 0x44) = func_002267C0(*(int *)(arg0 + 0x44));
    *(int *)(arg0 + 0x48) = func_002267C0(*(int *)(arg0 + 0x48));
    return 0;
}

extern char D_001864D0[];
extern char D_00187040[];
extern void *func_00226720_a(int) __asm__("func_00226720");
extern void func_0021F200(char *);

/* Keeps the item preview moby in step with the highlighted entry: drop it
   when the entry's class (D_001864D0 record +0x3A) is -1, spawn it in
   front of the camera focus when there is none yet, or respawn it with
   the old one's position and orientation when the class changed. */
int func_0021EFA0(char *arg0) {
    char *q = *(char **)(D_001D5F74 + 0x40);
    int item = *(short *)(*(int *)(q + 0x3C) * 10 + *(char **)(q + 0x48) + 6);
    short cur;
    char *rec;
    short want;

    cur = *(char **)(arg0 + 0x44) != 0 ? *(short *)(*(char **)(arg0 + 0x44) + 0xA6) : -1;
    rec = D_001864D0 + item * 0x4C;
    want = *(short *)(rec + 0x3A);
    if (want != -1 && cur == -1) {
        char *o = func_00226720_a(want);

        if (o != 0) {
            char *t = D_00187040;

            *(char **)(arg0 + 0x44) = o;
            *(short *)(o + 0x34) = 0;
            *(float *)(o + 0x10) = *(float *)(t + 0x140) + 6.0f;
            *(float *)(o + 0x14) = *(float *)(t + 0x144);
            *(float *)(o + 0x18) = *(float *)(t + 0x148) - 0.3f;
            *(float *)(o + 0x48) = 3.1415927f;
            *(void **)(o + 0x74) = (void *)func_0021F200;
            **(void ***)(o + 0x78) = arg0;
        }
    } else if (want == -1) {
        *(int *)(arg0 + 0x44) = func_002267C0(*(int *)(arg0 + 0x44));
    } else if (cur != want) {
        char *n = func_00226720_a(want);

        if (n != 0) {
            char *old;

            *(short *)(n + 0x34) = 0;
            old = *(char **)(arg0 + 0x44);
            qcopy(n + 0x10, old + 0x10);
            qcopy(n + 0x40, old + 0x40);
            *(int *)(n + 0x74) = *(int *)(old + 0x74);
            **(void ***)(n + 0x78) = arg0;
        }
        func_002267C0(*(int *)(arg0 + 0x44));
        *(char **)(arg0 + 0x44) = n;
    }
    return 0;
}

extern char *D_001D5F74 NOT_SDA;
extern void func_0020E180(int, int);
extern void func_001F4630(int);
extern void func_001F4748(void);
extern void *func_001FE540_id(int) __asm__("func_001FE540");
extern void func_00227A30(void *, char *);
extern void func_001F7560(void *, long, void *, int);

/* Draw/teardown callback (func_0021F610's family). The entry address is
   written inline in the index; a `char *e` local leaves an addu swap. */
int func_0021F118(char *arg0) {
    short buf[10];
    char *p = *(char **)(D_001D5F74 + 0x40);

    if (((unsigned char *)&D_0013D5C8)[*(short *)(*(int *)(p + 0x3C) * 10 + *(char **)(p + 0x48) + 6)] == 0) {
        return 0;
    }
    if (*(int *)(arg0 + 0x44) != 0) {
        DrawMobyList(*(int *)(arg0 + 0x44), 1);
        if (*(int *)(arg0 + 0x48) != 0) {
            DrawMobyList(*(int *)(arg0 + 0x48), 1);
        }
        return 8;
    }
    SetupGifPaging(0);
    func_00227A30(buf, arg0);
    buf[8] = 0x10;
    buf[9] = 3;
    func_001F7560(buf, 0x80FFA888L, func_001FE540_id(0x4F4D), -1);
    DoGifPaging();
    return 2;
}

extern float func_001FA748(float, float);

void func_0021F200(char *arg0) {
    *(float *)(arg0 + 0x48) = FastAddRots(*(float *)(arg0 + 0x48), 0.01f);
}

struct MenuScreen;
struct MenuPage {
    s32 moby_anims[14]; /* 0x00: animation per menu moby */
    struct MenuPage *back; /* 0x38: page for the back button (pad 0x10); 0 = none */
    s32 state; /* 0x3C: menu_system.state while this page is current */
    struct MenuScreen *focus; /* 0x40: screen with input focus */
    struct MenuScreen *screens[14]; /* 0x44 */
    u8 pad_7C[0x4];
    struct MenuScreen *pending_focus; /* 0x80: becomes focus on the next update */
    s32 confirmed; /* 0x84: confirm page answer, 1 = yes */
};
struct MenuSystem {
    /* Page switching (FUN_002192a8) */
    s32 state; /* 0x000: menu state, from page->state; 0x2D at init */
    struct MenuPage *current; /* 0x004: active page */
    struct MenuPage *next; /* 0x008: requested page */
    s32 close_request; /* 0x00C: nonzero closes the menu */
    s32 unk10; /* 0x010: buffer address D_001940C0[2] + 0xA0000 */
    s32 timer; /* 0x014: 12 on a switch, 2 on close; counts down */
    s32 saved_texture_start; /* 0x018: gs_texture_allocation_start to restore on close */
    s32 current_gadget; /* 0x01C: gadget of the queued preview animation, -1 none */
    u8 pad_20[0x10];
    s32 equipped[4]; /* 0x030: gadget id held by each slot */
    u8 pad_40[0x60];

    /* Preview animation stream */
    s32 stream_buffer[2]; /* 0x0A0: double buffer */
    s32 unkA8; /* 0x0A8: fun_00226848 first-resource argument */
    s32 unkAC; /* 0x0AC: fun_00226848 count argument */
    s32 unkB0[3]; /* 0x0B0: buffer addresses read by fun_00226848 */
    s32 read_offset; /* 0x0BC: offset of the data in the read buffer */
    s32 unkC0; /* 0x0C0: -1 at preview init */
    u8 pad_C4[0x4];
    u8 loaded_animation[2]; /* 0x0C8: animation held by each buffer, 0xFF none */
    u8 read_buffer_index; /* 0x0CA: buffer being read */
    u8 pending_buffer; /* 0x0CB: buffer index + 1 while a read is in flight */
    u32 streamed_animation_base; /* 0x0CC: first animation id read from the stream archive */

    struct MenuPage *previous; /* 0x0D0: page left by the last switch */
    s32 unkD4; /* 0x0D4: 0 saving_data_menu, 1/2 saving_data_menu2; picks draw_prompt_box text */
    s32 unkD8; /* 0x0D8: fun_002196b8 draws slot 6 only while set */
    s32 unkDC; /* 0x0DC: pause_all_sounds: (mode == 0x23) */
    s32 unkE0; /* 0x0E0: memcard_make_whole_save / memcard_restore_game argument */
    s32 unkE4; /* 0x0E4: written with unkF0/unkF4 before close_request 3 */
    u8 pad_E8[0x4];
    s32 unkEC; /* 0x0EC: menu_action_messages[] entry (fun_0021abf8) */
    struct MenuPage *unkF0; /* 0x0F0: menu_system.current, or D_001CF418 (draw_map_screen) */
    s32 unkF4; /* 0x0F4: 0, 2, 0xB or 0xF */
    s32 unkF8; /* 0x0F8: read by pause_all_sounds */

    /* Buffers carved at init (FUN_00218f98) */
    s32 unkFC; /* 0x0FC: from D_001940C0[1] */
    s32 unk100; /* 0x100: from D_001940C0[2] */
    s32 unk104; /* 0x104: D_001940C0[1] + 0xA0000 */
    s32 help_text_buffer; /* 0x108: help text read target, D_001940C0[1] + 0xDC000 */
    s32 unk10C; /* 0x10C: D_001940C0[2] + 0x180000 */

    s32 update_count; /* 0x110: FUN_002192a8 updates */
    u8 pad_114[0x4];

    /* Preview moby resources */
    s32 resource_table_toggle; /* 0x118: alternates per resource table select */
    s32 unk11C; /* 0x11C: an oclass, -1 none */
    s32 unk120; /* 0x120: an oclass, -1 none */

    s32 close_locked; /* 0x124: nonzero blocks start/back closing the menu */
    s32 card_op_pending; /* 0x128: card save/load started, waiting for the card */
    s32 card_op_text; /* 0x12C: text shown while pending (0x4FB5 save, 0x4FB6 load) */
    u8 pad_130[0x4];
    s32 unk134; /* 0x134: hides pages / items with flags 8 */
    s32 unk138; /* 0x138: hides pages / items with flags 4 */
    s32 unk13C; /* 0x13C: written by FUN_00218d78 */
    s32 unk140; /* 0x140: oclass at the last resource table select */
    s32 last_resource_table_toggle; /* 0x144: resource_table_toggle at the last select */
};
extern struct MenuSystem D_001D5F70_1F238 __asm__("D_001D5F70");
struct MenuScreenWords {
    u8 pad_30[0x4];
    s32 unk34; /* 0x34: flags 0x40 / 0x80 (draw_map_screen) */
    s32 unk38; /* 0x38: cleared by FUN_0021d1f8 */
    s32 unk3C; /* 0x3C: stream buffer (FUN_00225660) */
    u8 pad_40[0x10];
    s32 unk50; /* 0x50: 0, 1 or 3 (FUN_0021d1f8) */
    s32 unk54; /* 0x54: cleared by FUN_0021d1f8 */
    u8 pad_58[0x8];
};
struct MenuOption {
    void *unk0; /* 0x0: 0 ends the list */
    u8 *flag; /* 0x4: toggled by confirm (pad 0x40) */
    u8 pad_8[0x8];
    u32 flags; /* 0x10: bit 0 = fade out and toggle D_0016034C instead */
};
struct MenuOptionListData {
    u8 pad_30[0x4];
    struct MenuOption *list; /* 0x34: ended by a null unk0 */
    s32 selection; /* 0x38: pad 0x1000 up, 0x4000 down */
    s32 fade_timer; /* 0x3C: counts down after fade_to_black, drives D_0015F43C */
};
struct MenuChoice {
    s32 unk0; /* 0x0: 0 ends the list */
    u8 *value; /* 0x4: chosen option; confirm advances it */
    s32 option[4]; /* 0x8: 0 ends them early */
};
struct MenuChoiceListData {
    u8 pad_30[0x4];
    struct MenuChoice *list; /* 0x34: ended by a zero unk0 */
    s32 selection; /* 0x38: pad 0x1000 up, 0x4000 down */
};
struct MenuMissionListData {
    s32 choice[19]; /* 0x30: selected mission per level; -1 while unfocused */
    s32 count; /* 0x7C: collect_mission_ids result */
};
struct MenuGridCell {
    u16 icon; /* 0x0: get_icon_frame icon */
    s16 frame; /* 0x2: first frame */
    s16 kind; /* 0x4: 0 = item (owned D_0013D4C0[id]), else D_0013D388[id] */
    s16 id; /* 0x6: item id */
    s16 stream_entry; /* 0x8: stream entry shown for this cell (fun_0021fdc8) */
};
struct MenuItemGridData {
    s32 flags; /* 0x30: 2 fixed row step, 4/8 lockable, 0x20 no equipped frame */
    f32 margin_x; /* 0x34: left margin */
    f32 margin_y; /* 0x38: top margin */
    s32 selected_cell; /* 0x3C: highlighted cell */
    s32 rows; /* 0x40 */
    s32 cols; /* 0x44 */
    struct MenuGridCell *cells; /* 0x48: rows * cols */
    struct MenuScreen *up; /* 0x4C */
    struct MenuScreen *down; /* 0x50 */
    struct MenuScreen *left; /* 0x54 */
    struct MenuScreen *right; /* 0x58 */
};
struct MenuIcon {
    u16 icon; /* 0x0: get_icon_frame icon */
    s16 frame; /* 0x2: get_icon_frame frame */
};
struct MenuIconListData {
    u8 pad_30[0xC];
    s32 cursor; /* 0x3C: highlighted icon */
    s32 count; /* 0x40: icons drawn */
    u8 pad_44[0x4];
    struct MenuIcon *list; /* 0x48 */
    u8 pad_4C[0x10];
    s32 first_y; /* 0x5C: y of the first icon, 0x260 per icon */
};
struct MenuSaveSlotData {
    s32 flags; /* 0x30: bit 0 pad repeat mask, 0x2000 second variant */
    u8 pad_34[0xC];
    s32 slot; /* 0x40: memory card slot 0..4, mirrored in D_0015EE34 */
    u8 pad_44[0x4];
    s32 save_data; /* 0x48: buffer for prepare_save_game / FUN_00226a70 */
    s32 step; /* 0x4C: 0 idle, 1 start saving, 2 started */
};
struct MenuPromptData {
    u8 pad_30[0x24];
    s32 buffer; /* 0x54: stream buffer */
};
struct MenuItemSlotsData {
    s32 items[8]; /* 0x30: item id per slot */
    s32 cursor; /* 0x50: slot 0..7, pad 8/4 step it */
};
struct MenuCycleData {
    u8 pad_30[0x24];
    s32 selection; /* 0x54: one of twelve, wraps */
};
struct MenuTextItem {
    s16 text; /* 0x0: help text id; 0 ends the list */
    s16 action; /* 0x2: confirm action, see above */
    union {
        s32 value; /* page, close value, level or language */
        struct {
            u16 lo; /* action 6: menu_system.unkE4 */
            s16 hi; /* action 6: menu_action_messages index, 0 = none */
        } half;
    } param; /* 0x4 */
    s16 subtext; /* 0x8: second line text id, 0 = none */
    s16 fade_timer; /* 0xA: up while selected, down otherwise */
};
struct MenuTextListData {
    s32 flags; /* 0x30: 1 pad repeat, 2 no highlight, 4/8 font, 0x10 fixed step, 0x20 D_001A0314, 0x1000 wrap, 0x8000 scroll once */
    struct MenuTextItem *items; /* 0x34: ended by a zero text */
    struct MenuScreen *up; /* 0x38: focus above the first entry */
    struct MenuScreen *down; /* 0x3C: focus below the last entry */
    s32 selected; /* 0x40: highlighted entry */
    s32 scroll; /* 0x44: text offset, keeps the selection visible */
};
struct MenuStreamEntry {
    s32 sector; /* 0x0: start_audio_stream_read sector */
    s32 sector_count; /* 0x4: 0 = nothing to load */
};
struct MenuStreamData {
    struct MenuStreamEntry *entries; /* 0x30 */
    s32 flags; /* 0x34: see above */
    s32 texture_width; /* 0x38: draw size of the streamed image */
    s32 texture_height; /* 0x3C */
    s32 *language_base; /* 0x40: base entry for flags 0x1000 */
    s32 state; /* 0x44: see above */
    s32 buffer[2]; /* 0x48: stream buffers */
    s32 loaded_entry[2]; /* 0x50: entry held by each buffer, -1 none */
    s32 fixed_entry; /* 0x58: entry for flags 1, -1 none */
    s32 elapsed_frames; /* 0x5C: updates since enter */
    s32 read_offset; /* 0x60: data offset of a flags 0x20 read */
};
struct MenuItemPreviewData {
    u8 pad_30[0x14];
    char *moby; /* 0x44: preview moby (create_menu_preview_moby) */
    char *second_moby; /* 0x48: optional, drawn after moby */
};
struct MenuLabelData {
    s32 flags; /* 0x30: see above */
    s32 text_id; /* 0x34: help text id, or table of text ids per value */
    u32 text_stride; /* 0x38: byte stride of the text_id table */
    s32 scroll_offset; /* 0x3C: text scroll, 1/16 px */
    u8 pad_40[0x4];
    s32 fade_timer; /* 0x44: reset to scale_game_frames(menu_fade_duration) */
    s32 cached_value; /* 0x48: value shown while fading */
    s32 value_variant; /* 0x4C */
};
struct MenuScreen {
    s32 (*update)(struct MenuScreen *); /* 0x00 */
    u8 pad_4[0x4];
    void (*enter)(struct MenuScreen *, s32); /* 0x08 */
    void (*leave)(struct MenuScreen *, s32); /* 0x0C */
    s32 unk10; /* 0x10: bit 2 set by FUN_0021d1f8 */
    s32 moby; /* 0x14: its menu moby D_001D5D90[i]; menu sounds play on it */
    s32 x; /* 0x18: left edge */
    s32 y; /* 0x1C: top edge */
    s32 width; /* 0x20 */
    s32 height; /* 0x24 */
    u8 pad_28[0x8];
    union {
        struct MenuScreenWords raw; /* not understood yet */
        struct MenuOptionListData options; /* FUN_00220e28 */
        struct MenuChoiceListData choices; /* FUN_002212b8 */
        struct MenuMissionListData missions; /* FUN_0021c4c0 */
        struct MenuItemGridData grid; /* draw_menu_item_grid */
        struct MenuIconListData icons; /* FUN_00219fa0 */
        struct MenuSaveSlotData save; /* saving_data_menu, saving_data_menu2 */
        struct MenuItemSlotsData slots; /* FUN_0021c7a0 */
        struct MenuCycleData cycle; /* update_menu_cycle_selection */
        struct MenuTextListData list; /* draw_menu_text_list, fun_0021abf8 */
        struct MenuStreamData stream; /* fun_0021fdc8, fun_0021f990 */
        struct MenuItemPreviewData preview; /* FUN_0021e110, update_item_preview_moby */
        struct MenuLabelData label; /* fun_0021a328 */
        struct MenuPromptData prompt; /* draw_prompt_box, fun_00221a48 */
    } data; /* 0x30 */
};
struct ItemPreviewVars_1F238 {
    void *owner;
    u8 pad4[8];
    s32 item_index;
};
struct ItemPreviewBinding_1F238 {
    u8 pad0[0x14];
    s32 variant;
    u8 pad18[0x18];
    s32 flags;
    s32 state;
    f32 rotation_angle;
    u8 pad3C[8];
    struct Moby *primary_moby;
    struct Moby *secondary_moby;
};
struct PreviewItemDefinition {
    u8 pad0[8];
    s32 item_type;
    u8 padC[4];
    s32 oclass;
    u8 pad14[0x38];
};
struct ItemPreviewPlacement_1F238 {
    f32 alternate_x;
    f32 normal_x;
    f32 y;
    f32 z;
    f32 rotation_x;
    f32 rotation_y;
    u8 pad18[8];
};
struct PreviewCamera_1F238 {
    u8 pad0[0x140];
    f32 x;
    f32 y;
    f32 z;
};
struct PreviewClassResource {
    u8 pad0[0xD];
    u8 state_0d;
};
extern struct PreviewItemDefinition D_001864D0_1F238[] __asm__("D_001864D0");
extern struct PreviewCamera_1F238 D_00187040_1F238 __asm__("D_00187040");
extern struct ItemPreviewPlacement_1F238 D_001E0708_1F238[] __asm__("D_001E0708");
extern s32 D_00140508[];
extern s32 D_00160050 MACRO_ADDR;
extern u8 D_001B3E40[];
extern struct PreviewClassResource * D_001B3580_1F238[] __asm__("D_001B3580");
extern void func_0021F6A0();
extern void func_001E97F0(s32, s32);
extern void func_001E97F8_1F238(struct Moby *, s32) __asm__("func_001E97F8");
extern f32 func_001FA748(f32, f32);
extern void func_00205270(s32, s32);
extern void func_00213D28(struct Moby *, s32, s32);
extern struct Moby *func_00226720_1F238(s32) __asm__("func_00226720");
extern struct Moby *func_002267C0_1F238(struct Moby *) __asm__("func_002267C0");

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/textbin/fun_0021e230.c, update_item_preview_binding. */
s32 func_0021F238(struct ItemPreviewBinding_1F238 *preview) {
    struct MenuScreen *grid;
    struct Moby *moby;
    struct Moby *secondary_moby;
    struct PreviewItemDefinition *definition;
    s32 item_index;
    s32 previous_class;
    s32 oclass;
    s32 item_type;
    s32 has_secondary_moby;
    s32 load_class;
    s32 use_alternate_x;
    s32 animation_index;
    s32 last_animation;
    struct Moby *source_moby;
    s32 is_type2;
    struct ItemPreviewVars_1F238 *preview_vars;
    s32 is_type1;
    s32 item_two_difference;
    f32 camera_x;
    struct ItemPreviewVars_1F238 *secondary_vars;
    f32 position_x;

    grid = D_001D5F70_1F238.current->focus;
    item_index = grid->data.grid.cells[grid->data.grid.selected_cell].id;
    if (preview->primary_moby != 0) {
        previous_class = preview->primary_moby->oclass;
    } else {
        previous_class = -1;
    }
    load_class = 0;
    oclass = D_001864D0_1F238[item_index].oclass;
    if (item_index == 0x18) {
        oclass = 0x1DF;
    }
    preview->rotation_angle = FastAddRots(preview->rotation_angle, 0.01f);
    item_type = D_001864D0_1F238[item_index].item_type;
    is_type2 = item_type == 2;
    has_secondary_moby = item_type == 3;
    is_type1 = item_type == 1;
    if (!is_type2 && !has_secondary_moby && !is_type1) {
        load_class = 1;
    }
    if (item_index == 0x18) {
        load_class = 0;
    }
    use_alternate_x = preview->flags & 1;
    switch (preview->state) {
    case 0:
        preview->state = 1;
        break;
    case 1:
        if (oclass != -1) {
            preview->state = 2;
        }
        break;
    case 2:
        if (oclass == -1) {
            preview->state = 1;
            break;
        }
        if (load_class) {
            if (D_00140508[0] != 0 &&
                oclass != D_00140508[0]) {
                func_001E97F0(0, 0);
            }
            if (oclass != D_001D5F70_1F238.unk11C) {
                func_00205270(oclass, D_001D5F70_1F238.resource_table_toggle == 0);
                D_001D5F70_1F238.last_resource_table_toggle = D_00160050;
                D_001D5F70_1F238.unk140 = oclass;
                D_001D5F70_1F238.unk120 = oclass;
                D_001B3580_1F238[D_001B3E40[oclass]]->state_0d = 0;
            }
        }
        moby = func_00226720_1F238(oclass);
        item_two_difference = item_index ^ 2;
        if (item_two_difference == 0) {
            animation_index = 6;
        } else {
            animation_index = 1;
        }
        if (moby != 0) {
            if (load_class && D_0013E620[item_index] != 0) {
                func_001E97F8_1F238(moby, preview->variant);
            }
            preview->primary_moby = moby;
            moby->flags = 0;
            camera_x = D_00187040_1F238.x;
            if (use_alternate_x) {
                position_x = camera_x + D_001E0708_1F238[item_index].alternate_x;
            } else {
                position_x = camera_x + D_001E0708_1F238[item_index].normal_x;
            }
            moby->pos.x = position_x;
            moby->pos.y = D_00187040_1F238.y + D_001E0708_1F238[item_index].y;
            moby->pos.z = D_00187040_1F238.z + D_001E0708_1F238[item_index].z;
            moby->rot.x = D_001E0708_1F238[item_index].rotation_x;
            moby->rot.y = D_001E0708_1F238[item_index].rotation_y;
            moby->rot.z = 3.1415927f;
            moby->update = func_0021F6A0;
            preview_vars = (struct ItemPreviewVars_1F238 *)moby->pvars;
            preview_vars->owner = preview;
            preview_vars->item_index = item_index;
            last_animation = moby->pclass->seq_count - 1;
            if (animation_index < last_animation) {
                last_animation = animation_index;
            }
            func_00213D28(moby, last_animation, 0);
        }
        if (has_secondary_moby && moby != 0) {
            moby = func_00226720_1F238(D_001864D0_1F238[1].oclass);
            if (moby != 0) {
                moby->flags = 0;
                source_moby = preview->primary_moby;
                qcopy(&moby->pos, &source_moby->pos);
                qcopy(&moby->rot, &source_moby->rot);
                preview->secondary_moby = moby;
                moby->update = func_0021F6A0;
                secondary_vars = (struct ItemPreviewVars_1F238 *)moby->pvars;
                secondary_vars->item_index = item_index;
                secondary_vars->owner = preview;
                func_00213D28(moby,
                                   (animation_index < moby->pclass->seq_count - 1)
                                       ? animation_index
                                       : moby->pclass->seq_count - 1,
                                   0);
            }
        }
        preview->state = 3;
        break;
    case 3:
        if (previous_class != oclass) {
            preview->primary_moby = func_002267C0_1F238(preview->primary_moby);
            preview->secondary_moby = func_002267C0_1F238(preview->secondary_moby);
            preview->state = 2;
        }
        break;
    }
    return 0;
}

extern char *D_001D5F74 NOT_SDA;
extern void func_0020E180(int, int);

/* The entry address is written inline in the index, as in
   func_0021F118; a `char *e` local swapped the addu. */
int func_0021F610(char *arg0) {
    char *p = *(char **)(D_001D5F74 + 0x40);
    int v;

    if (((unsigned char *)&D_0013D5C8)[*(short *)(*(int *)(p + 0x3C) * 10 + *(char **)(p + 0x48) + 6)] == 0) {
        return 0;
    }
    v = *(int *)(arg0 + 0x44);
    if (v != 0) {
        DrawMobyList(v, 1);
    }
    v = *(int *)(arg0 + 0x48);
    if (v != 0) {
        DrawMobyList(v, 1);
    }
    return 8;
}

struct ItemPreviewPlacement {
    f32 alternate_x;
    f32 normal_x;
    f32 y;
    f32 z;
    u8 pad10[8]; /* Per-item record stride is 0x20. */
    f32 side_offset;
    f32 forward_offset;
};
struct ItemPreviewBinding {
    u8 pad0[0x30];
    s32 flags;
    u8 pad34[4];
    f32 rotation_angle;
};
struct ItemPreviewVars {
    struct ItemPreviewBinding *owner;
    u8 pad4[8];
    s32 item_index;
};
struct ItemPreviewMoby {
    u8 pad0[0x10];
    volatile f32 x;
    f32 y;
    f32 z;
    u8 pad1C[0x2C];
    f32 rotation_z;
    u8 pad4C[0x2C];
    struct ItemPreviewVars *preview_vars;
};
struct PreviewCamera {
    u8 pad0[0x140];
    f32 x;
    f32 y;
    f32 z;
};
extern struct PreviewCamera D_00187040_1F6A0 __asm__("D_00187040");
extern struct ItemPreviewPlacement D_001E0708[];
extern f32 func_001F9F90(f32);
extern f32 func_001F9FA8(f32);

/* Position the selected item relative to the preview camera. Owner flag bit 0 selects alternate_x; otherwise use normal_x. Rotate the side and forward offsets in the X/Y plane after adding the unrotated placement position.
   Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/textbin/update_item_preview_transform.c, update_item_preview_transform. */
void func_0021F6A0(struct ItemPreviewMoby *moby) {
    struct ItemPreviewBinding *preview = moby->preview_vars->owner;
    s32 item_index = moby->preview_vars->item_index;
    f32 side_offset;
    f32 forward_offset;
    f32 cosine;
    f32 sine;
    f32 negated_forward_offset;
    s32 flags = preview->flags;
    f32 y_offset;
    struct PreviewCamera *camera;

    moby->rotation_z = preview->rotation_angle;
    moby->x = D_00187040_1F6A0.x + ((flags & 1) ? D_001E0708[item_index].alternate_x
                                              : D_001E0708[item_index].normal_x);
    camera = &D_00187040_1F6A0;
    y_offset = D_001E0708[item_index].y;
    moby->y = camera->y + y_offset;
    moby->z = camera->z + D_001E0708[item_index].z;
    forward_offset = D_001E0708[item_index].forward_offset;
    side_offset = D_001E0708[item_index].side_offset;
    cosine = FastCos(moby->rotation_z);
    negated_forward_offset = -forward_offset;
    sine = FastSin(moby->rotation_z);
    moby->x += negated_forward_offset * sine + side_offset * cosine;
    moby->y += forward_offset * cosine + side_offset * sine;
}

extern int D_0013D48C;
extern int D_0015F6E4 MACRO_ADDR;
extern int D_0015F6FC_m __asm__("D_0015F6FC") MACRO_ADDR;
extern int D_0015F690 MACRO_ADDR;

/* Pad handler (func_0021ACD8's family). A separate `char *` local for
   each block that reads the pad word gives retail's pattern: the %hi
   kept in a register and the %lo rebuilt before each use. */
int func_0021F7D0(char *arg0) {
    char *pad = D_0013CA40;
    int v = *(int *)(pad + 0x1C4);

    if (v & 0xD00) {
        if (*(int *)(arg0 + 0x30) & 0x20) {
            D_001A0414 = D_0015EE84;
        }
        return -1;
    }
    if (v & 0x10) {
        char *g;
        int t;
        if (*(int *)(arg0 + 0x30) & 0x20) {
            D_001A0414 = D_0015EE84;
        }
        g = D_001D5F70;
        t = *(int *)(*(char **)(g + 4) + 0x38);
        if (t != 0) {
            *(int *)(g + 8) = t;
            return 0;
        }
        if (*(int *)(g + 0x124) == 0) {
            return -1;
        }
    }
    {
        char *pad2 = D_0013CA40;
        if (*(int *)(pad2 + 0x1C4) & 0x20) {
            D_0013D48C = 0;
            D_0015F6E4 = -1;
            D_0015F690 = 1;
            D_0015F6FC_m = 1;
        }
    }
    return 0;
}

extern void func_001153FC(void *, int, int); /* memset */
extern void func_00234C98_l(int, long) __asm__("func_00234C98");
extern void func_001F6EA8(int, int, long, void *, int);
extern void func_001F68E8_c(int, int, long, void *, int) __asm__("func_001F68E8");

/* DrawQuitGameMenu: a text box (zeroed, then sized from arg0+0x20/0x24)
   with text 0x4F6D, text 0x4F3F centred 0x40 above the bottom, and the
   two answers 0x5250/0x5254 at 0x28 and 0x14 above it, left-aligned so
   that the wider of the two (func_001F6600) is centred. The three y
   positions are computed before the calls (retail keeps them in saved
   registers); the box's y halfword is stored last, after the other
   fields in order, which gives retail's store schedule. */
int func_0021F898(char *arg0) {
    short box[12];
    int w = *(int *)(arg0 + 0x20);
    int h = *(int *)(arg0 + 0x24);
    int cx;
    int y1, y2, y3;
    int a, b, x;

    func_001153FC(box, 0, 0x18);
    cx = w / 2;
    y1 = h - 0x40;
    y2 = h - 0x28;
    y3 = h - 0x14;
    box[2] = 4;
    box[3] = w - 4;
    box[4] = cx;
    box[5] = 6;
    box[6] = w;
    box[7] = h;
    box[8] = 0x10;
    box[9] = 1;
    box[1] = h;
    SetupGifPaging(0);
    func_00234C98_l(0x47, 0x2004B);
    func_001F7560(box, 0x80FFA888L, func_001FE540_id(0x4F6D), -1);
    FontPrintCenter(cx, y1, 0x80FFA888L, func_001FE540_id(0x4F3F), -1);
    a = func_001F6600(func_001FE540_id(0x5250), -1);
    b = func_001F6600(func_001FE540_id(0x5254), -1);
    if (b >= a) {
        a = b;
    }
    x = (*(int *)(arg0 + 0x20) - a) >> 1;
    func_001F68E8_c(x, y2, 0x80FFA888L, func_001FE540_id(0x5250), -1);
    func_001F68E8_c(x, y3, 0x80FFA888L, func_001FE540_id(0x5254), -1);
    DoGifPaging();
    return 2;
}

extern char D_00187040[];
extern void func_00220128(void *);
extern void *func_00226720_a(int) __asm__("func_00226720");

/* Attach marker object 0x46E to arg0, parked just above the camera
   focus in D_00187040's 0x140 block, and point it back at its owner
   through the node at +0x78. Always returns 0. */
int func_0021FA50(void *arg0) {
    char *o = (char *)func_00226720_a(0x46E);
    char *t;

    if (o != 0) {
        t = D_00187040;
        *(char **)((char *)arg0 + 0x44) = o;
        *(short *)(o + 0x34) = 0;
        *(float *)(o + 0x10) = *(float *)(t + 0x140) + 8.0f;
        *(float *)(o + 0x14) = *(float *)(t + 0x144) + 0.5f;
        *(float *)(o + 0x18) = *(float *)(t + 0x148) - 0.1f;
        *(float *)(o + 0x44) = -1.9f;
        *(void **)(o + 0x74) = (void *)func_00220128;
        **(void ***)(o + 0x78) = arg0;
    }
    return 0;
}

extern int func_002267C0(int);

int func_0021FAF8(char *arg0) {
    *(int *)(arg0 + 0x44) = func_002267C0(*(int *)(arg0 + 0x44));
    return 0;
}

struct Screen {
    s32 width; /* 0x0: display width (D_00151780 +0x150) */
    s32 height; /* 0x4: display height (D_00151780 +0x152) */
    s32 half_width; /* 0x8: width >> 1 */
    s32 half_height; /* 0xC: height >> 1 */
    s32 left; /* 0x10: (0x800 - half_width) << 4 */
    s32 top; /* 0x14: (0x800 - half_height) << 4 */
    s32 right; /* 0x18: (0x800 + half_width) << 4 */
    s32 bottom; /* 0x1C: (0x800 + half_height) << 4 */
};
extern struct Screen D_0013E600_1FB28 __asm__("D_0013E600");
struct TextRegion {
    s16 top;
    s16 bottom;
    s16 left;
    s16 right;
    s16 anchor_x;
    s16 anchor_y;
    s16 measured_width;
    s16 rendered_height;
    s16 line_advance;
    u16 flags;
    s16 subpixel_x_sixteenths;
    s16 subpixel_y_sixteenths;
};
typedef struct {
    u8 pad0[0x20];
    s32 width;
    u16 height;
    u8 pad26[0x1E];
    s32 help_tip;
} ItemsMenu;
extern s32 D_0015EE88_1FB28[] __asm__("D_0015EE88") MACRO_ADDR;
extern char D_001603A0[];
extern void func_001F6CF8(s32, s32, u64, char *, s32);
extern void func_001F6E18(s32, s32, u64, char *, s32);
extern void func_001F7560_1FB28(struct TextRegion *, u64, char *, s32) __asm__("func_001F7560");
extern char *func_001FE540_1FB28(s32) __asm__("func_001FE540");
extern void func_00201640_1FB28(s32, s32, s32, s32, u64, s32) __asm__("func_00201640");
extern void func_0020E180(s32, s32);
extern s32 func_00216098(void);
extern s32 func_002160E0(void);
extern s32 func_00216150(void);
extern void *func_001153FC_1FB28(void *, s32, u32) __asm__("func_001153FC");
extern int func_00116248(char *, const char *, ...);
extern char *func_00116428(const char *, int);
extern char *func_001166FC_1FB28(char *, const char *) __asm__("func_001166FC");
extern u32 func_00116810_1FB28(const char *) __asm__("func_00116810");
static inline int add_offset(s32 arg0, int arg1) {
    return arg0 + arg1;
}
static inline int divide_coordinate(s32 a, int n) {
    return a / n;
}

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/textbin/ui/menus/inventory/draw_items_menu.c, draw_items_menu. */
s32 func_0021FB28(ItemsMenu *menu) {
    char text_buffer[0x80];
    char *hyphen;
    char *source_end;
    char *destination_end;
    char *format;
    s32 text_length;
    s32 column_divisor;
    s32 x;
    s32 y;
    s32 collected_count;
    if (menu->help_tip != 0) {
        DrawMobyList(menu->help_tip, 1);
    }
    SetupGifPaging(0);
    column_divisor = 3;
    {
        /* Retail clears the 24-byte text window, fills the height, left (8), signed
           width / 3, line advance 0x10 and flags 5, copies it, then centres the anchor. */
        struct TextRegion text_window;
        struct TextRegion window_fields;

        func_001153FC_1FB28(&window_fields, 0, sizeof(window_fields));
        window_fields.bottom = menu->height;
        window_fields.left = 8;
        window_fields.right = divide_coordinate(menu->width, column_divisor);
        window_fields.line_advance = 0x10;
        window_fields.flags = 5;
        text_window = window_fields;
        text_window.anchor_x = add_offset(text_window.left, text_window.right) >> 1;
        func_001166FC_1FB28(text_buffer, func_001FE540_1FB28(0x4F4E));
        if (D_0015EE88_1FB28[0] == column_divisor) {
            hyphen = func_00116428(text_buffer, 0x2D);
            if (hyphen != 0) {
                text_length = func_00116810_1FB28(text_buffer);
                source_end = text_buffer + text_length;
                if (hyphen < source_end) {
                    destination_end = source_end;
                    do {
                        destination_end[1] = *source_end;
                        destination_end--;
                        text_length--;
                        source_end = destination_end;
                    } while (hyphen < source_end);
                }
                text_buffer[add_offset(text_length, 1)] = 0x20;
            }
        }
        func_001F7560_1FB28(&text_window, 0x8000C0C0L, text_buffer, -1);
        text_window.flags ^= 4;
        text_window.anchor_y = (D_0013E600_1FB28.height - text_window.rendered_height) >> 1;
        func_001F7560_1FB28(&text_window, 0x8000C0C0L, text_buffer, -1);
    }
    x = add_offset((*(s32 *)&D_001602B8), 0xC8);
    y = add_offset((*(s32 *)&D_001602BC), 0x1D);
    font_print_right(x, y, 0x80000000L, func_001FE540_1FB28(0x4F4F), -1);
    x = add_offset((*(s32 *)&D_001602B8), 0xC8);
    y = add_offset((*(s32 *)&D_001602BC), 0x36);
    font_print_right(x, y, 0x80000000L, func_001FE540_1FB28(0x4F50), -1);
    x = add_offset((*(s32 *)&D_001602B8), 0xC8);
    y = add_offset((*(s32 *)&D_001602BC), 0x54);
    font_print_right(x, y, 0x80000000L, func_001FE540_1FB28(0x4F51), -1);
    font_print_right(0xC8, 0x1D, 0x80FFA888L, func_001FE540_1FB28(0x4F4F), -1);
    font_print_right(0xC8, 0x36, 0x80FFA888L, func_001FE540_1FB28(0x4F50), -1);
    font_print_right(0xC8, 0x54, 0x80FFA888L, func_001FE540_1FB28(0x4F51), -1);
    collected_count = count_nonzero_entries_up_to_40();
    func_00116248(text_buffer, D_001603A0, collected_count);
    font_print_right_large(add_offset((*(s32 *)&D_001602B8), 0xF0), add_offset((*(s32 *)&D_001602BC), 0x1D), 0x80000000L,
                    text_buffer, -1);
    font_print_right_large(0xF0, 0x1D, 0x80FFA888L, text_buffer, -1);
    func_00116248(text_buffer, D_001603A0, count_nonzero_entries_up_to_10() * 4);
    font_print_right_large(add_offset((*(s32 *)&D_001602B8), 0xF0), add_offset((*(s32 *)&D_001602BC), 0x36), 0x80000000L,
                    text_buffer, -1);
    font_print_right_large(0xF0, 0x36, 0x80FFA888L, text_buffer, -1);
    func_00116248(text_buffer, D_001603A0, func_00216098());
    font_print_right_large(add_offset((*(s32 *)&D_001602B8), 0xF0), add_offset((*(s32 *)&D_001602BC), 0x54), 0x80000000L,
                    text_buffer, -1);
    font_print_right_large(0xF0, 0x54, 0x80FFA888L, text_buffer, -1);
    func_00201640_1FB28(add_offset((*(s32 *)&D_001602B8), 0xD0), add_offset((*(s32 *)&D_001602BC), 0x4D),
                              add_offset((*(s32 *)&D_001602B8), 0xF2), add_offset((*(s32 *)&D_001602BC), 0x50),
                              0x80000000L, 0);
    func_00201640_1FB28(0xD0, 0x4D, 0xF2, 0x50, 0x80FFA888L, 0);
    DoGifPaging();
    return 8;
}

extern char D_00187040[];
/* 1.3f, in small data (gp -0x695C). The const float view is what the
   code reads (read-only, so its load is scheduled above the stores); the
   2-byte view, referenced last, makes the assembler use $gp. */
extern const float D_001603A4_f __asm__("D_001603A4") MACRO_ADDR;
extern short D_001603A4_s __asm__("D_001603A4");
extern char D_001603A8[];
extern int D_001E0B88[];
extern int D_0013E600[];
extern int func_00116248(char *, const char *, ...);
extern void func_001F6CF8_c(int, int, long, char *, int) __asm__("func_001F6CF8");

/* DrawGBsShipMenu (maybe): parks the marker object at arg0+0x44 by the
   camera focus in D_00187040, draws it, then prints "%s %d %s %d" (texts
   0x4F4F and 0x4F53, the number of D_0014BFC0 flags set for the current
   map, and the map's D_001E0B88 total) twice, as shadow and text. The
   flag count reads the map index inside its loop: PRE then computes
   D_001A01F0's %hi once, straight into the register that is kept for the
   second read, as retail has it (a pointer set before the loop leaves a
   copy). */
int func_0021FF80(char *arg0) {
    char buf[0x100];
    char *t = D_00187040;
    char *o = *(char **)(arg0 + 0x44);
    int count;

    *(float *)(o + 0x10) = *(float *)(t + 0x140) + 8.0f;
    *(float *)(o + 0x14) = *(float *)(t + 0x144) + D_001603A4_f;
    *(float *)(o + 0x18) = *(float *)(t + 0x148) - 0.1f;
    if (*(int *)(arg0 + 0x44) != 0) {
        DrawMobyList(*(int *)(arg0 + 0x44), 1);
    }
    SetupGifPaging(0);
    count = 0;
    {
        int k;
        for (k = 0; k < 4; k++) {
            if (D_0014BFC0[D_001A01F0[0x89] * 4 + k] != 0) {
                count = count + 1;
            }
        }
    }
    func_00116248(buf, D_001603A8, func_001FE540_id(0x4F4F), count,
                  func_001FE540_id(0x4F53), D_001E0B88[D_001A01F0[0x89]]);
    func_001F6CF8_c(*(int *)(arg0 + 0x20) - 0x10, (D_0013E600[1] >> 1) - 8,
                    0x80000000L, buf, -1);
    func_001F6CF8_c(*(int *)(arg0 + 0x20) - 0x11, (D_0013E600[1] >> 1) - 9,
                    0x80FFA888L, buf, -1);
    DoGifPaging();
    if (0) {
        /* no code: registers the 2-byte view last (see above) */
        (void)D_001603A4_s;
    }
    return 8;
}

extern float func_001FA748(float, float);

void func_00220128(void *arg0) {
    *(float *)((char *)arg0 + 0x40) =
        FastAddRots(*(float *)((char *)arg0 + 0x40), 0.02f);
}

/* func_00234C98's second parameter is 64-bit (as in draw.c and
   mobyfunc.c); the (int, int) declaration below this point is kept for
   the functions matched against it. */
extern void func_00234C98_l(int, long) __asm__("func_00234C98");
extern int func_00116248(char *, const char *, ...); /* sprintf */
extern int D_0013D530[];
extern char D_001E02B0[];
extern char D_001603A0[]; /* "%d" */
extern char D_001603B8[]; /* "%d/" */
extern char D_001603C0[]; /* "%d,%03d/" */
extern char D_001603D0[]; /* "%d,%03d" */

/* Draws the current item's count banner, gated like func_0021F118: the
   item's text id 0x4F52 when its D_001E02B0 record has no counter,
   otherwise "have/total" with thousands separators. The second sprintf
   appends at text + the first one's length; forming that pointer in each
   arm (`q = text + sprintf(...)`) lets gcc merge the two into one addu at
   the join, as retail has it. */
int func_00220160(char *arg0) {
    char text[0x50];
    short box[10];
    char *p = *(char **)(D_001D5F74 + 0x40);
    int id = *(short *)(*(int *)(p + 0x3C) * 10 + *(char **)(p + 0x48) + 6);
    int have, total;
    char *rec;

    if (((unsigned char *)&D_0013D5C8)[id] == 0) {
        return 0;
    }
    have = D_0013D530[id];
    rec = D_001E02B0 + id * 0x18;
    total = *(unsigned short *)(rec + 0xE);
    func_00234C98_l(0x42, 0x44);
    func_00234C98_l(0x47, 0xB);
    if (*(unsigned short *)(rec + 8) == 0) {
        func_00116248(text, func_001FE540_id(0x4F52));
    } else {
        char *q;
        if (have < 1000) {
            q = text + func_00116248(text, D_001603B8, have);
        } else {
            q = text + func_00116248(text, D_001603C0, have / 1000, have % 1000);
        }
        if (total < 1000) {
            func_00116248(q, D_001603A0, total);
        } else {
            func_00116248(q, D_001603D0, total / 1000, total % 1000);
        }
    }
    SetupGifPaging(0);
    func_00227A30(box, arg0);
    box[8] = 0x10;
    box[9] = 3;
    func_001F7560(box, 0x80FFA888L, text, -1);
    DoGifPaging();
    return 2;
}

extern void func_00234C98(int, int);
extern void func_00205E70(void);

int func_00220338(void) {
    VU1_addGSregister(0x42, 0x44);
    VU1_addGSregister(0x47, 0xB);
    func_00205E70();
    return 8;
}

extern void func_001F65A8(void);
extern void func_001F6598(void);
extern void func_001F6968_c(int, int, long, void *, int) __asm__("func_001F6968");

/* DrawMissionsMenu: the menu's text lines (0x4EEE, 0x4EEF only when
   D_0015EE84 is set, then 0x4EFA, 0x4EFB, 0x4EFC, 0x4EE0) in one column,
   left-aligned so the widest (func_001F6620) is centred in arg0+0x20 but
   at least 2 in, and spaced arg0+0x24 / (6 or 7 lines) apart starting a
   line minus 6 down. */
int func_00220370(char *arg0) {
    int maxw, w;
    int x, y, step;

    VU1_addGSregister(0x42, 0x44);
    VU1_addGSregister(0x47, 0xB);
    SetupGifPaging(0);
    maxw = func_001F6620(func_001FE540_id(0x4EEE), -1);
    w = func_001F6620(func_001FE540_id(0x4EFA), -1);
    if (w >= maxw) {
        maxw = w;
    }
    if (D_0015EE84 != 0) {
        w = func_001F6620(func_001FE540_id(0x4EEF), -1);
        if (w >= maxw) {
            maxw = w;
        }
    }
    w = func_001F6620(func_001FE540_id(0x4EFB), -1);
    if (w >= maxw) {
        maxw = w;
    }
    w = func_001F6620(func_001FE540_id(0x4EFC), -1);
    if (w >= maxw) {
        maxw = w;
    }
    w = func_001F6620(func_001FE540_id(0x4EE0), -1);
    if (w >= maxw) {
        maxw = w;
    }
    x = (*(int *)(arg0 + 0x20) - maxw) >> 1;
    if (x < 2) {
        x = 2;
    }
    step = *(int *)(arg0 + 0x24) / (D_0015EE84 != 0 ? 7 : 6);
    func_001F65A8();
    y = step - 6;
    func_001F6968_c(x, y, 0x80FFA888L, func_001FE540_id(0x4EEE), -1);
    if (D_0015EE84 != 0) {
        y += step;
        func_001F6968_c(x, y, 0x80FFA888L, func_001FE540_id(0x4EEF), -1);
    }
    y += step;
    func_001F6968_c(x, y, 0x80FFA888L, func_001FE540_id(0x4EFA), -1);
    y += step;
    func_001F6968_c(x, y, 0x80FFA888L, func_001FE540_id(0x4EFB), -1);
    y += step;
    func_001F6968_c(x, y, 0x80FFA888L, func_001FE540_id(0x4EFC), -1);
    y += step;
    func_001F6968_c(x, y, 0x80FFA888L, func_001FE540_id(0x4EE0), -1);
    func_001F6598();
    DoGifPaging();
    return 2;
}
__asm__(".section .text\n\tnop\n");

extern void func_001F4630(int);
extern void func_001F4748(void);
/* func_001F68E8 is defined above with pointer parameters; this site
   passes a packed 64-bit colour in $a2, and func_001FE540 above is
   declared (void) while retail's caller here passes an id in $a0 --
   reach both through aliases rather than redeclaring them. */
extern void func_001F68E8_c(int, int, long, void *, int)
    __asm__("func_001F68E8");
extern void *func_001FE540_id(int) __asm__("func_001FE540");

/* Sibling of func_00220338 above: the same two func_00234C98 setup
   calls, then two banner draws. 0x80FFA888 is spelled `long` (64-bit)
   so it builds via ori/dsll/ori rather than a sign-extending lui. */
int func_00220600(void) {
    VU1_addGSregister(0x42, 0x44);
    VU1_addGSregister(0x47, 0xB);
    SetupGifPaging(0);
    func_001F68E8_c(4, 7, 0x80FFA888L, func_001FE540_id(0x4EE0), -1);
    func_001F68E8_c(4, 0x17, 0x80FFA888L, func_001FE540_id(0x4F05), -1);
    DoGifPaging();
    return 2;
}

INCLUDE_ASM("asm/nonmatchings/text", func_00220690);

extern short D_001602B0;              /* SDA, gp -0x6A50 */
extern void func_00201640(int, int, int, int, long, long);
extern int func_00200198(int, int);
extern void func_00200468(int, int, int, int, int, int);

void func_002208F8(int x, int y, int flag) {
    func_00201640(x - 5, y - 5, x + 5, y + 5, 0x80FFA888L, 0);
    func_00201640(x - 4, y - 4, x + 4, y + 4, *(int *)&D_001602B0, 0);
    if (flag != 0) {
        int c = GetIconFrame(0xE99E, 1);
        HudSprite(c, x - 0xD, y - 0x12, 0x1E, 0x1E, 0x80);
    }
}

struct MenuScreen_209A0;

struct MenuPage_209A0 {
    s32 moby_anims[14];
    struct MenuPage_209A0 *back;
    s32 state;
    struct MenuScreen_209A0 *focus;
    struct MenuScreen_209A0 *screens[14];
    u8 pad_7C[0x4];
    struct MenuScreen_209A0 *pending_focus;
    s32 confirmed;
};

struct MenuSystem_209A0 {

    s32 state;
    struct MenuPage_209A0 *current;
    struct MenuPage_209A0 *next;
    s32 close_request;
    s32 unk10;
    s32 timer;
    s32 saved_texture_start;
    s32 current_gadget;
    u8 pad_20[0x10];
    s32 equipped[4];
    u8 pad_40[0x60];

    s32 stream_buffer[2];
    s32 unkA8;
    s32 unkAC;
    s32 unkB0[3];
    s32 read_offset;
    s32 unkC0;
    u8 pad_C4[0x4];
    u8 loaded_animation[2];
    u8 read_buffer_index;
    u8 pending_buffer;
    u32 streamed_animation_base;

    struct MenuPage_209A0 *previous;
    s32 unkD4;
    s32 unkD8;
    s32 unkDC;
    s32 unkE0;
    s32 unkE4;
    u8 pad_E8[0x4];
    s32 unkEC;
    struct MenuPage_209A0 *unkF0;
    s32 unkF4;
    s32 unkF8;

    s32 unkFC;
    s32 unk100;
    s32 unk104;
    s32 help_text_buffer;
    s32 unk10C;

    s32 update_count;
    u8 pad_114[0x4];

    s32 resource_table_toggle;
    s32 unk11C;
    s32 unk120;

    s32 close_locked;
    s32 card_op_pending;
    s32 card_op_text;
    u8 pad_130[0x4];
    s32 unk134;
    s32 unk138;
    s32 unk13C;
    s32 unk140;
    s32 last_resource_table_toggle;
    u8 pad_148[0xC];
    s32 unk154;               /* 0x154: PAL only */
};

extern struct MenuSystem_209A0 D_001D5F70_209A0 __asm__("D_001D5F70");

struct MenuGridCell_209A0 {
    u16 icon;
    s16 frame;
    s16 kind;
    s16 id;
    s16 stream_entry;
};

struct MenuItemGridData_209A0 {
    s32 flags;
    f32 margin_x;
    f32 margin_y;
    s32 selected_cell;
    s32 rows;
    s32 cols;
    struct MenuGridCell_209A0 *cells;
    struct MenuScreen_209A0 *up;
    struct MenuScreen_209A0 *down;
    struct MenuScreen_209A0 *left;
    struct MenuScreen_209A0 *right;
};

struct MenuSaveSlotData_209A0 {
    s32 flags;
    u8 pad_34[0xC];
    s32 slot;
    u8 pad_44[0x4];
    s32 save_data;
    s32 step;
};

struct MenuTextItem_209A0 {
    s16 text;
    s16 action;
    union {
        s32 value;
        struct {
            u16 lo;
            s16 hi;
        } half;
    } param;
    s16 subtext;
    s16 fade_timer;
};

struct MenuTextListData_209A0 {
    s32 flags;
    struct MenuTextItem_209A0 *items;
    struct MenuScreen_209A0 *up;
    struct MenuScreen_209A0 *down;
    s32 selected;
    s32 scroll;
};

struct MenuStreamEntry_209A0 {
    s32 sector;
    s32 sector_count;
};

struct MenuStreamData_209A0 {
    struct MenuStreamEntry_209A0 *entries;
    s32 flags;
    s32 texture_width;
    s32 texture_height;
    s32 *language_base;
    s32 state;
    s32 buffer[2];
    s32 loaded_entry[2];
    s32 fixed_entry;
    s32 elapsed_frames;
    s32 read_offset;
};

struct MenuScreen_209A0 {
    s32 (*update)(struct MenuScreen_209A0 *);
    u8 pad_4[0x4];
    void (*enter)(struct MenuScreen_209A0 *, s32);
    void (*leave)(struct MenuScreen_209A0 *, s32);
    s32 unk10;
    s32 moby;
    s32 x;
    s32 y;
    s32 width;
    s32 height;
    u8 pad_28[0x8];
    union {
        struct MenuItemGridData_209A0 grid;
        struct MenuSaveSlotData_209A0 save;
        struct MenuTextListData_209A0 list;
        struct MenuStreamData_209A0 stream;
    } data;
};

struct MapIcon_209A0;
struct MapMarker_209A0;
struct MapHdr_209A0;

struct MapState_209A0 {
    u8 pad0[0x8];
    s32 unk8;
    u8 *mask;
    u8 pad10[0x4];
    s32 unk14;
    s32 z;
    struct MapMarker_209A0 *markers;
    struct MapIcon_209A0 *icons;
    s32 unk24;
    s32 unk28;
    s32 unk2C;
    struct {
        s32 unk0;
        u8 pad4[0xC];
    } unk30[8];
    s32 marker_count;
    f32 zoom[20];
    s32 pan_x[20];
    s32 pan_y[20];
    u8 pad1A4[0x80];
    s32 level;
    s32 loaded;
    s32 unk22C;
    s32 unk230;
    s32 unk234;
    u8 pad238[0x4];
    struct MapHdr_209A0 *hdr;
    s32 map_image_vram;
    s32 tex_clut_vram;
    s32 tex0_vram;
    s32 tex1_vram;
    s32 tex2_vram;
    u8 pad254[0x4];
    s64 tex0;
    s64 tex1;
    s64 tex2;
    u8 pad270[0x8];
    s32 slot[5];
    s32 slot_id[5];
    s32 sel;
    s32 slot_size[5];
};

extern struct MapState_209A0 D_001A01F0_209A0 __asm__("D_001A01F0");

struct ClutImage_209A0 {
    u8 pad0[0x8];
    s32 width;
    s32 height;
    u8 pad10[0x10];
    u8 clut[0x400];
    u8 pixels[1];
};

/* Memory card state at D_0013D390, as far as this function reads it (PAL offsets). */
struct MemoryCardState_209A0 {
    u8 pad_0[0x20];
    struct {
        s32 unk0;             /* resource of the save in this slot, -1 none */
        u8 pad_4[0x18];
    } entries[5];             /* 0x20 */
    u8 pad_AC[0x30];
    s32 state;                /* 0xDC */
    s32 sub;
    s32 pending_state;        /* 0xE4 */
};

extern struct MemoryCardState_209A0 D_0013D390_209A0 __asm__("D_0013D390");


extern s32 func_001F9968_209A0(s32) __asm__("func_001F9968");
extern s64 func_00205660_209A0(s32, s32, u8 *, u8 *, s32, s32) __asm__("func_00205660");
extern void func_0020C2F8_209A0(void) __asm__("func_0020C2F8");
extern void func_0020C468_209A0(u8 *, s32) __asm__("func_0020C468");
extern s32 func_00217628_209A0(u8 *, s32, s32) __asm__("func_00217628");
extern s32 func_00227018_209A0(s32) __asm__("func_00227018");
extern s32 func_00227068_209A0(s32) __asm__("func_00227068");
extern s32 func_002270B0_209A0(s32) __asm__("func_002270B0");

/* Resource stream screen update for a single buffer: picks the entry from the screen's flags,
   streams it in, unpacks it and binds the image it holds as the map screen's first texture.
   Adapted from Lombyte (MIT) for PAL: src/ui/menus/fun_0021f990.c, FUN_0021f990. */
s32 func_002209A0(struct MenuScreen_209A0 *stream) {
    s32 flags;
    s32 idx;
    s32 r;
    s32 x;
    s32 y;
    u8 *p;
    struct ClutImage_209A0 *b;
    u8 *p20;
    u8 *p420;

    flags = stream->data.stream.flags;
    if (flags & 1) {
        idx = stream->data.stream.fixed_entry;
        if (idx == -1) {
            return 0;
        }
    } else if (flags & 2) {
        idx = D_001A0414_1BB90[0];
    } else if (flags & 4) {
        idx = D_001D5F70_209A0.current->focus->data.grid.selected_cell;
    } else if (flags & 0x100) {
        idx = D_001D5F70_209A0.current->focus->data.save.slot;
        if (idx <= -1) {
            idx = 0;
        }
        if (idx >= 5) {
            idx = 4;
        }
        if (D_0013D390_209A0.state < 3 && D_0013D390_209A0.pending_state < 0 && D_001D5F70_209A0.unk154 >= 0xB) {
            if (stream->data.stream.state == -1) {
                stream->data.stream.state = 0;
            }
            idx = D_0013D390_209A0.entries[idx].unk0;
        } else {
            stream->data.stream.state = -1;
        }
    } else {
        idx = D_001D5F70_209A0.current->focus->data.list.selected;
        if (idx <= -1) {
            idx = 0;
        }
    }

    switch (stream->data.stream.state) {
    case 0:
    case 2:
        if (idx == stream->data.stream.loaded_entry[0]) {
            break;
        }
        if (stream->data.stream.buffer[0] == 0) {
            break;
        }
        if (D_001517D8_1A1A0[0] != 0) {
            break;
        }
        if (stream->data.stream.entries[idx].sector_count == 0) {
            break;
        }
        p = (u8 *)stream->data.stream.buffer[0];
        if (stream->data.stream.flags & 0x20) {
            r = func_00227018_209A0(stream->data.stream.buffer[0]) -
                (stream->data.stream.entries[idx].sector_count << 11);
            stream->data.stream.read_offset = r;
            p += r;
        }
        if (stream->data.stream.flags & 0x10) {
            r = func_00217628_209A0(p, stream->data.stream.entries[idx].sector,
                                        stream->data.stream.entries[idx].sector_count);
        } else {
            r = func_00217628_209A0(p, stream->data.stream.entries[idx].sector,
                                        stream->data.stream.entries[idx].sector_count);
        }
        if (r != 0) {
            func_00227068_209A0(stream->data.stream.buffer[0]);
            stream->data.stream.loaded_entry[0] = idx;
            stream->data.stream.state++;
        } else {
            stream->data.stream.state = -1;
        }
        break;
    case 1:
    case 3:
        if (D_001517D8_1A1A0[0] != 0) {
            break;
        }
        func_002270B0_209A0(stream->data.stream.buffer[0]);
        if (stream->data.stream.flags & 0x20) {
            func_0020C468_209A0((u8 *)stream->data.stream.buffer[0] + stream->data.stream.read_offset,
                         stream->data.stream.buffer[0]);
            stream->data.stream.read_offset = 0;
        }
        b = (struct ClutImage_209A0 *)stream->data.stream.buffer[0];
        p20 = b->clut;
        p420 = b->pixels;
        x = func_001F9968_209A0(b->width);
        y = func_001F9968_209A0(b->height);
        D_001A01F0_209A0.tex0 = func_00205660_209A0(x, y, p20, p420, D_001A01F0_209A0.tex_clut_vram, D_001A01F0_209A0.tex0_vram);
        func_0020C2F8_209A0();
        stream->data.stream.state = 2;
        break;
    }
    return 0;
}

extern void func_001F5800(int, int, int, int, int, int, int, int, long,
                          long);
extern short D_00151880[];
extern long D_001A0448;

/* Eight register arguments ($a0-$a3, $t0-$t3) then two 64-bit stack
   slots -- both written with `sd`, so they are `long`, not `long long`
   (which would be 128-bit here). */
int func_00220C90(void *arg0) {
    if (*(int *)((char *)arg0 + 0x44) < 2) {
        return 0;
    }
    DrawTexturedQuad(0, 0, D_00151880[0xB0], D_00151880[0xB1], 0, 0,
                  *(int *)((char *)arg0 + 0x38),
                  *(int *)((char *)arg0 + 0x3C), 0x80808080L, D_001A0448);
    return 0x10;
}

extern int func_00226EA8(int);

/* Sibling of func_00220DA0 below: the same slot set (0x44/0x48/0x4C/
   0x50/0x54/0x5C) on the same object, seeded here instead of torn down.
   The tail stores go 5C, 50, 54. */
int func_00220D08(void *arg0) {
    char *s = (char *)arg0;

    *(int *)(s + 0x44) = 0;
    *(int *)(s + 0x48) = func_00226EA8(*(int *)(s + 0x34) & 0x200);
    *(int *)(s + 0x4C) = func_00226EA8(*(int *)(s + 0x34) & 0x200);
    if ((*(int *)(s + 0x34) & 0x200) == 0) {
        if (*(int *)(s + 0x48) == 0) {
            *(int *)(s + 0x48) = func_00226EA8(1);
        }
        if (*(int *)(s + 0x4C) == 0) {
            *(int *)(s + 0x4C) = func_00226EA8(1);
        }
    }
    *(int *)(s + 0x5C) = 0;
    *(int *)(s + 0x50) = -1;
    *(int *)(s + 0x54) = -1;
    return 0;
}

extern int func_00226F68(int);

int func_00220DA0(void *arg0) {
    char *s = (char *)arg0;
    *(int *)(s + 0x48) = func_00226F68(*(int *)(s + 0x48));
    *(int *)(s + 0x4C) = func_00226F68(*(int *)(s + 0x4C));
    *(int *)(s + 0x50) = -1;
    *(int *)(s + 0x54) = -1;
    *(int *)(s + 0x44) = -1;
    return 0;
}

struct MenuScreen_20DF0;

struct MenuPage_20DF0 {
    s32 moby_anims[14];
    struct MenuPage_20DF0 *back;
    s32 state;
    struct MenuScreen_20DF0 *focus;
    struct MenuScreen_20DF0 *screens[14];
    u8 pad_7C[0x4];
    struct MenuScreen_20DF0 *pending_focus;
    s32 confirmed;
};

struct MenuSystem_20DF0 {

    s32 state;
    struct MenuPage_20DF0 *current;
    struct MenuPage_20DF0 *next;
    s32 close_request;
    s32 unk10;
    s32 timer;
    s32 saved_texture_start;
    s32 current_gadget;
    u8 pad_20[0x10];
    s32 equipped[4];
    u8 pad_40[0x60];

    s32 stream_buffer[2];
    s32 unkA8;
    s32 unkAC;
    s32 unkB0[3];
    s32 read_offset;
    s32 unkC0;
    u8 pad_C4[0x4];
    u8 loaded_animation[2];
    u8 read_buffer_index;
    u8 pending_buffer;
    u32 streamed_animation_base;

    struct MenuPage_20DF0 *previous;
    s32 unkD4;
    s32 unkD8;
    s32 unkDC;
    s32 unkE0;
    s32 unkE4;
    u8 pad_E8[0x4];
    s32 unkEC;
    struct MenuPage_20DF0 *unkF0;
    s32 unkF4;
    s32 unkF8;

    s32 unkFC;
    s32 unk100;
    s32 unk104;
    s32 help_text_buffer;
    s32 unk10C;

    s32 update_count;
    u8 pad_114[0x4];

    s32 resource_table_toggle;
    s32 unk11C;
    s32 unk120;

    s32 close_locked;
    s32 card_op_pending;
    s32 card_op_text;
    u8 pad_130[0x4];
    s32 unk134;
    s32 unk138;
    s32 unk13C;
    s32 unk140;
    s32 last_resource_table_toggle;
    u8 pad_148[0xC];
    s32 unk154;               /* 0x154: PAL only */
};

extern struct MenuSystem_20DF0 D_001D5F70_20DF0 __asm__("D_001D5F70");

struct MenuGridCell_20DF0 {
    u16 icon;
    s16 frame;
    s16 kind;
    s16 id;
    s16 stream_entry;
};

struct MenuItemGridData_20DF0 {
    s32 flags;
    f32 margin_x;
    f32 margin_y;
    s32 selected_cell;
    s32 rows;
    s32 cols;
    struct MenuGridCell_20DF0 *cells;
    struct MenuScreen_20DF0 *up;
    struct MenuScreen_20DF0 *down;
    struct MenuScreen_20DF0 *left;
    struct MenuScreen_20DF0 *right;
};

struct MenuSaveSlotData_20DF0 {
    s32 flags;
    u8 pad_34[0xC];
    s32 slot;
    u8 pad_44[0x4];
    s32 save_data;
    s32 step;
};

struct MenuTextItem_20DF0 {
    s16 text;
    s16 action;
    union {
        s32 value;
        struct {
            u16 lo;
            s16 hi;
        } half;
    } param;
    s16 subtext;
    s16 fade_timer;
};

struct MenuTextListData_20DF0 {
    s32 flags;
    struct MenuTextItem_20DF0 *items;
    struct MenuScreen_20DF0 *up;
    struct MenuScreen_20DF0 *down;
    s32 selected;
    s32 scroll;
};

struct MenuStreamEntry_20DF0 {
    s32 sector;
    s32 sector_count;
};

struct MenuStreamData_20DF0 {
    struct MenuStreamEntry_20DF0 *entries;
    s32 flags;
    s32 texture_width;
    s32 texture_height;
    s32 *language_base;
    s32 state;
    s32 buffer[2];
    s32 loaded_entry[2];
    s32 fixed_entry;
    s32 elapsed_frames;
    s32 read_offset;
};

struct MenuScreen_20DF0 {
    s32 (*update)(struct MenuScreen_20DF0 *);
    u8 pad_4[0x4];
    void (*enter)(struct MenuScreen_20DF0 *, s32);
    void (*leave)(struct MenuScreen_20DF0 *, s32);
    s32 unk10;
    s32 moby;
    s32 x;
    s32 y;
    s32 width;
    s32 height;
    u8 pad_28[0x8];
    union {
        struct MenuItemGridData_20DF0 grid;
        struct MenuSaveSlotData_20DF0 save;
        struct MenuTextListData_20DF0 list;
        struct MenuStreamData_20DF0 stream;
    } data;
};

/* Memory card state at D_0013D390, as far as this function reads it (PAL offsets). */
struct MemoryCardState_20DF0 {
    u8 pad_0[0x8];
    s32 type;                 /* 0x08: card 0's type, 2 = a PS2 card */
    u8 pad_C[0x14];
    struct {
        s32 unk0;             /* resource of the save in this slot, -1 none */
        u8 pad_4[0x18];
    } entries[5];             /* 0x20 */
    u8 pad_AC[0x30];
    s32 state;                /* 0xDC */
    s32 sub;
    s32 pending_state;        /* 0xE4 */
};

extern struct MemoryCardState_20DF0 D_0013D390_20DF0 __asm__("D_0013D390");

typedef struct {
    s32 values[6];
} LanguageResourceOffsets_20DF0;
extern LanguageResourceOffsets_20DF0 D_001E8B00_20DF0 __asm__("D_001E8B00");

extern u8 D_0013D510_20DF0[] __asm__("D_0013D510");
extern s32 func_001F98C0_20DF0() __asm__("func_001F98C0");
extern s32 func_0020C468_20DF0() __asm__("func_0020C468");
extern s32 func_00217628_20DF0() __asm__("func_00217628");
/* Retail keeps both flag-selected call sites of this read helper; a second C name for
   the same function keeps the two calls from being cross-jumped. */
extern s32 func_00217628_20DF0b() __asm__("func_00217628");
extern s32 func_00227018_20DF0() __asm__("func_00227018");
extern s32 func_00227068_20DF0() __asm__("func_00227068");
extern s32 func_002270B0_20DF0() __asm__("func_002270B0");

/* Resource stream screen update: picks the entry to show from the screen's flags (fixed, the
   focus list or grid selection, the save slot's level, a timed cycle, the language), then runs
   the two-buffer read state machine that streams it in and unpacks it.
   Adapted from Lombyte (MIT) for PAL: src/textbin/fun_0021fdc8.c, update_menu_resource_stream. */
s32 func_00220DF0(struct MenuScreen_20DF0 *stream) {
    LanguageResourceOffsets_20DF0 language_offsets;
    s32 *completed_buffer_slot;
    s32 primary_entry_offset;
    s32 secondary_entry_offset;
    s32 replacement_entry_offset;
    s32 stream_flags;
    s32 slot_index;
    s32 primary_read_offset;
    s32 secondary_read_offset;
    s32 replacement_read_offset;
    s32 stream_state;
    s32 primary_buffer;
    s32 completed_buffer;
    s32 secondary_buffer;
    s32 replacement_buffer;
    s32 selection_flags;
    s32 resource_index;
    s32 primary_read_address;
    s32 secondary_read_address;
    s32 replacement_read_address;
    s32 primary_read_started;
    s32 secondary_read_started;
    s32 replacement_read_started;
    struct MenuStreamEntry_20DF0 *primary_entry;
    struct MenuStreamEntry_20DF0 *primary_entry_alternate;
    struct MenuStreamEntry_20DF0 *secondary_entry;
    struct MenuStreamEntry_20DF0 *secondary_entry_alternate;
    struct MenuStreamEntry_20DF0 *replacement_entry;
    struct MenuStreamEntry_20DF0 *replacement_entry_alternate;
    struct MenuScreen_20DF0 *item_selection;
    struct MenuScreen_20DF0 *group_selection;
    u8 *item_table_entry;

    selection_flags = stream->data.stream.flags;
    stream->data.stream.elapsed_frames = (s32)(stream->data.stream.elapsed_frames + 1);
    if (selection_flags & 1) {
        resource_index = stream->data.stream.fixed_entry;
        if (resource_index == -1) {
            return 0;
        }
        goto process_stream_state;
    } else {
        if (selection_flags & 2) {
            resource_index = D_001A0414_1BB90[0];
        } else if (selection_flags & 4) {
            resource_index = D_001D5F70_20DF0.current->focus->data.grid.selected_cell;
        } else if (selection_flags & 0x100) {
            resource_index = D_001D5F70_20DF0.current->focus->data.save.slot;
            if (resource_index < 0) {
                resource_index = 0;
            }
            if (resource_index >= 5) {
                resource_index = 4;
            }
            if (D_0013D390_20DF0.state < 3) {
                if (D_0013D390_20DF0.pending_state < 0 && D_001D5F70_20DF0.unk154 >= 0xB) {
                    if (stream->data.stream.state == -1) {
                        stream->data.stream.state = 0;
                    }
                    resource_index =
                        *(s32 *)((u8 *)&D_0013D390_20DF0 + resource_index * 0x1C + 0x20);
                    if (resource_index == -1) {
                        stream->data.stream.state = resource_index;
                    }
                    if (D_0013D390_20DF0.type != 2) {
                        stream->data.stream.state = -1;
                    }
                } else {
                    goto disable_stream;
                }
            } else {
                goto disable_stream;
            }
        } else if (selection_flags & 8) {
            group_selection = D_001D5F70_20DF0.current->focus;
            resource_index = (s32) * (s16 *)((u8 *)((group_selection->data.grid.selected_cell * 0xA) +
                                                    (s32)group_selection->data.grid.cells) +
                                             0x8);
            if (group_selection->data.grid.rows == 0) {
            disable_stream:
                stream->data.stream.state = -1;
            }
        } else if (selection_flags & 0x400) {
            resource_index = (s32)((s32)stream->data.stream.elapsed_frames / func_001F98C0_20DF0(0x12C)) % 19;
        } else if (selection_flags & 0x1000) {
            language_offsets = D_001E8B00_20DF0;
            resource_index = *stream->data.stream.language_base +
                             language_offsets.values[D_0015EE88_1BB90];
        } else {
            item_selection = D_001D5F70_20DF0.current->focus;
            resource_index = item_selection->data.list.selected;
            if (resource_index < 0) {
                resource_index = 0;
            }
            if (selection_flags & 0x4000) {
                item_table_entry = (u8 *)item_selection->data.list.items;
                item_table_entry += resource_index * 0xC;
                resource_index = (*(s16 *)(item_table_entry + 2) == 2) ? 9 : resource_index;
            }
        }
    process_stream_state:
        if (stream->data.stream.flags & 0x2000) {
            resource_index = (D_0013D510_20DF0[resource_index] == 0) ? 0x1E : resource_index;
        }
        stream_state = stream->data.stream.state;
        switch (stream_state) {
        case 0:
            primary_buffer = stream->data.stream.buffer[0];
            if (primary_buffer != 0 && D_001517D8_1A1A0[0] == 0) {
                primary_entry_offset = resource_index * 8;
                if (*(s32 *)((u8 *)(primary_entry_offset + (s32)stream->data.stream.entries) + 0x4) !=
                    0) {
                    primary_read_address = primary_buffer;
                    if (stream->data.stream.flags & 0x20) {
                        primary_read_offset =
                            func_00227018_20DF0(primary_buffer) -
                            (*(s32 *)((u8 *)(primary_entry_offset + (s32)stream->data.stream.entries) +
                                      0x4)
                             << 0xB);
                        stream->data.stream.read_offset = primary_read_offset;
                        primary_read_address += primary_read_offset;
                    }
                    if (stream->data.stream.flags & 0x10) {
                        primary_entry = (struct MenuStreamEntry_20DF0 *)(primary_entry_offset +
                                                                     (s32)stream->data.stream.entries);
                        primary_read_started = func_00217628_20DF0(
                            primary_read_address, primary_entry->sector, primary_entry->sector_count);
                    } else {
                        primary_entry_alternate =
                            (struct MenuStreamEntry_20DF0 *)(primary_entry_offset +
                                                         (s32)stream->data.stream.entries);
                        primary_read_started = func_00217628_20DF0b(
                            primary_read_address, primary_entry_alternate->sector,
                            primary_entry_alternate->sector_count);
                    }
                    if (primary_read_started != 0) {
                        func_00227068_20DF0(stream->data.stream.buffer[0]);
                        stream->data.stream.loaded_entry[0] = resource_index;
                        stream->data.stream.state = stream->data.stream.state + 1;
                    } else {
                        stream->data.stream.state = -1;
                    }
                }
            }
            break;
        case 1:
        case 3:
        case 5:
            if (D_001517D8_1A1A0[0] == 0) {
                stream_flags = stream->data.stream.flags;
                if (stream_flags & 0x20) {
                    completed_buffer_slot = &stream->data.stream.buffer[0];
                    slot_index = stream->data.stream.state == 3;
                    completed_buffer_slot += slot_index;
                    func_002270B0_20DF0(*completed_buffer_slot);
                    completed_buffer = *completed_buffer_slot;
                    func_0020C468_20DF0(completed_buffer + stream->data.stream.read_offset, completed_buffer);
                    stream->data.stream.read_offset = 0;
                }
                stream->data.stream.state = stream->data.stream.state + 1;
            }
            break;
        case 6:
            stream->data.stream.state = 2;
            /* fallthrough */
        case 2:
            if (resource_index != stream->data.stream.loaded_entry[0]) {
                if (resource_index == stream->data.stream.loaded_entry[1]) {
                    stream->data.stream.state = 4;
                } else {
                    secondary_buffer = stream->data.stream.buffer[1];
                    if (secondary_buffer == 0) {
                        stream->data.stream.state = 0;
                    } else if (D_001517D8_1A1A0[0] == 0) {
                        secondary_entry_offset = resource_index * 8;
                        if (*(s32 *)((u8 *)(secondary_entry_offset + (s32)stream->data.stream.entries) +
                                     0x4) != 0) {
                            secondary_read_address = secondary_buffer;
                            if (stream->data.stream.flags & 0x20) {
                                secondary_read_offset =
                                    func_00227018_20DF0(secondary_buffer) -
                                    (*(s32 *)((u8 *)(secondary_entry_offset +
                                                     (s32)stream->data.stream.entries) +
                                              0x4)
                                     << 0xB);
                                stream->data.stream.read_offset = secondary_read_offset;
                                secondary_read_address += secondary_read_offset;
                            }
                            if (stream->data.stream.flags & 0x10) {
                                secondary_entry =
                                    (struct MenuStreamEntry_20DF0 *)(secondary_entry_offset +
                                                                 (s32)stream->data.stream.entries);
                                secondary_read_started = func_00217628_20DF0(
                                    secondary_read_address, secondary_entry->sector,
                                    secondary_entry->sector_count);
                            } else {
                                secondary_entry_alternate =
                                    (struct MenuStreamEntry_20DF0 *)(secondary_entry_offset +
                                                                 (s32)stream->data.stream.entries);
                                secondary_read_started = func_00217628_20DF0b(
                                    secondary_read_address, secondary_entry_alternate->sector,
                                    secondary_entry_alternate->sector_count);
                            }
                            if (secondary_read_started != 0) {
                                func_00227068_20DF0(stream->data.stream.buffer[1]);
                                stream->data.stream.loaded_entry[1] = resource_index;
                                stream->data.stream.state = stream->data.stream.state + 1;
                            } else {
                                stream->data.stream.state = -1;
                            }
                        }
                    }
                }
            }
            break;
        case 4:
            if (resource_index != stream->data.stream.loaded_entry[1]) {
                if (resource_index == stream->data.stream.loaded_entry[0]) {
                    stream->data.stream.state = 2;
                } else {
                    replacement_buffer = stream->data.stream.buffer[0];
                    if (replacement_buffer == 0) {
                        stream->data.stream.state = -1;
                    } else if (D_001517D8_1A1A0[0] == 0) {
                        replacement_entry_offset = resource_index * 8;
                        if (*(s32 *)((u8 *)(replacement_entry_offset +
                                            (s32)stream->data.stream.entries) +
                                     0x4) != 0) {
                            replacement_read_address = replacement_buffer;
                            if (stream->data.stream.flags & 0x20) {
                                replacement_read_offset =
                                    func_00227018_20DF0(replacement_buffer) -
                                    (*(s32 *)((u8 *)(replacement_entry_offset +
                                                     (s32)stream->data.stream.entries) +
                                              0x4)
                                     << 0xB);
                                stream->data.stream.read_offset = replacement_read_offset;
                                replacement_read_address += replacement_read_offset;
                            }
                            if (stream->data.stream.flags & 0x10) {
                                replacement_entry =
                                    (struct MenuStreamEntry_20DF0 *)(replacement_entry_offset +
                                                                 (s32)stream->data.stream.entries);
                                replacement_read_started = func_00217628_20DF0(
                                    replacement_read_address, replacement_entry->sector,
                                    replacement_entry->sector_count);
                            } else {
                                replacement_entry_alternate =
                                    (struct MenuStreamEntry_20DF0 *)(replacement_entry_offset +
                                                                 (s32)stream->data.stream.entries);
                                replacement_read_started = func_00217628_20DF0b(
                                    replacement_read_address,
                                    replacement_entry_alternate->sector,
                                    replacement_entry_alternate->sector_count);
                            }
                            if (replacement_read_started != 0) {
                                func_00227068_20DF0(stream->data.stream.buffer[0]);
                                stream->data.stream.loaded_entry[0] = resource_index;
                                stream->data.stream.state = stream->data.stream.state + 1;
                            } else {
                                stream->data.stream.state = -1;
                            }
                        }
                    }
                }
            }
            break;
        default:
            break;
        }
    }
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/text", func_00221380);

/* The per-controller request slots in D_00137C80 (declared above as a
   flat int array). */
typedef struct { int a, b; } Pair8;
typedef struct {
    char pad[0x2C8];
    Pair8 req0[6]; /* 0x2C8 */
    Pair8 req2[6]; /* 0x2F8 */
} Tbl137C80;
extern Tbl137C80 D_00137C80_t __asm__("D_00137C80");
extern int D_0015EE88 MACRO_ADDR;

/* Pause sub-state machine at arg0+0x44. States 0 and 2 wait for their
   request (arg0+0x48 / +0x4C) and for D_001517D0[4] to clear, then run
   func_00217628 with the controller's slot and advance (or go to -1 on
   failure); states 1 and 3 advance once D_001517D0[4] clears. Always
   returns 0. A switch with its cases in source order 0-3 gives retail's
   case tree and block order; reading the slot's two words as fields of a
   struct gives two address adds (one becomes retail's copy). */
int func_00221688(char *arg0) {
    switch (*(int *)(arg0 + 0x44)) {
    case 0:
        if (*(int *)(arg0 + 0x48) != 0 && D_001517D0[4] == 0) {
            if (func_00217628_3(*(int *)(arg0 + 0x48),
                                D_00137C80_t.req0[D_0015EE88].a,
                                D_00137C80_t.req0[D_0015EE88].b) != 0) {
                *(int *)(arg0 + 0x44) += 1;
            } else {
                *(int *)(arg0 + 0x44) = -1;
            }
        }
        break;
    case 1:
        if (D_001517D0[4] == 0) {
            *(int *)(arg0 + 0x44) = 2;
        }
        break;
    case 2:
        if (*(int *)(arg0 + 0x4C) != 0 && D_001517D0[4] == 0) {
            if (func_00217628_3(*(int *)(arg0 + 0x4C),
                                D_00137C80_t.req2[D_0015EE88].a,
                                D_00137C80_t.req2[D_0015EE88].b) != 0) {
                *(int *)(arg0 + 0x44) += 1;
            } else {
                *(int *)(arg0 + 0x44) = -1;
            }
        }
        break;
    case 3:
        if (D_001517D0[4] == 0) {
            *(int *)(arg0 + 0x44) = 4;
        }
        break;
    }
    return 0;
}

/* Returns the packed texture handle the draw call takes as its last
   64-bit argument, so it is `long`: retail stores $v0 straight to the
   stack slot with `sd`, without sign-extending it. */
extern long func_00205520(int);

/* Sibling of func_00220C90 above: same 0x44 guard and the same
   func_001F5800 draw, twice, with the colour held in one local because
   retail keeps it in a callee-saved register across both calls. */
int func_002217C8(void *arg0) {
    long c;

    if (*(int *)((char *)arg0 + 0x44) < 4) {
        return 0;
    }
    c = 0x80808080L;
    SetupGifPaging(0);
    DrawTexturedQuad(0, 0, 0x100, 0x100, 0, 0, 0x100, 0x100, c,
                  func_00205520(*(int *)((char *)arg0 + 0x48)));
    DrawTexturedQuad(0x100, 0, 0x100, 0x100, 0, 0, 0x100, 0x100, c,
                  func_00205520(*(int *)((char *)arg0 + 0x4C)));
    DoGifPaging();
    return 8;
}

extern char D_001A01F0_c[] __asm__("D_001A01F0");
extern char D_00151880_c[] __asm__("D_00151880");
extern int D_0015F538 MACRO_ADDR;
extern int D_001DE2E8[];
extern void func_00200CA0(long, int, int, int, int, int, int, int, int, int);

/* Draws the three overlay layers (textures at D_001A01F0 +0x258/0x260/
   0x268) over the whole screen (D_00151880's +0x160/+0x162 extent).
   States 6, 13 and 17 blink the upper two layers on D_0015F538; the
   others inset them by D_001DE2E8[state] and scroll the bottom one.
   Each branch reaches the two globals through its own block-scoped
   pointers, which gives retail's registers. */
int func_00221888(void) {
    char *g = D_001A01F0_c;
    int s = *(int *)(g + 0x228);
    int k, d, u, t;

    if (s < 0) {
        return 0;
    }
    if (s == 6 || s == 13 || s == 17) {
        char *g2;
        char *fb;

        func_00234C98_l(0x47, 0);
        func_00234C98_l(8, 5);
        g2 = D_001A01F0_c;
        fb = D_00151880_c;
        func_00200CA0(*(long *)(g2 + 0x258), 0, 0, 7, 7,
                      *(short *)(fb + 0x160) << 4, *(short *)(fb + 0x162) << 4, 0, 0, 0x80);
        func_00234C98_l(0x47, 0x360B);
        if (D_0015F538 % 60 < 40) {
            func_00200CA0(*(long *)(g2 + 0x260), 0, 0, 7, 7,
                          *(short *)(fb + 0x160) << 4, *(short *)(fb + 0x162) << 4, 0, 0, 0x80);
        }
        if (D_0015F538 % 150 < 90) {
            func_00200CA0(*(long *)(g2 + 0x268), 0, 0, 7, 7,
                          *(short *)(fb + 0x160) << 4, *(short *)(fb + 0x162) << 4, 0, 0, 0x80);
        }
    } else {
        char *fb;

        t = D_0015F538 + s * 0x2AB;
        k = D_001DE2E8[s];
        d = k * 2;
        u = t % 2048;
        func_00234C98_l(0x47, 0);
        func_00234C98_l(8, 0);
        fb = D_00151880_c;
        func_00200CA0(*(long *)(g + 0x258), k, k, 7, 7,
                      (*(short *)(fb + 0x160) << 4) - d, (*(short *)(fb + 0x162) << 4) - d, u, 0, 0x80);
        func_00234C98_l(0x47, 0x360B);
        func_00234C98_l(8, 5);
        func_00200CA0(*(long *)(g + 0x260), k, k, 7, 7,
                      (*(short *)(fb + 0x160) << 4) - d, (*(short *)(fb + 0x162) << 4) - d, 0, 0, 0x80);
        func_00200CA0(*(long *)(g + 0x268), k, k, 7, 7,
                      (*(short *)(fb + 0x160) << 4) - d, (*(short *)(fb + 0x162) << 4) - d, 0, 0, 0x80);
    }
    return 4;
}

extern float func_001FA7D8(float, int);
extern int func_001F9B70(int); /* abs */

extern char D_001864D0[];
extern int D_0015F538 MACRO_ADDR;
extern char D_001603D8[];
extern char D_001603E0[];
extern short D_001602B0; /* SDA, gp -0x6A50 */

/* The icon slots are a real member: written as pointer arithmetic, gcc
   strength-reduces the index into a 7th saved register (retail uses 6). */
typedef struct {
    char pad[0x30];
    int slots[8];
} PauseIcons;

/* Draws the pause menu's ring of 8 icons and the quick-select overlay.
   arg0 is the pause-state object: unk20/unk24 give the viewport
   width/height (used for the ring's center and radius), unk50 the
   selected slot (drawn with a pulsing highlight box), and slots[i]
   indexes an icon-info table (D_001864D0, 0x4C bytes/entry) and a byte
   flags table (D_0013E620) when nonzero. */
int func_00221B58(char *arg0) {
    int v0, v1;
    int flag;
    float cx, cy, radius;
    char *iconTab;
    int i;

    SetupGifPaging(0);
    v1 = *(int *)(arg0 + 0x20);
    iconTab = D_001864D0;
    v0 = *(int *)(arg0 + 0x24);
    flag = v1 < v0;
    /* v1 doubles as `selected`: retail's movz reuses the same register
       that already holds v1 (loaded straight into it), so the C must
       overwrite v1 itself rather than assign a fresh local -- that
       fresh local costs an extra `move` before the conditional one. */
    cx = (float)v1 * 0.5f;
    cy = (float)v0 * 0.5f;
    if (flag == 0) {
        v1 = v0;
    }
    radius = (float)v1 * 0.5f - 40.0f;
    i = 0;
    do {
        float ang, t, x, y;
        int idx;

        ang = (float)i * 0.7853982f + -1.5707964f;
        t = FastNormalizeAngle(ang, flag);
        x = cx + FastCos(t) * radius;
        y = cy + FastSin(t) * radius;
        if (i == *(int *)(arg0 + 0x50)) {
            unsigned int color = ((func_001F9B70((D_0015F538 & 0x3F) - 0x20) + 0x40) * 0x10202) | 0x80000000;
            func_00201640((int)x - 0x13, (int)y - 0x13, (int)x + 0x13, (int)y + 0x13, color, 0);
            func_00201640((int)x - 0x12, (int)y - 0x12, (int)x + 0x12, (int)y + 0x12, *(int *)&D_001602B0, 0);
        }
        idx = ((PauseIcons *)arg0)->slots[i];
        if (idx == 0) {
            func_00201640((int)x - 0xF, (int)y - 0xF, (int)x + 0xF, (int)y + 0xF, 0x40404040L, 0);
        } else {
            int id = GetIconFrame(*(unsigned short *)(iconTab + idx * 0x4C + 0x38), D_0013E620[idx] == 0 ? 0 : 4);
            HudSprite(id, (int)x - 0x11, (int)y - 0x11, 0x20, 0x20, 0x80);
        }
        i += 1;
        flag = 0xE99E;
    } while (i < 8);
    HudSprite(GetIconFrame(0xE99E, 0), 8, 0x27, 0x20, -0x20, 0x80);
    HudSprite(GetIconFrame(0xE99E, 0), *(int *)(arg0 + 0x20) - 0xA, 0x27, -0x20, -0x20, 0x80);
    func_001F68E8_c(0x28, 0xF, 0x80FFA888L, D_001603D8, -1);
    func_001F68E8_c(*(int *)(arg0 + 0x20) - 0x3C, 0xF, 0x80FFA888L, D_001603E0, -1);
    DoGifPaging();
    return 2;
}

extern void func_0022ED80(int, int, int);
extern void func_001FBC80(int, void *, int);
extern int func_001F98C0(int);
extern void func_001F4E08(int);
extern float D_0015F53C MACRO_ADDR;
extern int D_0016044C_i __asm__("D_0016044C") MACRO_ADDR;
typedef struct {
    int w0;              /* 0x00 */
    unsigned char *buf;  /* 0x04 */
    int e[2];            /* 0x08 */
    int flags;           /* 0x10 */
} Row14;

/* Pad handler for a row list of toggles (func_002222F8's sibling, one
   `char *` pad local per block). While the timer at arg0+0x3C runs it
   counts down and sets D_0015F53C to min(t, 4) / 4. Otherwise, for the
   active item: 0xD00/0x10 leave, 0x1000/0x4000 move the cursor with a
   notify on change, and 0x40 flips the current row's byte, or for a row
   with flag 1 starts the timer and flips D_0016044C (func_001FBC80 once
   it is set). The row is indexed directly at each use (index-first addu)
   and the cursor compare is written new != old, as retail's beq has it. */
int func_00221E60(char *arg0) {
    char *g = D_001D5F70;
    int sel = *(char **)(*(char **)(g + 4) + 0x40) == arg0;

    if (*(int *)(arg0 + 0x3C) != 0) {
        int t = *(int *)(arg0 + 0x3C) - 1;
        *(int *)(arg0 + 0x3C) = t;
        D_0015F53C = (float)(t < 5 ? t : 4) * 0.25f;
        return 0;
    }
    if (!sel) {
        return 0;
    }
    {
        char *pad = D_0013CA40;
        if (*(int *)(pad + 0x1C4) & 0xD00) {
            if (*(int *)(g + 0x124) == 0) {
                return 1;
            }
        }
    }
    {
        char *pad2 = D_0013CA40;
        if (*(int *)(pad2 + 0x1C4) & 0x10) {
            char *g2 = D_001D5F70;
            int t = *(int *)(*(char **)(g2 + 4) + 0x38);
            if (t != 0) {
                *(int *)(g2 + 8) = t;
                return 0;
            }
            if (*(int *)(g2 + 0x124) == 0) {
                return -1;
            }
        }
    }
    {
        char *pad3 = D_0013CA40;
        int old = *(int *)(arg0 + 0x38);
        if ((*(int *)(pad3 + 0x1C4) & 0x1000) && old != 0) {
            *(int *)(arg0 + 0x38) = old - 1;
        }
        {
            char *pad4 = D_0013CA40;
            if (*(int *)(pad4 + 0x1C4) & 0x4000) {
                int c = *(int *)(arg0 + 0x38);
                if ((*(Row14 **)(arg0 + 0x34))[c + 1].w0 != 0) {
                    *(int *)(arg0 + 0x38) = c + 1;
                }
            }
        }
        {
            char *pad5 = D_0013CA40;
            if (*(int *)(pad5 + 0x1C4) & 0x40) {
                func_0022ED80(0, 0x11, *(int *)(arg0 + 0x14));
                if ((*(Row14 **)(arg0 + 0x34))[*(int *)(arg0 + 0x38)].flags & 1) {
                    if (*(unsigned char *)&D_0016044C_i) {
                        char *g3 = D_001D5F70;
                        func_001FBC80(6, *(void **)(g3 + 4), 0);
                    } else {
                        FadeToBlack(4);
                        *(int *)(arg0 + 0x3C) = func_001F98C0(0x10);
                        *(unsigned char *)&D_0016044C_i = !*(unsigned char *)&D_0016044C_i;
                    }
                } else {
                    unsigned char *buf = (*(Row14 **)(arg0 + 0x34))[*(int *)(arg0 + 0x38)].buf;
                    if (buf != 0) {
                        buf[0] = !buf[0];
                    }
                }
            }
        }
        if (*(int *)(arg0 + 0x38) != old) {
            func_0022ED80(1, 0x11, *(int *)(arg0 + 0x14));
        }
    }
    return 0;
}
__asm__(".section .text\n\tnop\n");

typedef struct TextRegion FontWindow;
struct CheatMenuEntry {
    s32 text_id;
    u8 *enabled_flag;
    s32 enabled_text_id;
    s32 disabled_text_id;
    s32 reserved10;
};
struct CheatsMenu {
    u8 pad0[0x20];
    s32 width;
    s32 height;
    u8 pad28[8];
    s32 flags;
    struct CheatMenuEntry *entries;
    s32 selected_entry;
};
extern void func_00234C98_22070(s32, u64) __asm__("func_00234C98");
extern void func_001153FC_22070(void *, s32, u32) __asm__("func_001153FC");
extern char *func_001FE540_22070(s32) __asm__("func_001FE540");
extern void func_001F7560_22070(FontWindow *, u64, char *, s32) __asm__("func_001F7560");
extern void func_001F68E8_22070(s32, s32, u64, char *, s32) __asm__("func_001F68E8");

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/textbin/ui/menus/draw_cheats_menu.c, draw_cheats_menu. */
s32 func_00222070(struct CheatsMenu *menu) {
    FontWindow text_window;
    s16 window_fields[12];
    struct CheatMenuEntry *entry;
    s32 entry_count;
    s32 entry_index;
    s32 draw_index;
    s32 draw_y;
    s32 line_spacing;
    s32 color;
    s32 enabled;

    func_00234C98_22070(0x47, 0x2004B);
    SetupGifPaging(0);
    if ((menu->flags & 1) && menu->entries->text_id == 0) {
        func_001153FC_22070(window_fields, 0, sizeof(window_fields));
        window_fields[1] = menu->height + 1;
        window_fields[3] = menu->width + 1;
        window_fields[4] = menu->width >> 1;
        window_fields[0] = 1;
        window_fields[2] = 1;
        window_fields[5] = menu->height / 3;
        window_fields[8] = 16;
        window_fields[9] = 1;

        text_window = *(FontWindow *)window_fields;
        func_001F7560_22070(&text_window, 0x80FFA888, func_001FE540_22070(0x4FC0), -1);
    }
    /* Entries end at a zero text ID; each retail entry occupies 0x14 bytes. */
    entry_count = 0;
    while (menu->entries[entry_count].text_id != 0) {
        entry_count++;
    }
    line_spacing = menu->height / (entry_count + 1);
    draw_y = menu->height / (entry_count + 1) - 8;
    entry_index = 0;
    draw_index = 0;
    if (menu->entries[0].text_id != 0) {
        do {
            entry = &menu->entries[draw_index];
            color = entry_index == menu->selected_entry ? 0x8020FFFF : 0x80FFA888;
            enabled = 0;
            if (entry->enabled_flag != 0) {
                enabled = *entry->enabled_flag;
            }
            func_001F68E8_22070(0xC, draw_y, color, func_001FE540_22070(entry->text_id), -1);
            font_print_right(
                menu->width - 0xC, draw_y, 0x80FFA888,
                func_001FE540_22070(enabled ? entry->enabled_text_id : entry->disabled_text_id),
                -1);
            draw_y += line_spacing;
            draw_index++;
            entry_index++;
        } while (menu->entries[entry_index].text_id != 0);
    }
    DoGifPaging();
    return 2;
}

extern void func_0022ED80(int, int, int);
typedef struct {
    int w0;              /* 0x00 */
    unsigned char *buf;  /* 0x04 */
    int e[4];            /* 0x08 */
} Row18;

/* Pad handler for a row list (func_0021ACD8's gate, one `char *` pad
   local per block): the cursor at arg0+0x38 moves down on 0x1000 and up
   on 0x4000 while the next row is used, with a func_0022ED80 notify on
   change; the current row's used entries (up to 4) are counted, and 0x40
   cycles the row's byte selector through them. The row is indexed with a
   fresh read of the cursor after the notify (reorg skips that load on the
   no-notify path, where the compare already left it in $v0), and the
   count walks an `int *e = rec->e` set before its first test, which puts
   `rec + 8` in the test's delay slot as retail does. */
int func_002222F8(char *arg0) {
    char *g = D_001D5F70;

    if (*(char **)(*(char **)(g + 4) + 0x40) != arg0) {
        return 0;
    }
    {
        char *pad = D_0013CA40;
        if (*(int *)(pad + 0x1C4) & 0xD00) {
            if (*(int *)(g + 0x124) == 0) {
                return 1;
            }
        }
    }
    {
        char *pad2 = D_0013CA40;
        if (*(int *)(pad2 + 0x1C4) & 0x10) {
            char *g2 = D_001D5F70;
            int t = *(int *)(*(char **)(g2 + 4) + 0x38);
            if (t != 0) {
                *(int *)(g2 + 8) = t;
                return 0;
            }
            if (*(int *)(g2 + 0x124) == 0) {
                return -1;
            }
        }
    }
    {
        char *pad3 = D_0013CA40;
        int old = *(int *)(arg0 + 0x38);
        if ((*(int *)(pad3 + 0x1C4) & 0x1000) && old != 0) {
            *(int *)(arg0 + 0x38) = old - 1;
        }
        {
            char *pad4 = D_0013CA40;
            if (*(int *)(pad4 + 0x1C4) & 0x4000) {
                int c = *(int *)(arg0 + 0x38);
                if ((*(Row18 **)(arg0 + 0x34))[c + 1].w0 != 0) {
                    *(int *)(arg0 + 0x38) = c + 1;
                }
            }
        }
        if (old != *(int *)(arg0 + 0x38)) {
            func_0022ED80(1, 0x11, *(int *)(arg0 + 0x14));
        }
    }
    {
        Row18 *rec = &(*(Row18 **)(arg0 + 0x34))[*(int *)(arg0 + 0x38)];
        int n = 0;
        char *pad5;
        int *e;

        e = rec->e;
        if (e[0] != 0) {
            do {
                n++;
            } while (e[n] != 0 && n < 4);
        }
        pad5 = D_0013CA40;
        if (*(int *)(pad5 + 0x1C4) & 0x40) {
            unsigned char *buf = rec->buf;
            if (buf != 0) {
                buf[0] = (buf[0] + 1) % n;
                func_0022ED80(0, 0x11, *(int *)(arg0 + 0x14));
            }
        }
    }
    return 0;
}

struct OptItem { int text; unsigned char *value; int names[4]; };
struct OptMenu { unsigned char pad0[0x20]; int x; int height; unsigned char pad28[0xC]; struct OptItem *items; int selected; };

/* Draws an options menu: each item's label at left, its current value's name at right, selected one highlighted. Adapted from Lombyte (MIT) for PAL: src/ui/menus/fun_00221460.c, FUN_00221460. */
int func_002224A8(struct OptMenu *m) {
    struct OptItem *p;
    struct OptItem *it;
    int n;
    int i;
    int step;
    int y;
    int color;

    func_00234C98_l(0x47, 0x2004B);
    SetupGifPaging(0);
    n = 0;
    for (p = m->items; p->text != 0; p++) {
        n++;
    }
    step = m->height / (n + 1);
    p = m->items;
    y = step - 8;
    for (i = 0; m->items[i].text != 0; i++) {
        it = &m->items[i];
        if (i == m->selected) {
            color = 0x8020FFFF;
        } else {
            color = 0x80FFA888;
        }
        func_001F68E8_c(0xC, y, color, func_001FE540_id(it->text), -1);
        func_001F6CF8_c(m->x - 0xC, y, 0x80FFA888, func_001FE540_id(it->names[*it->value]), -1);
        y += step;
    }
    DoGifPaging();
    return 2;
}

typedef struct {
    unsigned short a;   /* +0 */
    short b;            /* +2 */
    short c;            /* +4 */
    short id;           /* +6 */
    short idx;          /* +8 */
} Item0A;
extern Item0A D_001CF4A0[];
extern Item0A D_001D6470[];
extern int D_001D6508[];
extern char D_001864D0[];
extern char D_001D1080[];

/* Builds the pause-menu item list from the 15 entries of D_001CF4A0
   that D_0013D5C8 enables. The flag needs its own `f = b != 0`:
   `(b != 0) << 2` folds into a branch. */
int func_00222640(void) {
    int i;
    int n = 0;
    char *m;

    for (i = 0; i < 15; i++) {
        int id = D_001CF4A0[i].id;
        if (((unsigned char *)&D_0013D5C8)[id] != 0) {
            Item0A *out = &D_001D6470[n];
            char *rec = D_001864D0 + id * 0x4C;
            unsigned char b = D_0013E620[id];
            int f = b != 0;
            out->a = *(unsigned short *)(rec + 0x38);
            out->b = f << 2;
            out->c = 0;
            out->id = id;
            out->idx = i;
            D_001D6508[n] = b ? *(short *)(rec + 0x42) : *(short *)(rec + 0x40);
            n++;
        }
    }
    m = D_001D1080;
    *(int *)(m + 0x40) = n;
    return 0;
}

extern Item0A D_001CEFA0[];
extern Item0A D_001CEFE0[];
extern Item0A D_001CF000[];
extern Item0A D_001CF020[];
extern Item0A D_001D6548[];
extern int D_001D65D8[];
extern int D_001D6610[];
extern char D_001864D0[];
extern char D_001D1408[];

/* func_00222640's sibling over four source tables. */
int func_00222708(void) {
    int i;
    int id;
    int n = 0;
    char *m;

    for (i = 0; i < 14; i++) {
        if (i < 6) {
            id = D_001CEFA0[i].id;
        } else if (i < 9) {
            id = D_001CEFE0[i - 6].id;
        } else if (i < 12) {
            id = D_001CF000[i - 9].id;
        } else {
            id = D_001CF020[i - 12].id;
        }
        if (((unsigned char *)&D_0013D5C8)[id] != 0) {
            char *rec = D_001864D0 + id * 0x4C;
            D_001D6548[n].a = *(unsigned short *)(rec + 0x38);
            D_001D6548[n].b = 0;
            D_001D6548[n].c = 0;
            D_001D6548[n].id = id;
            D_001D6548[n].idx = i;
            D_001D65D8[n] = *(short *)(rec + 0x40);
            D_001D6610[n] = *(short *)(rec + 0x44);
            n++;
        }
    }
    m = D_001D1408;
    *(int *)(m + 0x40) = n;
    return 0;
}

int func_00222840(void) {
    return 0;
}

typedef struct {
    unsigned short a;   /* +0 */
    short b;            /* +2 */
    int c;              /* +4 */
    unsigned short d;   /* +8 */
    short e;            /* +A */
} Out0C;
typedef struct {
    unsigned short a;   /* +0 */
    short pad2;
    unsigned short b;   /* +4 */
    short pad6;
    int pad8;
} Src0C;
extern int D_0013D618[];
extern Src0C D_001DE0C0[];
extern Out0C D_001D6648[];
extern short D_001602E0_s __asm__("D_001602E0");
extern int *D_001602E0_m __asm__("D_001602E0") MACRO_ADDR;

typedef struct {
    char pad00[0x30];
    int flags;          /* +0x30 */
    char pad34[0xC];
    int sel;            /* +0x40 */
} MenuObj40;

/* Menu builder: the item list from D_0013D618, then the selection. The
   scalar MACRO_ADDR view of D_001602E0 lets sched2 hoist its load, and
   the dead reference to the 2-byte view keeps `.extern D_001602E0, 2`
   last, so the assembler still uses $gp. A later 4-byte reference in
   this file would undo that. */
int func_00222848(MenuObj40 *arg0) {
    int i;

    for (i = 0; i < 20 && gGalaxyList[i] != 0; i++) {
        D_001D6648[i].a = *(unsigned short *)((char *)D_001DE0C0 + gGalaxyList[i] * 12);
        D_001D6648[i].b = 1;
        D_001D6648[i].c = 0;
        D_001D6648[i].d = *(unsigned short *)((char *)D_001DE0C0 + gGalaxyList[i] * 12 + 4);
    }
    D_001D6648[i].a = 0;
    arg0->sel = 0;
    arg0->flags |= 0x8000;
    for (i = 0; D_001602E0_m[i] != 0; i++) {
        char *slots = (char *)D_001A01F0;
        if (*(int *)(slots + 0x224) == D_001602E0_m[i]) {
            arg0->sel = i;
            break;
        }
    }
    if (0) {
        /* no code: registers the 2-byte view last, so the file ends with
           `.extern D_001602E0, 2` and the assembler uses $gp */
        (void)D_001602E0_s;
    }
    return 0;
}

extern int D_0013CC04 NOT_SDA;
extern char D_001D2678[];
extern char *D_001D5F78 NOT_SDA;

int func_00222950(void) {
    if (D_0013CC04 & 0x40) {
        D_001D5F78 = D_001D2678;
    }
    return 0;
}


int func_00222978(void *arg0) {
    int *p = (int *)arg0;
    func_001FDF78(p[6], p[6] + p[8], p[7], p[7] + p[9]);
    return 2;
}

extern void func_0022ED80(int, int, int);

/* Pause sub-menu input on the pad word D_0013CC04. One if/else-if chain
   falling to a single `return 0`; early returns in the 0x10 arm give a
   movn instead. */
int func_002229B0(char *arg0) {
    int v = D_0013CC04;

    if (v & 0x10) {
        char *g = D_001D5F70;
        int t = *(int *)(*(char **)(g + 4) + 0x38);
        if (t != 0) {
            *(int *)(g + 8) = t;
        } else if (*(int *)(g + 0x124) == 0) {
            return -1;
        }
    } else if (v & 0x800) {
        D_001A0414 = D_0015EE84;
        return 1;
    } else if (v & 0x40) {
        func_0022ED80(0, 0x11, *(int *)(arg0 + 0x14));
        return 1;
    } else if (v & 0x20) {
        char *g = D_001D5F70;
        *(int *)(g + 0xE4) = D_001A0414;
        *(int *)(g + 0xF0) = *(int *)(g + 4);
        *(int *)(g + 0xC) = 3;
        *(int *)(g + 0xF4) = 0xF;
        func_0022ED80(0, 0x11, *(int *)(arg0 + 0x14));
    }
    return 0;
}

extern int func_00226EA8(int);
extern char *D_001D5F74 NOT_SDA;

int func_00222A90(void *arg0) {
    *(int *)(D_001D5F74 + 0x84) = 0;
    *(int *)((char *)arg0 + 0x54) = func_00226EA8(0);
    return 0;
}

extern int func_00226F68(int);

int func_00222AD0(void *arg0) {
    *(int *)((char *)arg0 + 0x54) = func_00226F68(*(int *)((char *)arg0 + 0x54));
    return 0;
}

/*
 * Near-miss, same size (4 words differ, allocator only): in the default
 * arm retail keeps the D_001D5F70 base in $v1 (the register that held
 * D_0015EFB0) with the loaded pointer in $a0 and the value in $a1; we
 * get base $a1, pointer $a0, value $v1. Spellings tried (asm-differ
 * score, lower is better; all 0x94 bytes):
 *   this one (if/!=, base local per arm)                       25
 *   base, pointer and value as separate locals in the arm      45
 *   la-macro alias (MACRO_ADDR) for the base in the arm        235
 *   switch with default first and break                        2430
 * D_0013CBE4 is NOT a macro access: retail splits its lui into the
 * second beq's delay slot and branches past it, which only the split
 * form can do (MACRO_ADDR there trips check_macro_slots).
 */
/* D_0015EFB0 is SDA elsewhere; pause.cpp reaches it through the
   assembler macro. */
extern int D_0015EFB0_m __asm__("D_0015EFB0") MACRO_ADDR;
extern int D_0013CBE4;

/* variation: separate "g" local per arm (shadowed), not shared across
   the whole function, to see if that lets the allocator pick the
   per-block register retail uses instead of one merged pseudo. */
extern char D_001D5F70[] NOT_SDA;

int func_00222B00(void) {
    if (D_0015EFB0_m != 0x10 && D_0015EFB0_m != 1) {
        char *g = D_001D5F70;
        int val = *(int *)(*(char **)(g + 4) + 0x38);
        *(int *)(g + 8) = val;
        return 0;
    }
    {
        int v = D_0013CBE4;
        if (v & 0x20) {
            char *g = D_001D5F70;
            *(int *)(g + 0xD4) = 0;
            *(int *)(g + 8) = *(int *)(*(char **)(g + 4) + 0x38);
            *(int *)(*(char **)(g + 4) + 0x84) = 1;
        } else if (v & 0x10) {
            char *g = D_001D5F70;
            *(int *)(g + 8) = *(int *)(*(char **)(g + 4) + 0x38);
            *(int *)(*(char **)(g + 4) + 0x84) = 0;
        }
    }
    return 0;
}

typedef struct {
    short s[12];
} TextBox;

extern char D_001603E8[];
extern int D_001D6044;
extern void func_00234C98_l(int, long) __asm__("func_00234C98");
extern void func_001F4630(int);
extern void func_001F4748(void);
extern void *func_001FE540_id(int) __asm__("func_001FE540");
extern void func_001F75D0(TextBox *, long, char *, int);

/* Draws the two-line prompt box (text 0x4FB3 for D_001D6044 in 0..2,
   0x4FB5 for 3, else D_001603E8) sized from arg0's +0x20/+0x24. The box
   is an aggregate initializer: this compiler clears it with a memset
   libcall, fills a temporary and copies that into the local with
   ldl/ldr/sdl/sdr pairs, exactly retail's sequence. */
int func_00222B98(char *arg0) {
    char *text;
    int v;

    func_00234C98_l(0x42, 0x44);
    func_00234C98_l(0x47, 0x2004B);
    SetupGifPaging(0);
    text = D_001603E8;
    v = D_001D6044;
    switch (v) {
    case 0:
    case 1:
    case 2:
        text = func_001FE540_id(0x4FB3);
        break;
    case 3:
        text = func_001FE540_id(0x4FB5);
        break;
    }
    {
        TextBox c = { { 1, *(int *)(arg0 + 0x24) + 1, 1, *(int *)(arg0 + 0x20) + 1,
                        *(int *)(arg0 + 0x20) >> 1, 5, 0, 0, 0x10, 5 } };

        func_001F75D0(&c, 0x80000000L, text, -1);
        c.s[5] = (*(int *)(arg0 + 0x24) - c.s[7]) >> 1;
        c.s[9] ^= 4;
        func_001F75D0(&c, 0x80000000L, text, -1);
        c.s[0]--;
        c.s[1]--;
        c.s[2]--;
        c.s[3]--;
        c.s[4]--;
        c.s[5]--;
        func_001F75D0(&c, 0x80FFA888L, text, -1);
    }
    DoGifPaging();
    return 2;
}

extern int D_001D48A8[];

/* `D_001D48A8[(unsigned)D_0015EE84 % 19]`; the older near-miss
   predated MACRO_ADDR. */
int func_00222D70(char *arg0) {
    *(int *)(arg0 + 0x34) = D_001D48A8[(unsigned int)D_0015EE84 % 19];
    return 0;
}

extern void func_0022ED80(int, int, int);

/* Pause sub-menu with a 30-entry wrapping cursor at arg0+0x40, in
   func_0021F7D0's shape. */
int func_00222DB0(char *arg0) {
    char *pad = D_0013CA40;
    int w, old;

    if (*(int *)(pad + 0x1C4) & 0xD00) {
        if (*(int *)(D_001D5F70 + 0x124) == 0) {
            return 1;
        }
    }
    {
        char *pad2 = D_0013CA40;
        if (*(int *)(pad2 + 0x1C4) & 0x10) {
            char *g = D_001D5F70;
            int t = *(int *)(*(char **)(g + 4) + 0x38);
            if (t != 0) {
                *(int *)(g + 8) = t;
                return 0;
            }
            if (*(int *)(g + 0x124) == 0) {
                return -1;
            }
        }
    }
    {
        char *pad3 = D_0013CA40;
        w = *(int *)(pad3 + 0x1A4);
    }
    old = *(int *)(arg0 + 0x40);
    if (w & 0x40) {
        *(int *)(arg0 + 0x40) = (old + 1) % 30;
    } else if (w & 0x20) {
        *(int *)(arg0 + 0x40) = (old + 29) % 30;
    }
    if (*(int *)(arg0 + 0x40) != old) {
        func_0022ED80(1, 0x11, *(int *)(arg0 + 0x14));
    }
    return 0;
}

extern void func_0022ED80(int, int, int);

/* func_00222DB0's twin with a 12-entry cursor at arg0+0x54. */
int func_00222E98(char *arg0) {
    char *pad = D_0013CA40;
    int w;

    if (*(int *)(pad + 0x1C4) & 0xD00) {
        if (*(int *)(D_001D5F70 + 0x124) == 0) {
            return 1;
        }
    }
    {
        char *pad2 = D_0013CA40;
        if (*(int *)(pad2 + 0x1C4) & 0x10) {
            char *g = D_001D5F70;
            int t = *(int *)(*(char **)(g + 4) + 0x38);
            if (t != 0) {
                *(int *)(g + 8) = t;
                return 0;
            }
            if (*(int *)(g + 0x124) == 0) {
                return -1;
            }
        }
    }
    {
        char *pad3 = D_0013CA40;
        w = *(int *)(pad3 + 0x1A4);
    }
    if (w & 0x2040) {
        *(int *)(arg0 + 0x54) = (*(int *)(arg0 + 0x54) + 1) % 12;
        func_0022ED80(1, 0x11, *(int *)(arg0 + 0x14));
    } else if (w & 0x8020) {
        *(int *)(arg0 + 0x54) = (*(int *)(arg0 + 0x54) + 11) % 12;
        func_0022ED80(1, 0x11, *(int *)(arg0 + 0x14));
    }
    return 0;
}

typedef struct { char c[2]; } Glyph2;
extern char D_001603F0[];
extern char D_001603F8[];
extern int func_00200248(int);
/* x, y first: arguments are evaluated in order, so the y conversion runs
   before the nested texture calls and is kept in $f20 across them, as in
   retail (and in func_00205E70, the other caller). */
extern void func_00200E38_f(float, float, int, int, int, float, float, float)
    __asm__("func_00200E38");

/* Draws the glyph "\x10" (arg0->0x38 set) or "\x11" and a gauge sprite
   below it, rotated by pi in the second case. The glyph string is a
   2-byte char struct copied onto the stack (retail's lb/lb/sb/sb). */
int func_00222FA8(char *arg0) {
    Glyph2 buf;

    SetupGifPaging(0);
    if (*(int *)(arg0 + 0x38) != 0) {
        buf = *(Glyph2 *)D_001603F0;
        func_001F68E8_c(4, *(int *)(arg0 + 0x24) / 2 - 8, 0x80FFA888L, &buf, -1);
        func_00200E38_f(640.0f, (float)(*(int *)(arg0 + 0x24) << 3), 0x20, 0x10,
                        GetFrameTex(GetIconFrame(0xE99E, 6)), 128.0f,
                        256.0f, 0.0f);
    } else {
        buf = *(Glyph2 *)D_001603F8;
        func_001F68E8_c(*(int *)(arg0 + 0x20) - 0x18,
                        *(int *)(arg0 + 0x24) / 2 - 8, 0x80FFA888L, &buf, -1);
        func_00200E38_f(192.0f, (float)(*(int *)(arg0 + 0x24) << 3), 0x20, 0x10,
                        GetFrameTex(GetIconFrame(0xE99E, 6)), 128.0f,
                        256.0f, 3.14159274f);
    }
    DoGifPaging();
    return 2;
}

/* D_001DE0C0 is declared above as Src0C (halfword view); these rows are
   read here as two word-sized icon ids. */
typedef struct { int a; int b; int c; } IconRow;
extern IconRow D_001DE0C0_i[] __asm__("D_001DE0C0");
extern void func_001F6EA8(int, int, long, void *, int);

/* Draws the current page's icons: the page record (stride 0xC) gives a
   row of D_001DE0C0, or -1 for the single icon 0x5019 centred in the
   box; otherwise the row's two icons at 1/3 and 2/3 of the height. The
   row is read before func_001F4630, and each call is written with its
   arguments inline: gcc evaluates them in order, so x and y are computed
   before the nested func_001FE540 call. */
int func_00223140(char *arg0) {
    char *p = *(char **)(D_001D5F74 + 0x40);
    int row = *(int *)(*(char **)(p + 0x34) + *(int *)(p + 0x40) * 12 + 4);

    SetupGifPaging(0);
    if (row == -1) {
        FontPrintCenter(*(int *)(arg0 + 0x20) / 2, *(int *)(arg0 + 0x24) / 2 - 8,
                      0x80FFA888L, func_001FE540_id(0x5019), -1);
    } else {
        FontPrintCenter(*(int *)(arg0 + 0x20) / 2, *(int *)(arg0 + 0x24) / 3 - 8,
                      0x80FFA888L, func_001FE540_id(D_001DE0C0_i[row].a), -1);
        FontPrintCenter(*(int *)(arg0 + 0x20) / 2, *(int *)(arg0 + 0x24) * 2 / 3 - 8,
                      0x80FFA888L, func_001FE540_id(D_001DE0C0_i[row].b), -1);
    }
    DoGifPaging();
    return 2;
}

extern int D_0015EFB4_m __asm__("D_0015EFB4") MACRO_ADDR;
extern int D_0015F6CC MACRO_ADDR;
extern char D_001D5240[];
extern void func_001FBC80(int, void *, int);
extern unsigned char D_0014C008[];
extern unsigned char D_0015EEB0[] MACRO_ADDR;
extern unsigned char D_0015EEC0[] MACRO_ADDR;
extern void func_001F9A00(void *, void *, int);
extern void func_00209CE8(int);
extern void func_0020BFC8(int, int);
extern int func_001F98C0(int);
extern unsigned char D_0013F450[];
extern void func_00228268(void);

/* Pad handler for the restart option (pad word D_0013CBE4): 0x20 opens
   the confirm popup; 0x40 restarts: the level reload (func_00209CE8)
   runs with the D_0013D510+0x1D byte, the 4 bytes at D_0014C008 and the
   D_0015EEB0/D_0015EEC0 tables saved and restored around it, then the
   pause state is reset. The flags are MACRO_ADDR (retail's one-register
   loads, $at stores and $gp-relative stores in delay slots); globals
   used as bases get a `char *` local per block, so retail keeps only the
   D_0013D510 %hi across the calls. */
int func_002232E0(void) {
    int v = D_0013CBE4;

    if (v & 0x20) {
        D_0015EFB4_m |= 2;
        func_001FBC80(3, D_001D5240, 0);
        return 0;
    }
    if (v & 0x40) {
        unsigned char save[4];
        char tbl0[0xC];
        char tbl1[0xC];
        unsigned char tag;
        int i;

        {
            unsigned char *m = gSkillPoints;
            tag = m[0x1D];
        }
        for (i = 0; i < 4; i++) {
            save[i] = D_0014C008[i];
        }
        func_001F9A00(tbl0, gCheats, 0xC);
        func_001F9A00(tbl1, D_0015EEC0, 0xC);
        {
            char *g = D_001D5F70;
            memcard_RestoreGame(*(int *)(g + 0xE0));
        }
        {
            unsigned char *m2 = gSkillPoints;
            m2[0x1D] = tag;
        }
        for (i = 0; i < 4; i++) {
            D_0014C008[i] = save[i];
        }
        func_001F9A00(gCheats, tbl0, 0xC);
        func_001F9A00(D_0015EEC0, tbl1, 0xC);
        D_0015EFA0 = 1;
        D_0015F6CC = 0;
        memcard_Save(0, -1);
        func_00228268();
        FadeToBlack(func_001F98C0(0x10));
        D_0013F450[0x20B1] = 1;
        return -1;
    }
    return 0;
}

int func_00223478(void *arg0) {
    *(int *)((char *)arg0 + 0x40) = 0;
    *(int *)((char *)arg0 + 0x50) = 0;
    *(int *)((char *)arg0 + 0x3C) = 0;
    return 0;
}

extern int D_001D2E74;
extern char D_001D5418[];
extern char D_001D54C8[];
extern char D_001D5588[];
extern char D_001D5618[];
extern char D_001D5630[];

/* A timed text sequence (credits-style): arg0+0x50 is the step, +0x40
   a countdown (func_001F98C0-scaled), +0x3C a scroll that advances 10 a
   frame (20 while all four shoulder buttons are held, which also halves
   the countdown), +0x34 the current text (a D_001D5xxx page or a text
   id) with D_001D2E74 its title id. Case bodies are in retail's layout
   order (4 before 3), and the shared "wait, then next step" tail is
   written out in each case: cross-jumping merges it, leaving each
   case's own argument load in its branch's delay slot. */
int func_00223490(char *arg0) {
    char *pad = D_0013CA40;
    char *g = D_001D5F70;
    int fast = (*(long *)(pad + 0x1A0) & 0xF) == 0xF;
    int x;

    if (*(int *)(g + 0xDC) == 0 || (fast && (*(int *)(pad + 0x1A0) & 0x10))) {
        if (*(int *)(pad + 0x1C4) & 0xD00) {
            if (*(int *)(g + 0x124) == 0) {
                return 1;
            }
        }
        {
            char *pad2 = D_0013CA40;
            if (*(int *)(pad2 + 0x1C4) & 0x10) {
                char *g2 = D_001D5F70;
                int t = *(int *)(*(char **)(g2 + 4) + 0x38);
                if (t != 0) {
                    *(int *)(g2 + 8) = t;
                    return 0;
                }
                if (*(int *)(g2 + 0x124) == 0) {
                    return -1;
                }
            }
        }
    }
    x = *(int *)(arg0 + 0x40);
    *(int *)(arg0 + 0x40) = x - 1;
    if (fast) {
        *(int *)(arg0 + 0x40) = x - 2;
    }
    if (*(int *)(arg0 + 0x40) < 0) {
        *(int *)(arg0 + 0x40) = 0;
    }
    switch (*(int *)(arg0 + 0x50)) {
    case 0:
        *(int *)(arg0 + 0x50) = *(int *)(arg0 + 0x50) + 1;
        *(int *)(arg0 + 0x40) = func_001F98C0(0xB4);
        *(int *)(arg0 + 0x3C) = 0;
        D_001D2E74 = 0x50A9;
        *(char **)(arg0 + 0x34) = D_001D5418;
        break;
    case 1:
    case 5:
    case 8:
    case 12:
    case 16:
        if (*(int *)(arg0 + 0x40) != 0) {
            return 0;
        }
        *(int *)(arg0 + 0x54) = 0;
        *(int *)(arg0 + 0x50) = *(int *)(arg0 + 0x50) + 1;
        break;
    case 2:
    case 6:
    case 9:
    case 13:
    case 17:
        x = *(int *)(arg0 + 0x3C);
        *(int *)(arg0 + 0x3C) = x + 10;
        if (fast) {
            *(int *)(arg0 + 0x3C) = x + 20;
        }
        if (*(int *)(arg0 + 0x54) == 0) {
            return 0;
        }
        *(int *)(arg0 + 0x40) = func_001F98C0(0xB4);
        *(int *)(arg0 + 0x50) = *(int *)(arg0 + 0x50) + 1;
        break;
    case 4:
        if (*(int *)(arg0 + 0x40) != 0) {
            return 0;
        }
        *(int *)(arg0 + 0x50) = 5;
        *(int *)(arg0 + 0x40) = func_001F98C0(0xB4);
        *(int *)(arg0 + 0x3C) = 0;
        D_001D2E74 = 0x50D6;
        *(char **)(arg0 + 0x34) = D_001D54C8;
        break;
    case 3:
        if (*(int *)(arg0 + 0x40) != 0) {
            return 0;
        }
        D_001D2E74 = 0x50D4;
        *(int *)(arg0 + 0x34) = 0x50D5;
        *(int *)(arg0 + 0x40) = func_001F98C0(0xF0);
        *(int *)(arg0 + 0x3C) = 0;
        *(int *)(arg0 + 0x50) = *(int *)(arg0 + 0x50) + 1;
        break;
    case 7:
        if (*(int *)(arg0 + 0x40) != 0) {
            return 0;
        }
        D_001D2E74 = 0x510B;
        *(char **)(arg0 + 0x34) = D_001D5588;
        *(int *)(arg0 + 0x40) = func_001F98C0(0xF0);
        *(int *)(arg0 + 0x3C) = 0;
        *(int *)(arg0 + 0x50) = *(int *)(arg0 + 0x50) + 1;
        break;
    case 10:
        if (*(int *)(arg0 + 0x40) != 0) {
            return 0;
        }
        D_001D2E74 = 0x513B;
        *(int *)(arg0 + 0x34) = 0x513C;
        *(int *)(arg0 + 0x40) = func_001F98C0(0xF0);
        *(int *)(arg0 + 0x3C) = 0;
        *(int *)(arg0 + 0x50) = *(int *)(arg0 + 0x50) + 1;
        break;
    case 11:
        if (*(int *)(arg0 + 0x40) != 0) {
            return 0;
        }
        *(int *)(arg0 + 0x40) = func_001F98C0(0xB4);
        *(int *)(arg0 + 0x3C) = 0;
        D_001D2E74 = 0x513D;
        *(char **)(arg0 + 0x34) = D_001D5618;
        *(int *)(arg0 + 0x50) = 12;
        break;
    case 14:
        if (*(int *)(arg0 + 0x40) != 0) {
            return 0;
        }
        D_001D2E74 = 0;
        *(int *)(arg0 + 0x34) = 0x5148;
        *(int *)(arg0 + 0x40) = func_001F98C0(0xF0);
        *(int *)(arg0 + 0x3C) = 0;
        *(int *)(arg0 + 0x50) = *(int *)(arg0 + 0x50) + 1;
        break;
    case 15:
        if (*(int *)(arg0 + 0x40) != 0) {
            return 0;
        }
        *(int *)(arg0 + 0x40) = func_001F98C0(0xB4);
        *(int *)(arg0 + 0x3C) = 0;
        D_001D2E74 = 0x5149;
        *(char **)(arg0 + 0x34) = D_001D5630;
        *(int *)(arg0 + 0x50) = 16;
        break;
    case 18:
        if (*(int *)(arg0 + 0x40) != 0) {
            return 0;
        }
        D_001D2E74 = 0;
        *(int *)(arg0 + 0x34) = 0x517A;
        *(int *)(arg0 + 0x50) = *(int *)(arg0 + 0x50) + 1;
        *(int *)(arg0 + 0x40) = func_001F98C0(300);
        break;
    case 19:
        if (*(int *)(arg0 + 0x40) != 0) {
            return 0;
        }
        {
            char *g3 = D_001D5F70;
            if (*(int *)(g3 + 0xDC) != 0) {
                return 1;
            }
        }
        break;
    }
    return 0;
}

/* A text box on the menu's own geometry (func_00227A30's box, then
   arg0+0x18..0x24: top y + 4, bottom y + h - 4, x, x + w, centre).
   Per state (arg0+0x50): the list states draw each text id of the -1
   terminated list at arg0+0x34 down from a start that the line count
   (arg0+0x3C >> 4) centres, taking each height from box[7] as the text
   call leaves it, and flag arg0+0x54 when the list ends 0x18 above the
   bottom; states 4, 11, 15 and 19 draw the single text id at +0x34. The
   line count's low nibble goes through `& 0xF` (retail's lbu + dsrl) and
   the start subtracts its own variable, which keeps retail's order. */
int func_00223810(char *arg0) {
    short box[12];
    int ypos;
    int n;
    int i;

    func_00234C98_l(0x47, 0x30000);
    func_00234C98_l(0x42, 0x8000000044L);
    SetupGifPaging(0);
    func_00227A30(box, arg0);
    box[9] = 9;
    box[3] = *(int *)(arg0 + 0x18) + *(int *)(arg0 + 0x20);
    box[0] = *(int *)(arg0 + 0x1C) + 4;
    box[1] = *(int *)(arg0 + 0x1C) + *(int *)(arg0 + 0x24) - 4;
    box[2] = *(int *)(arg0 + 0x18);
    box[4] = *(int *)(arg0 + 0x18) + (*(int *)(arg0 + 0x20) >> 1);
    box[10] = 0;
    box[11] = 0;
    switch (*(int *)(arg0 + 0x50)) {
    case 1:
    case 2:
    case 3:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 12:
    case 13:
    case 14:
    case 16:
    case 17:
    case 18:
        n = (*(int *)(arg0 + 0x3C) >> 4) - 4;
        ypos = *(int *)(arg0 + 0x1C) - n;
        for (i = 0; (*(int **)(arg0 + 0x34))[i] != -1; i++) {
            box[5] = ypos;
            box[11] = (*(int *)(arg0 + 0x3C) >> 4) & 0xF;
            func_001F7560(box, 0x80FFA888L,
                          func_001FE540_id((*(int **)(arg0 + 0x34))[i]), -1);
            ypos += box[7];
            ypos += 10;
        }
        if (ypos + 0x18 < *(int *)(arg0 + 0x1C) + *(int *)(arg0 + 0x24)) {
            *(int *)(arg0 + 0x54) = 1;
        }
        break;
    case 4:
    case 11:
    case 15:
    case 19:
        box[5] = *(int *)(arg0 + 0x1C) + 0x20;
        func_001F7560(box, 0x80FFA888L, func_001FE540_id(*(int *)(arg0 + 0x34)), -1);
        break;
    case 0:
        break;
    }
    DoGifPaging();
    return 2;
}

extern void func_001153FC(void *, int, int); /* memset */
typedef struct {
    short f0, f2, f4, f6;
    short x, y;
    short fC;
    unsigned short fE;
    short f10, f12, f14, f16;
} Box18;

/* ObtainAllGoldWeaponsMenu: two icon + text rows (texts 0x5187, 0x5188)
   in a box sized from arg0+0x20/0x24. The box is built zeroed in a
   temporary and copied (a struct assignment: retail's ldl/ldr copy). y is
   one variable, 4 and then the bottom of the first text (buf.fE, written
   by func_001F7560) plus 0x10, with each icon at y + 8; it and x stay in
   callee-saved registers across the calls, as in retail. */
int func_002239F0(char *arg0) {
    Box18 buf;
    Box18 tmp;
    int x, y;

    func_001153FC(&tmp, 0, 0x18);
    tmp.f2 = *(unsigned short *)(arg0 + 0x24);
    tmp.f6 = *(unsigned short *)(arg0 + 0x20);
    tmp.f10 = 0x10;
    buf = tmp;
    x = 0x18;
    y = 4;
    SetupGifPaging(0);
    HudSprite(GetIconFrame(0xE99A, 6), 4, y + 8, 0x10, 0x10, 0x80);
    buf.x = x;
    buf.y = y;
    func_001F7560(&buf, 0x80FFA888L, func_001FE540_id(0x5187), -1);
    y = (short)buf.fE + 0x10;
    buf.y = y;
    HudSprite(GetIconFrame(0xE99A, 6), 4, y + 8, 0x10, 0x10, 0x80);
    func_001F7560(&buf, 0x80FFA888L, func_001FE540_id(0x5188), -1);
    DoGifPaging();
    return 2;
}

struct ThreeOptionMenu_23B40 {
    u8 pad0[0x20];
    s32 menu_width;
    s32 height;
    s32 spacing_divisor;
};

extern u8 D_001DF3D0_23B40[] __asm__("D_001DF3D0");
extern u8 D_001DF770_23B40[] __asm__("D_001DF770");
extern void func_001F4630_23B40(s32) __asm__("func_001F4630");
extern void func_001F4748_23B40(void) __asm__("func_001F4748");
extern char *func_001FE540_23B40(s32) __asm__("func_001FE540");
extern s32 func_002160E0_23B40(void) __asm__("func_002160E0");
extern s32 func_00216150_23B40(void) __asm__("func_00216150");
extern s32 func_00216198_23B40(void) __asm__("func_00216198");
extern s32 func_00116248_23B40(char *, const char *, ...) __asm__("func_00116248");
extern s32 func_001F6600_23B40(char *, s32) __asm__("func_001F6600");
extern void func_001F65A8_23B40(void) __asm__("func_001F65A8");
extern void func_001F6598_23B40(void) __asm__("func_001F6598");
extern void func_001F6EA8_23B40(s32, s32, u64, char *, s32) __asm__("func_001F6EA8");
extern void func_002208F8_23B40(s32, s32, s32) __asm__("func_002208F8");
extern u64 func_001F4868_23B40(s32) __asm__("func_001F4868");
extern void func_001F6668_23B40(s32, s32, u64, char *, s32, s64, u8 *) __asm__("func_001F6668");

/* Draws the three completion counters page (n of 40, n of 10, n of 30): measures the three
   lines to pick the regular or the narrow font, then a centred title and each line with its
   tick mark when complete.
   Adapted from Lombyte (MIT) for PAL: src/textbin/ui/menus/draw_localized_three_option_menu.c, draw_localized_three_option_menu. */
s32 func_00223B40(struct ThreeOptionMenu_23B40 *menu) {
    char text_buffer[0x50];
    s32 font_texture_index;
    u8 *glyphs;
    s32 maximum_text_width;
    s32 measured_width;
    s32 line_spacing;
    s32 draw_y;

    font_texture_index = 1;
    func_001F4630_23B40(0);
    line_spacing = menu->height / 5;
    draw_y = menu->height / 5 - 8;
    maximum_text_width = 0;
    func_00116248_23B40(text_buffer, func_001FE540_23B40(0x5234), func_002160E0_23B40(), 0x28);
    measured_width = func_001F6600_23B40(text_buffer, -1);
    if (maximum_text_width < measured_width) {
        maximum_text_width = measured_width;
    }
    func_00116248_23B40(text_buffer, func_001FE540_23B40(0x5235), func_00216150_23B40(), 10);
    measured_width = func_001F6600_23B40(text_buffer, -1);
    if (maximum_text_width < measured_width) {
        maximum_text_width = measured_width;
    }
    func_00116248_23B40(text_buffer, func_001FE540_23B40(0x5236), func_00216198_23B40(), 0x1E);
    measured_width = func_001F6600_23B40(text_buffer, -1);
    if (maximum_text_width < measured_width) {
        maximum_text_width = measured_width;
    }
    glyphs = D_001DF3D0_23B40;
    if (menu->menu_width < maximum_text_width + 0x18) {
        font_texture_index = 2;
        glyphs = D_001DF770_23B40;
    }
    func_001F65A8_23B40();
    func_001F6EA8_23B40(menu->menu_width >> 1, draw_y, 0x80FFA888, func_001FE540_23B40(0x5233), -1);
    draw_y += line_spacing;
    func_002208F8_23B40(0xB, draw_y + 9, func_002160E0_23B40() == 0x28);
    func_00116248_23B40(text_buffer, func_001FE540_23B40(0x5234), func_002160E0_23B40(), 0x28);
    func_001F6668_23B40(0x14, draw_y, 0x80FFA888, text_buffer, -1, func_001F4868_23B40(font_texture_index),
                        glyphs);
    draw_y += line_spacing;
    func_002208F8_23B40(0xB, draw_y + 9, func_00216150_23B40() == 10);
    func_00116248_23B40(text_buffer, func_001FE540_23B40(0x5235), func_00216150_23B40(), 10);
    func_001F6668_23B40(0x14, draw_y, 0x80FFA888, text_buffer, -1, func_001F4868_23B40(font_texture_index),
                        glyphs);
    draw_y += line_spacing;
    func_002208F8_23B40(0xB, draw_y + 9, func_00216198_23B40() == 0x1E);
    func_00116248_23B40(text_buffer, func_001FE540_23B40(0x5236), func_00216198_23B40(), 0x1E);
    func_001F6668_23B40(0x14, draw_y, 0x80FFA888, text_buffer, -1, func_001F4868_23B40(font_texture_index),
                        glyphs);
    func_001F6598_23B40();
    func_001F4748_23B40();
    return 2;
}

/* Page records in D_0013D390: stride 0x1C from +0x20. */
typedef struct { int row; char pad[0x18]; } PageRec;
typedef struct { char pad[0x20]; PageRec rec[1]; } PageTbl;

/* func_00223140's sibling for the D_0013D390 pages: if the page passes
   four gates, draws icon 0x521C centred (row -1) or the row's two
   D_001DE0C0 icons at y 4 and 0x14. The row is read before
   func_001F4630; the record is reached as a struct member off the page
   base (retail's base-first add). */
int func_00223E40(char *arg0) {
    char *g = D_001D5F70;
    char *b = D_0013D390;
    int row = ((PageTbl *)b)->rec[*(int *)(*(char **)(*(char **)(g + 4) + 0x40) + 0x40)].row;

    SetupGifPaging(0);
    if (*(int *)(b + 0xDC) < 3 && *(int *)(b + 0xE4) < 0
        && *(int *)(g + 0x154) >= 0xB && *(int *)(b + 8) == 2) {
        if (row == -1) {
            FontPrintCenter(*(int *)(arg0 + 0x20) / 2, *(int *)(arg0 + 0x24) / 2 - 8,
                          0x80FFA888L, func_001FE540_id(0x521C), -1);
        } else {
            FontPrintCenter(*(int *)(arg0 + 0x20) / 2, 4, 0x80FFA888L,
                          func_001FE540_id(D_001DE0C0_i[row].a), -1);
            FontPrintCenter(*(int *)(arg0 + 0x20) / 2, 0x14, 0x80FFA888L,
                          func_001FE540_id(D_001DE0C0_i[row].b), -1);
        }
    }
    DoGifPaging();
    return 2;
}


int func_00223FD0(void *arg0) {
    func_00226D50(1);
    *(int *)((char *)arg0 + 0x48) = func_00226EA8(0);
    *(int *)((char *)arg0 + 0x4C) = 0;
    return 0;
}

int func_00224010(void *arg0) {
    *(int *)((char *)arg0 + 0x48) = func_00226F68(*(int *)((char *)arg0 + 0x48));
    return 0;
}

extern int D_0015EF34 MACRO_ADDR;
extern int D_0015FF4C MACRO_ADDR;
extern int D_0015EE98 MACRO_ADDR;
extern int D_0015EF20 MACRO_ADDR;
extern int D_0015EF24 MACRO_ADDR;
extern int D_0015EE84_mm __asm__("D_0015EE84") MACRO_ADDR;
/* The 8-byte D_0015EF98 pair saved into a slot. As a char block in small
   data, its copy is la + ldl/ldr, as in retail. */
typedef struct {
    char b[8];
} SaveWord2;
extern SaveWord2 D_0015EF98_s __asm__("D_0015EF98") MACRO_ADDR;
extern char D_001D28F8[];
extern int D_001D29C0;
extern void func_00227C78(int, int);

/* Save-slot menu handler (func_002243E8's sibling): on entry it may arm
   a save (0x4FB5 prompt), then while the prompt is up it either aborts
   (flag 0x80 in D_0015EFB4) or writes the current game into slot
   d+0x14 of D_0013D390's 0x1C-byte records; otherwise the usual pad
   handling, with slot selection on 0x1000/0x4000 and 0x40 either
   loading the slot (via D_001D28F8) or re-arming. Each branch reaches
   the globals through its own block-scoped pointers. */
int func_00224040(char *arg0) {
    char *g = D_001D5F70;
    int s;
    int old;

    if (*(char **)(*(char **)(g + 4) + 0x40) != arg0) {
        return 0;
    }
    s = *(int *)(arg0 + 0x4C);
    if (s == 0) {
        char *m = *(char **)(g + 0xD0);
        if (m == D_001D28F8) {
            if (*(int *)(m + 0x84) != 0) {
                *(int *)(arg0 + 0x4C) = 1;
            }
        }
        s = *(int *)(arg0 + 0x4C);
    }
    if (s == 1) {
        char *g2;

        func_00227C78(*(int *)(arg0 + 0x48), *(int *)(arg0 + 0x40));
        g2 = D_001D5F70;
        *(int *)(g2 + 0x12C) = 0x4FB5;
        *(int *)(g2 + 0x128) = 1;
        D_0015FF4C = 0;
    }
    *(int *)(arg0 + 0x4C) = 2;
    old = *(int *)(arg0 + 0x40);
    {
        char *g3 = D_001D5F70;

        if (*(int *)(g3 + 0x128) != 0) {
            char *d = D_0013D390;
            char *e;

            if (*(int *)(d + 0xDC) >= 3) {
                return 0;
            }
            if (*(int *)(d + 0xE4) >= 0) {
                return 0;
            }
            if (*(int *)(g3 + 0x154) < 11) {
                return 0;
            }
            *(int *)(g3 + 0x128) = 0;
            if (*(int *)(d + 0xEC) != 0 || *(int *)(d + 0x1C) != 0 || D_0015FF4C != 0) {
                char *d2 = D_0013D390;
                char *g4 = D_001D5F70;

                D_0015EFB4_m |= 0x80;
                *(int *)(d2 + 0xFC) = 0;
                func_001FBC80(3, *(void **)(g4 + 4), 0);
                return 0;
            }
            e = d + 0x30;
            *(int *)(d + 0xFC) = 1;
            *(int *)(d + 0x24 + *(int *)(d + 0x14) * 0x1C) = gBolts;
            *(int *)(d + 0x20 + *(int *)(d + 0x14) * 0x1C) = D_0015EE84_mm;
            *(int *)(d + 0x2C + *(int *)(d + 0x14) * 0x1C) = D_0015EF24;
            *(SaveWord2 *)(e + *(int *)(d + 0x14) * 0x1C) = D_0015EF98_s;
            *(int *)(d + 0x28 + *(int *)(d + 0x14) * 0x1C) = D_0015EF20;
        }
    }
    {
        char *pad = D_0013CA40;
        if (*(int *)(pad + 0x1C4) & 0xD00) {
            char *g2 = D_001D5F70;
            if (*(int *)(g2 + 0x124) == 0) {
                return 1;
            }
        }
    }
    {
        char *pad = D_0013CA40;
        if (*(int *)(pad + 0x1C4) & 0x10) {
            char *g2 = D_001D5F70;
            int t = *(int *)(*(char **)(g2 + 4) + 0x38);
            if (t != 0) {
                *(int *)(g2 + 8) = t;
                return 0;
            }
            if (*(int *)(g2 + 0x124) == 0) {
                return -1;
            }
        }
    }
    if (D_0015EFB0_m != 0x10 && D_0015EFB0_m != 1) {
        char *g2 = D_001D5F70;
        *(int *)(g2 + 8) = *(int *)(*(char **)(g2 + 4) + 0x38);
        return 0;
    }
    {
        char *d = D_0013D390;
        int p;
        int x;

        if (*(int *)(d + 0xDC) >= 3) {
            return 0;
        }
        if (*(int *)(d + 0xE4) >= 0) {
            return 0;
        }
        {
            char *g2 = D_001D5F70;
            if (*(int *)(g2 + 0x154) < 11) {
                return 0;
            }
        }
        if (*(int *)(d + 8) != 2) {
            return 0;
        }
        if (*(int *)(arg0 + 0x30) & 1) {
            char *pad = D_0013CA40;
            p = *(int *)(pad + 0x1B4);
        } else {
            char *pad = D_0013CA40;
            p = *(int *)(pad + 0x1A4);
        }
        x = D_0015EF34;
        *(int *)(arg0 + 0x40) = x;
        if ((p & 0x1000) && x != 0) {
            *(int *)(arg0 + 0x40) = x - 1;
        }
        if ((p & 0x4000) && *(int *)(arg0 + 0x40) < 4) {
            *(int *)(arg0 + 0x40) = *(int *)(arg0 + 0x40) + 1;
        }
        D_0015EF34 = *(int *)(arg0 + 0x40);
        if (p & 0x40) {
            char *d2 = D_0013D390;

            if (*(int *)(d2 + 8) == 2) {
                if (*(int *)(d2 + 0x20 + D_0015EF34 * 0x1C) != -1) {
                    char *g2 = D_001D5F70;

                    *(int *)(g2 + 0xD4) = 0;
                    *(char **)(g2 + 8) = D_001D28F8;
                    D_001D29C0 = *(int *)(arg0 + 0x40);
                } else {
                    *(int *)(arg0 + 0x4C) = 1;
                }
            }
        }
    }
    if (*(int *)(arg0 + 0x40) != old) {
        func_0022ED80(1, 0x11, *(int *)(arg0 + 0x14));
    }
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/text", func_002243E8);

INCLUDE_ASM("asm/nonmatchings/text", func_00224728);

typedef struct {
    u8 pad_0[0x20];
    s32 width;
    u8 pad_24[0x1C];
    s32 selected_slot;
} SaveMenu_24C30;

typedef struct {
    s32 id;
    s32 bolts;
    s32 count;
    s32 time;
    u8 pad_10[5];
    u8 b15;
    u8 b16;
    u8 b17;
    u8 pad_18[4];
} SaveSlot_24C30;

typedef struct {
    u8 pad_0[8];
    s32 state;
    u8 pad_C[0x14];
    SaveSlot_24C30 slots[5];
    u8 pad_AC[0x30];
    s32 xDC;
    s32 pad_E0;
    s32 xE4;
} SaveInfo_24C30;

/* Menu system state at D_001D5F70, as far as this function reads it. */
struct MenuSystem_24C30 {
    u8 pad_0[0x154];
    s32 unk154;               /* 0x154: PAL only, updates since the memory card went idle */
};
extern struct MenuSystem_24C30 D_001D5F70_24C30 __asm__("D_001D5F70");

extern SaveInfo_24C30 D_0013D390_24C30 __asm__("D_0013D390");
extern s32 D_0015EE80_24C30 __asm__("D_0015EE80") MACRO_ADDR;
extern s32 D_001602B0_24C30 __asm__("D_001602B0") MACRO_ADDR;
extern char D_001603A0_24C30[] __asm__("D_001603A0");
extern char D_001603D0_24C30[] __asm__("D_001603D0");
extern char D_00160400_24C30[] __asm__("D_00160400");
extern char D_00160410_24C30[] __asm__("D_00160410");
extern char D_00160420_24C30[] __asm__("D_00160420");

extern void func_001F4630_24C30(s32) __asm__("func_001F4630");
extern void func_001F4748_24C30(void) __asm__("func_001F4748");
extern void func_001F6968_24C30(s32, s32, u64, char *, s32) __asm__("func_001F6968");
extern void func_001F6F40_24C30(s32, s32, u64, char *, s32) __asm__("func_001F6F40");
extern char *func_001FE540_24C30(s32) __asm__("func_001FE540");
extern s32 func_00200198_24C30(s32, s32) __asm__("func_00200198");
extern void func_00200468_24C30(s32, s32, s32, s32, s32, s32) __asm__("func_00200468");
extern void func_00201640_24C30(s32, s32, s32, s32, u64, s32) __asm__("func_00201640");
extern s32 func_00116248_24C30(char *, const char *, ...) __asm__("func_00116248");

/* Draws the five save slots of the memory card page: the cursor frame around the selected
   one once the card is idle, then per slot "empty" or play time, gold bolts, bolts and date.
   Adapted from Lombyte (MIT) for PAL: src/ui/menus/draw_save_slot_list.c, draw_save_slot_list. */
s32 func_00224C30(SaveMenu_24C30 *menu) {
    char text[0x50];
    SaveSlot_24C30 *slot;
    s32 color;
    s32 width;
    s32 inner_right;
    s32 highlight_top;
    s32 highlight_bottom;
    s32 inner_top;
    s32 inner_bottom;
    s32 i;
    s32 y;
    s32 play_ticks;
    s32 hours;
    s32 bolts;
    s32 minutes;

    y = 4;
    func_001F4630_24C30(0);
    for (i = 0; i < 5; i++) {
        width = menu->width;
        highlight_top = y - 4;
        highlight_bottom = y + 0x34;
        inner_right = width - 3;
        inner_top = y - 1;
        inner_bottom = y + 0x31;
        if (D_0013D390_24C30.xDC < 3 && D_0013D390_24C30.xE4 < 0 && D_001D5F70_24C30.unk154 >= 0xB &&
            menu->selected_slot == i) {
            func_00201640_24C30(0, highlight_top, width, highlight_bottom, 0x8020FFFF, 0);
            func_00201640_24C30(3, inner_top, inner_right, inner_bottom, D_001602B0_24C30, 0);
        }
        func_00201640_24C30(3, inner_top, inner_right, inner_bottom, 0x80303030, 0);
        slot = &D_0013D390_24C30.slots[i];
        if (D_0013D390_24C30.xDC >= 3 || D_0013D390_24C30.xE4 >= 0 || D_001D5F70_24C30.unk154 < 0xB ||
            D_0013D390_24C30.state != 2) {
            y += 0x30;
        } else {
            if (slot->id == -1) {
                y += 0x10;
                func_001F6F40_24C30(menu->width / 2, y, 0x80FFA888,
                                        func_001FE540_24C30(0x521C), -1);
                y += 0x20;
            } else {
                play_ticks = slot->time;
                hours = play_ticks / 216000;
                minutes = play_ticks / 3600 - hours * 60;
                color = menu->selected_slot == i ? (s32)0x8020FFFF : (s32)0x80FFA888;
                if (hours > 99) {
                    hours = 99;
                    minutes = 59;
                }
                func_00116248_24C30(text, D_00160400_24C30, hours, minutes);
                func_00200468_24C30(func_00200198_24C30(0xE99E, 3), 4, y, 0x10, 0x10,
                                0x80);
                func_001F6968_24C30(0x16, y, color, text, -1);
                if (slot->count != 0) {
                    func_00200468_24C30(func_00200198_24C30(0xE99E, 4), 0x4E, y, 0x10,
                                    0x10, 0x80);
                    func_00116248_24C30(text, D_001603A0_24C30, slot->count < 100 ? slot->count : 99);
                    func_001F6968_24C30(0x60, y, color, text, -1);
                }
                bolts = slot->bolts;
                if (bolts > 9999999) {
                    bolts = 9999999;
                }
                y += 0x10;
                if (bolts < 1000) {
                    func_00116248_24C30(text, D_001603A0_24C30, bolts);
                } else if (bolts < 1000000) {
                    func_00116248_24C30(text, D_001603D0_24C30, bolts / 1000, bolts % 1000);
                } else {
                    func_00116248_24C30(text, D_00160410_24C30, bolts / 1000000, bolts % 1000000 / 1000,
                            bolts % 1000);
                }
                func_00200468_24C30(func_00200198_24C30(0x754F, 0xF), 4, y, 0x10, 0x10,
                                0x80);
                func_001F6968_24C30(0x16, y, color, text, -1);
                y += 0x10;
                func_00116248_24C30(text, D_00160420_24C30, slot->b16, slot->b15, slot->b17);
                func_00200468_24C30(func_00200198_24C30(0xE99E, 2), 4, y, 0x10, 0x10,
                                0x80);
                func_001F6968_24C30(0x16, y, color, text, -1);
                y += 0x10;
            }
        }
        if (D_0015EE80_24C30 != 0) {
            y += 0x24;
        } else {
            y += 0x1B;
        }
    }
    func_001F4748_24C30();
    return 2;
}

struct MenuFlashingPanel {
    u8 pad0[0x48];
    s32 active;
    s32 time;
    s32 x;
    s32 y;
    s32 width;
    s32 height;
};
struct MenuPanelOwner {
    u8 pad0[0x78];
    struct MenuFlashingPanel *flash;
};
extern short D_0015EE80_250B8 __asm__("D_0015EE80");
extern void func_00234C98_250B8(s32, u64) __asm__("func_00234C98");
extern s32 func_002140B0(s32);
extern s64 func_001F4868_250B8(s32) __asm__("func_001F4868");
extern void func_001F5800_250B8(s32, s32, s32, s32, s32, s32, s32, s32, s64, s64) __asm__("func_001F5800");
void func_002250B8(struct MenuPanelOwner *owner);

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/ui/menus/fun_00223e28.c, draw_menu_flashing_panel. */
void func_002250B8(struct MenuPanelOwner *owner) {
    struct MenuFlashingPanel *panel;
    s32 x;
    s32 y;
    s32 width;
    s32 height;
    s32 u;
    s32 v;
    s32 alpha;
    s32 width_adjustment;
    s32 height_adjustment;

    if (owner == 0) {
        return;
    }
    panel = owner->flash;
    if (panel == 0) {
        return;
    }
    x = panel->x;
    y = panel->y;
    width = panel->width;
    height = panel->height;
    if (x >= 0x200 || x + width < 0) {
        return;
    }
    if (!((*(s32 *)&D_0015EE80_250B8) != 0 ? y < 0x1C1 : y < 0x1A1) || y + height < 0) {
        return;
    }
    func_00234C98_250B8(8, 0);
    func_00234C98_250B8(0x42, 0x8000000044ULL);
    func_002008B8(GetIconFrame(0xE99E, 7), x << 4, y << 4, width << 4, height << 4, 0x80);
    if (panel->active) {
        panel->time += 2;
        u = random_integer_below(200);
        v = random_integer_below(200);
        alpha = 0x80 - func_001F9B70(panel->time - 0x80);
        func_00234C98_250B8(8, 0);
        alpha = alpha * 2;
        func_00234C98_250B8(0x42, ((u64)(alpha > 0x80 ? 0x80 : alpha) << 32) | 0x68);
        func_001F5800_250B8(x, y, width, height, u, v, width, height, 0x808080, func_001F4868_250B8(0x1A));
        if (panel->time >= 0x100) {
            panel->active = 0;
        }
    } else if (random_integer_below(2000) == 0) {
        panel->time = 0;
        panel->active = 1;
    }
    func_00234C98_250B8(8, 0);
    func_00234C98_250B8(0x42, 0x8000000044ULL);
    width_adjustment = -2;
    height_adjustment = -2;
    func_001F5800_250B8(x, y, width, height, 0, 0, width, (height * 3) >> 1, 0x50606060, func_001F4868_250B8(0x1C));
    if (width >= 0x4C) {
        width_adjustment = -1;
    }
    if (height >= 0x4C) {
        height_adjustment = -1;
    }
    if (width >= 0x97) {
        width_adjustment = 0;
    }
    if (height >= 0x97) {
        height_adjustment = 0;
    }
    func_001F5800_250B8(x + 1, y + 1, width + width_adjustment, height + height_adjustment, 1, 1, 0x3E, 0x3E, 0x80808080, func_001F4868_250B8(0x19));
}

extern int func_002279D0(void);
extern int D_0016004C MACRO_ADDR;
extern char D_001D6160[];
extern char D_001D61A0[];
extern char D_001D61E0[];
extern char D_00186410[];
extern void func_001E9790(void *);
extern void func_00225DF0(void);
extern void func_00226250(void *);

/* Menu setup: resets the pad bindings, marks D_001D5F70's selections
   unset, fetches three entries with func_00226EA8, clears the 24 flag
   bytes at arg0+0xA4, then spawns moby 0 in front of the camera (update
   func_00225DF0, owner arg0) and moby 0x259 (update func_00226250). One
   `g` local per block reading D_001D5F70 gives retail's %hi kept in a
   saved register with %lo rebuilt; the -1 stores are written in the
   order that gives retail's schedule. */
int func_00225358(char *arg0) {
    char *m;
    int *p;
    int *v;
    char *q;
    int i, j;

    func_00226D50(1);
    func_002279D0();
    {
        unsigned char *g = D_001D5F70;

        D_0016004C = -1;
        *(int *)(g + 0x11C) = -1;
        *(int *)(g + 0x120) = -1;
        *(int *)(g + 0xA0) = func_00226EA8(1);
        *(int *)(g + 0xA4) = func_00226EA8(1);
        g[0xC8] = 0xFF;
        g[0xC9] = 0xFF;
        g[0xCA] = 0;
        p = (int *)(g + 0xB0);
        for (i = 2; i >= 0; i--) {
            *p = func_00226EA8(0);
            p++;
        }
    }
    {
        char *g = D_001D5F70;

        D_001D6160[1] = 0;
        *(int *)(g + 0x1C) = -1;
        D_001D61A0[1] = 0;
        D_001D61E0[1] = 0;
    }
    m = func_00226720_a(0);
    q = arg0 + 0xBB;
    for (j = 23; j >= 0; j--) {
        *q = 0;
        q--;
    }
    if (m != 0) {
        char *cam = D_00187040;
        char *g;

        *(char **)(arg0 + 0x44) = m;
        *(short *)(m + 0x34) = 0;
        *(float *)(m + 0x10) = *(float *)(cam + 0x140) + 4.0f;
        *(float *)(m + 0x14) = *(float *)(cam + 0x144);
        *(float *)(m + 0x18) = *(float *)(cam + 0x148) - 0.6f;
        *(float *)(m + 0x48) = 3.1415927f;
        *(void **)(m + 0x74) = func_00225DF0;
        v = *(int **)(m + 0x78);
        v[0] = (int)arg0;
        v[1] = 0;
        v[2] = 0;
        g = D_001D5F70;
        *(int *)(g + 0xC0) = -1;
        FastMemSet(D_00186410, 0, 0x40);
        func_001E9790(m);
    }
    m = func_00226720_a(0x259);
    if (m != 0) {
        **(int **)(m + 0x78) = (int)arg0;
        *(void **)(m + 0x74) = func_00226250;
        *(short *)(m + 0x34) = 4;
    }
    *(char **)(arg0 + 0x48) = m;
    return 0;
}
__asm__(".section .text\n\tnop\n");

/*
 * Exact once the short-loop padding retail's assembler adds is reproduced
 * (tools/fix_short_loops.py): both loops are five instructions and get one
 * nop each. g is unsigned char so 0xFF is built as 0xFF, not -1. Two
 * separate loop counters are load-bearing: with one variable reused, both
 * loops take $s2 and q takes $s1.
 */
extern int func_002267C0(int);
extern int func_00226F68(int);
extern void func_00227A70(void);

int func_00225548(void *arg0) {
    unsigned char *g; int *p; int *q; int i; int j;

    p = (int *)((char *)arg0 + 0x44);
    for (i = 23; i >= 0; i--) { *p = func_002267C0(*p); p++; }
    g = D_001D5F70;
    *(int *)(g + 0xA0) = func_00226F68(*(int *)(g + 0xA0));
    *(int *)(g + 0xA4) = func_00226F68(*(int *)(g + 0xA4));
    g[0xC8] = 0xFF;      // emitted 0xC9 then 0xC8: two stores of the
    g[0xC9] = 0xFF;      // same value come out in the opposite order
    g[0xCA] = 0;
    func_00227A70();
    q = (int *)(g + 0xB0);
    for (j = 2; j >= 0; j--) { *q = func_00226F68(*q); q++; }
    return 0;
}

typedef struct HandGadgetState {
    u8 pad00[0x44];
    s32 source_moby_address;
    u8 pad48[4];
    Moby *pose_moby;
    Moby *animation_moby;
    Moby *class_pose_moby;
    Moby *first_attachment_moby;
    Moby *second_attachment_moby;
    s32 x60;
    s32 x64;
    s32 x68;
    Moby *class_0197_moby;
    Moby *class_0266_moby;
    Moby *class_026a_moby;
    Moby *ammo_mobys[8];
    u8 pad98[0xC];
    u8 timers[0x18];
} HandGadgetState;
typedef struct HandGadgetDefinition {
    u8 pad00[0x10];
    s32 oclass;
    u8 pad14[0x38];
} HandGadgetDefinition;
typedef struct HandGadgetSelection {
    u8 pad00[0x1C];
    s32 current_gadget;
    u8 pad20[0x10];
    s32 selected_gadget;
    s32 attachment_gadget;
    s32 animation_gadget;
    s32 pose_gadget;
    u8 pad40[0x8C];
    s32 animation_base;
    u8 padD0[0x48];
    s32 D_00160050;
    s32 active_resource_class;
    s32 requested_resource_class;
    u8 pad124[0x1C];
    s32 last_requested_resource_class;
    s32 last_resource_request_state;
} HandGadgetSelection;
typedef struct HandGadgetManipulator {
    u8 pad0;
    u8 active;
    u8 pad2[0x1E];
    float rotation_x;
    float rotation_y;
    float rotation_z;
} HandGadgetManipulator;
typedef struct HandGadgetAnimation {
    s32 resource_first;
    s32 resource_count;
    s32 primary_animation;
    s32 delay_frames;
    s32 item_animation;
    s32 secondary_animation;
    s32 attachment0_class;
    s32 attachment0_animation;
    s32 attachment1_class;
    s32 attachment1_animation;
    s32 attachment2_class;
    s32 attachment2_animation;
} HandGadgetAnimation;
typedef struct HandGadgetPlayerState {
    u8 pad0[0x10B8];
    s32 equipped_gadget;
    u8 pad10BC[0xF3A];
    u8 ammo_used;
    u8 ammo_capacity;
} HandGadgetPlayerState;
extern HandGadgetPlayerState D_0013F450_255F8 __asm__("D_0013F450");
extern HandGadgetDefinition D_001864D0_255F8[] __asm__("D_001864D0");
extern u8 * D_001B3580_255F8[] __asm__("D_001B3580");
extern u8 D_001B3E40[];
extern HandGadgetAnimation D_001D5668[];
extern HandGadgetSelection D_001D5F70_255F8 __asm__("D_001D5F70");
extern HandGadgetManipulator D_001D6160_255F8 __asm__("D_001D6160");
extern HandGadgetManipulator D_001D61A0_255F8 __asm__("D_001D61A0");
extern HandGadgetManipulator D_001D61E0_255F8 __asm__("D_001D61E0");
extern float D_001D6220[];
extern s32 D_001D6238_255F8[] __asm__("D_001D6238");
extern void func_001E97F8(Moby *, s32);
extern void func_0020D960_255F8(s32, s32, HandGadgetManipulator *) __asm__("func_0020D960");
extern void func_0020D9D8_255F8(s32, HandGadgetManipulator *) __asm__("func_0020D9D8");
extern void func_00213DE0(void *, int, int, int);
extern Moby *func_00226720_255F8(s32) __asm__("func_00226720");
extern Moby *func_002267C0_255F8(Moby *) __asm__("func_002267C0");
extern void func_00227100(s32, Moby *, Moby *, s32 *, s32 *, s32 *);
extern s32 func_00227890(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);
extern s32 func_002279D0(void);
extern void func_00225E00();
extern void func_00225FB8();
extern void func_002260A8();
extern void func_00226250();
extern void func_00226380();
extern void func_00226410();

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/textbin/gameplay/gadgets/load_hand_gadget.c, load_hand_gadget. */
s32 func_002255F8(HandGadgetState *hand) {
    Moby *moby;
    s32 previous_selected_class;
    s32 previous_animation_class;
    s32 previous_attachment_class;
    s32 previous_pose_class;
    s32 previous_class_0197;
    s32 previous_class_0266;
    s32 previous_class;
    s32 requested_class;
    s32 selected_class;
    s32 selected_gadget;
    s32 loaded_gadget;
    s32 slot_index;
    s32 velocity_offset;
    HandGadgetAnimation *animation;
    float *ammo_offset;
    s32 selected_class_ready;
    Moby **ammo_moby_slot;
    loaded_gadget = 0;
    moby = hand->class_pose_moby;
    previous_selected_class = (moby != 0) ? (moby->oclass) : (-1);
    selected_gadget = D_001D5F70_255F8.selected_gadget;
    selected_class = D_001864D0_255F8[selected_gadget].oclass;
    selected_class_ready = selected_class == D_001D5F70_255F8.requested_resource_class;
    if ((previous_selected_class != selected_class) && selected_class_ready) {
        func_002267C0_255F8(moby);
        if (D_001D6160_255F8.active) {
            func_0020D9D8_255F8(hand->source_moby_address, &D_001D6160_255F8);
        }
        if ((D_0013F450_255F8.equipped_gadget != 0) &&
            (selected_gadget != D_0013F450_255F8.equipped_gadget)) {
            func_001E97F0(0, 0);
        }
        D_00160050 = D_001D5F70_255F8.D_00160050 == 0;
        func_00205270(selected_class, -1);
        D_001D5F70_255F8.active_resource_class = selected_class;
        D_001D5F70_255F8.D_00160050 = D_00160050;
        D_001D5F70_255F8.last_requested_resource_class = selected_class;
        D_001D5F70_255F8.last_resource_request_state = D_00160050;
        D_001B3580_255F8[D_001B3E40[selected_class]][0xD] = 0;
        moby = func_00226720_255F8(selected_class);
        if (moby != 0) {
            loaded_gadget = selected_gadget;
            if (D_0013E620[loaded_gadget]) {
                func_001E97F8(moby, hand->source_moby_address);
            }
            *((void **)moby->pvars) = hand;
            (*(void **)&moby->update) = func_00225E00;
            moby->flags = 4;
            if (loaded_gadget == 0x12) {
                func_0020D960_255F8(hand->source_moby_address, 0, &D_001D6160_255F8);
                D_001D6160_255F8.rotation_x = 0;
                D_001D6160_255F8.rotation_y = 0;
                D_001D6160_255F8.rotation_z = 0;
            }
        }
        hand->class_pose_moby = moby;
    }
    moby = hand->animation_moby;
    previous_animation_class = (moby != 0) ? (moby->oclass) : (-1);
    requested_class = D_001864D0_255F8[D_001D5F70_255F8.animation_gadget].oclass;
    if (previous_animation_class != requested_class) {
        moby = func_002267C0_255F8(moby);
        if (requested_class != (-1)) {
            moby = func_00226720_255F8(requested_class);
            if (moby != 0) {
                loaded_gadget = D_001D5F70_255F8.animation_gadget;
                *((void **)moby->pvars) = hand;
                (*(void **)&moby->update) = func_00225FB8;
                moby->flags = 4;
            }
        }
        hand->animation_moby = moby;
    }
    moby = hand->first_attachment_moby;
    previous_attachment_class = (moby != 0) ? (moby->oclass) : (-1);
    requested_class = D_001864D0_255F8[D_001D5F70_255F8.attachment_gadget].oclass;
    if (previous_attachment_class != requested_class) {
        moby = func_002267C0_255F8(moby);
        if (D_001D61A0_255F8.active) {
            func_0020D9D8_255F8(hand->source_moby_address, &D_001D61A0_255F8);
        }
        if (D_001D61E0_255F8.active) {
            func_0020D9D8_255F8(hand->source_moby_address, &D_001D61E0_255F8);
        }
        if (requested_class != (-1)) {
            moby = func_00226720_255F8(requested_class);
            if (moby != 0) {
                loaded_gadget = D_001D5F70_255F8.attachment_gadget;
                *((void **)moby->pvars) = hand;
                (*(void **)&moby->update) = func_002260A8;
                moby->flags = 4;
                func_0020D960_255F8(hand->source_moby_address, 0x16, &D_001D61A0_255F8);
                func_0020D960_255F8(hand->source_moby_address, 0x17, &D_001D61E0_255F8);
                D_001D61E0_255F8.rotation_z =
                    (D_001D61A0_255F8.rotation_z =
                         (D_001D61E0_255F8.rotation_y =
                              (D_001D61E0_255F8.rotation_x =
                                   (D_001D61A0_255F8.rotation_y =
                                        (D_001D61A0_255F8.rotation_x = 0.01f)))));
            }
        }
        hand->first_attachment_moby = moby;
        moby = func_002267C0_255F8(hand->second_attachment_moby);
        if (requested_class != (-1)) {
            moby = func_00226720_255F8(requested_class);
            if (moby != 0) {
                *((void **)moby->pvars) = hand;
                (*(void **)&moby->update) = func_002260A8;
                moby->flags = 4;
            }
        }
        hand->second_attachment_moby = moby;
    }
    moby = hand->pose_moby;
    previous_pose_class = (moby != 0) ? (moby->oclass) : (-1);
    requested_class = D_001864D0_255F8[D_001D5F70_255F8.pose_gadget].oclass;
    if (previous_pose_class != requested_class) {
        moby = func_002267C0_255F8(moby);
        if (requested_class != (-1)) {
            moby = func_00226720_255F8(requested_class);
            if (moby != 0) {
                *((void **)moby->pvars) = hand;
                (*(void **)&moby->update) = func_00226250;
                moby->flags = 4;
                if (moby->oclass == 0x25F) {
                    if (moby->prev_seq != 6) {
                        func_00213DE0(moby, 6, 0, 10);
                    }
                    moby->state = 8;
                }
            }
        }
        hand->pose_moby = moby;
    }
    moby = hand->class_0197_moby;
    previous_class_0197 = (moby != 0) ? (moby->oclass) : (-1);
    requested_class = (D_0013D5C8_1E950[0x23]) ? (0x197) : (-1);
    if (previous_class_0197 != requested_class) {
        moby = func_002267C0_255F8(moby);
        if (requested_class != (-1)) {
            moby = func_00226720_255F8(requested_class);
            if (moby != 0) {
                *((void **)moby->pvars) = hand;
                (*(void **)&moby->update) = func_00226380;
                moby->flags = 4;
            }
        }
        hand->class_0197_moby = moby;
    }
    moby = hand->class_0266_moby;
    previous_class_0266 = (moby != 0) ? (moby->oclass) : (-1);
    requested_class = (D_0013D5C8_1E950[0x21]) ? (0x266) : (-1);
    if (previous_class_0266 != requested_class) {
        moby = func_002267C0_255F8(moby);
        if (requested_class != (-1)) {
            moby = func_00226720_255F8(requested_class);
            if (moby != 0) {
                *((void **)moby->pvars) = hand;
                (*(void **)&moby->update) = func_00226380;
                moby->flags = 4;
            }
        }
        hand->class_0266_moby = moby;
    }
    moby = hand->class_026a_moby;
    previous_class = (moby != 0) ? (moby->oclass) : (-1);
    requested_class = (D_0013D5C8_1E950[0x22]) ? (0x26A) : (-1);
    if (previous_class != requested_class) {
        moby = func_002267C0_255F8(moby);
        if (requested_class != (-1)) {
            moby = func_00226720_255F8(requested_class);
            if (moby != 0) {
                *((void **)moby->pvars) = hand;
                (*(void **)&moby->update) = func_00226380;
                moby->flags = 4;
            }
        }
        hand->class_026a_moby = moby;
    }
    slot_index = 0;
    ammo_offset = D_001D6220;
    ammo_moby_slot = hand->ammo_mobys;
    do {
        moby = *ammo_moby_slot;
        previous_class = (moby != 0) ? (moby->oclass) : (-1);
        requested_class = (slot_index < D_0013F450_255F8.ammo_capacity) ? (0x1DF) : (-1);
        if (previous_class != requested_class) {
            moby = func_002267C0_255F8(moby);
            if (requested_class != (-1)) {
                moby = func_00226720_255F8(requested_class);
                velocity_offset = slot_index * 4;
                *ammo_offset = (slot_index < D_0013F450_255F8.ammo_used) ? (0.0f) : (3.0f);
                *(s32 *)((u8 *)D_001D6238_255F8 + velocity_offset) = 0;
                if (moby != 0) {
                    *((void **)moby->pvars) = hand;
                    (*(void **)&moby->update) = func_00226410;
                    moby->flags = 4;
                    moby->unkBC = slot_index;
                }
            }
            *ammo_moby_slot = moby;
        }
        slot_index++;
        ammo_moby_slot++;
        ammo_offset++;
    } while (slot_index < 8);
    if (((loaded_gadget != D_001D5F70_255F8.current_gadget) && (loaded_gadget > 0)) &&
        (loaded_gadget < 0x24)) {
        D_001D5F70_255F8.current_gadget = loaded_gadget;
        func_002279D0();
        func_00227890(D_001D5668[loaded_gadget].primary_animation +
                                    D_001D5F70_255F8.animation_base,
                                0, D_001D5668[loaded_gadget].delay_frames, loaded_gadget,
                                D_001D5668[loaded_gadget].item_animation,
                                D_001D5668[loaded_gadget].attachment0_class,
                                D_001D5668[loaded_gadget].attachment0_animation,
                                D_001D5668[loaded_gadget].attachment1_class,
                                D_001D5668[loaded_gadget].attachment1_animation,
                                D_001D5668[loaded_gadget].attachment2_class,
                                D_001D5668[loaded_gadget].attachment2_animation,
                                D_001D5668[loaded_gadget].resource_first,
                                D_001D5668[loaded_gadget].resource_count);
        if (D_001D5668[loaded_gadget].delay_frames != 0) {
            func_00227890(D_001D5668[loaded_gadget].secondary_animation +
                                        D_001D5F70_255F8.animation_base,
                                    2, 0, loaded_gadget, 1, -1, 0, -1, 0, -1, 0, 0, 0);
        }
    }
    for (slot_index = 0; slot_index < 0x18; slot_index++) {
        if (hand->timers[slot_index] != 0) {
            hand->timers[slot_index]--;
        }
    }

    func_00227100(hand->source_moby_address, hand->class_pose_moby,
                                             hand->animation_moby, &hand->x60, &hand->x64,
                                             &hand->x68);
    return 0;
}

void func_00225DF0(void) {
}

void func_00225DF8(void) {
}

extern char D_001864D0_a[] __asm__("D_001864D0");
extern int func_0020E3D0(void *);
extern void func_0020DAF8(int, int, void *);
extern void func_00214F78(void *);
extern void func_0020EEE8(void *);
extern void func_001E9800(void *, void *, int, int, int);

/* The pause-screen mobys these updates drive. */
typedef struct {
    char pad00[0x10];
    float pos[4];       /* 0x10 */
    char pad20[4];
    int sound;          /* 0x24 */
    char pad28[0x28];
    int x50;            /* 0x50 */
    int x54;            /* 0x54 */
    char pad58[0x10];
    char *x68;          /* 0x68 */
    char *x6C;          /* 0x6C */
    char pad70[8];
    char **cls;         /* 0x78 */
    char pad7C[0x2A];
    short oclass;       /* 0xA6 */
    char padA8[0x18];
    float mtx[16];      /* 0xC0 */
} PauseMoby;

/* D_001864D0's 0x4C-byte per-class records. */
typedef struct {
    char pad00[0xC];
    int bone;           /* 0x0C */
    int cls;            /* 0x10 */
    char pad14[4];
    int still;          /* 0x18 */
    char pad1C[0x30];
} PauseClassRec;

extern PauseClassRec D_001864D0_r[] __asm__("D_001864D0");
extern void func_0020D9D8(int, void *);
extern void func_0020D960(int, int, void *);

/* Moby update with a per-class record: find the class in D_001864D0 (0x4C
   bytes each, 0x25 of them), refresh the matrix from the record's bone,
   and either register the moby (+0x18 set) or restart its idle sound;
   also re-sync the D_001D6160 animation group while its +1 flag is up.
   The two store groups are in the order that schedules as retail's. */
void func_00225E00(void *arg0) {
    PauseMoby *m = arg0;
    unsigned char *b = arg0;
    float mtx[16];
    int id = *(int *)(*m->cls + 0x44);
    int i;
    int still;
    int sync;
    unsigned char *s;

    if ((b[0x70] & 2) && b[0x53] != 1) {
        func_00213DE0(m, 1, 0, 0);
    }
    for (i = 0; i < 0x25; i++) {
        if (D_001864D0_r[i].cls == m->oclass) {
            break;
        }
    }
    func_0020DAF8(id, D_001864D0_r[i].bone, mtx);
    qcopy(m->pos, &mtx[12]);
    func_0020ED48(m);
    still = D_001864D0_r[i].still == 0;
    func_001FA480(m->mtx, mtx);
    if (!still) {
        normalize_vector_triplet(m->mtx);
    }
    func_0020EEE8(m);
    sync = 0;
    s = (unsigned char *)D_001D6160;
    if (s[1] != 0) {
        sync = 1;
        DetachManipulator(id, s);
    }
    if (still) {
        func_001E9800(D_001864D0_a, D_001864D0, m->sound, 0, id);
        m->x68 = D_001864D0;
        m->x54 = 0;
        m->x6C = D_001864D0;
        *(int *)(b + 0x58) = 0;
        m->x50 = 0;
    }
    if (sync) {
        AttachManipulator(id, 0, s);
        *(int *)(s + 0x20) = 0;
        *(int *)(s + 0x24) = 0;
        *(int *)(s + 0x28) = 0;
    }
}

/* Moby update: refresh its matrix from bone 4 of its class (+0x44),
   copying the translation row to the position, re-register it, start
   its idle sound (6 for class 0x1B1, else 0) on the D_001864D0 table
   and reset the sound fields. */
void func_00225FB8(PauseMoby *m) {
    float mtx[16];
    int id = *(int *)(*m->cls + 0x44);

    func_0020E3D0(m);
    func_0020ED48(m);
    func_0020DAF8(id, 4, mtx);
    qcopy(m->pos, &mtx[12]);
    func_001FA480(m->mtx, mtx);
    normalize_vector_triplet(m->mtx);
    func_0020EEE8(m);
    if (m->oclass == 0x1B1) {
        func_001E9800(D_001864D0_a, D_001864D0, m->sound, 6, id);
    } else {
        func_001E9800(D_001864D0_a, D_001864D0, m->sound, 0, id);
    }
    m->x50 = 0;
    m->x68 = D_001864D0;
    m->x54 = 0;
    m->x6C = D_001864D0;
}

typedef struct {
    char pad00[0x10];
    f32 pos[4];
    char pad20[4];
    s32 resource_address;
    char pad28[0x28];
    s32 binding_state;
    s32 binding_blend_word;
    char pad58[0x10];
    char *primary_binding;
    char *secondary_binding;
    char pad70[8];
    char **vars;
    char pad7C[0x2A];
    s16 oclass;
    char padA8[0x18];
    f32 basis[12];
    f32 cached_vector[4];
} PauseMoby_260A8;

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/textbin/fun_00224e18.c, update_menu_preview_pose_and_attachments. */
void func_002260A8(void *preview) {
    PauseMoby_260A8 *moby = preview;
    char **preview_vars = moby->vars;
    f32 transform[16];
    s32 source_moby_address = *(int *)(*preview_vars + 0x44);
    s32 is_second_preview_moby;
    s32 first_attachment_active;
    s32 second_attachment_active;

    func_0020E3D0(moby);
    func_0020ED48(moby);
    is_second_preview_moby = moby == *(PauseMoby_260A8 **)(*preview_vars + 0x5C);
    func_0020DAF8(source_moby_address, is_second_preview_moby ? 3 : 2, transform);
    qcopy(moby->pos, &transform[12]);
    func_001FA480(moby->basis, transform);
    normalize_vector_triplet(moby->basis);
    func_0020EEE8(moby);
    first_attachment_active = 0;
    second_attachment_active = 0;
    if ((unsigned char)D_001D61E0[1] != 0) {
        first_attachment_active = 1;
        DetachManipulator(source_moby_address, D_001D61E0);
    }
    if ((unsigned char)D_001D61A0[1] != 0) {
        second_attachment_active = 1;
        DetachManipulator(source_moby_address, D_001D61A0);
    }
    if (!is_second_preview_moby) {
        func_001E9800(D_001864D0_a, D_001864D0,
                        moby->resource_address, 0, source_moby_address);
        moby->primary_binding = D_001864D0;
        moby->secondary_binding = D_001864D0;
    } else {
        func_001E9800(D_001864D0, D_001864D0_a,
                        moby->resource_address, 0, source_moby_address);
        moby->primary_binding = D_001864D0_a;
        moby->secondary_binding = D_001864D0_a;
    }
    if (first_attachment_active) {
        AttachManipulator(source_moby_address, 0x17, D_001D61E0);
        *(int *)(D_001D61E0 + 0x20) = 0;
        *(int *)(D_001D61E0 + 0x24) = 0;
        *(int *)(D_001D61E0 + 0x28) = 0;
    }
    if (second_attachment_active) {
        AttachManipulator(source_moby_address, 0x16, D_001D61A0);
        *(int *)(D_001D61A0 + 0x20) = 0;
        *(int *)(D_001D61A0 + 0x24) = 0;
        *(int *)(D_001D61A0 + 0x28) = 0;
    }
    moby->binding_blend_word = 0;
    moby->binding_state = 0;
}


/* Moby 0x259 update: while flag 0x02 is set, keep its animation on 1 (or,
   for class 0x25F in state 6, run the 6 animation until its timer +0x20
   runs out and then blend back to 1), then refresh its matrix from bone 5
   as func_00225FB8 does. */
void func_00226250(void *arg0) {
    PauseMoby *m = arg0;
    unsigned char *b = (unsigned char *)m;
    float mtx[16];
    int id = *(int *)(*m->cls + 0x44);

    if (b[0x70] & 2) {
        if (m->oclass == 0x25F) {
            if (b[0x52] == 6) {
                if (--b[0x20] == 0) {
                    if (b[0x53] != 1) {
                        func_00213DE0(m, 1, 0, 10);
                    }
                } else if (b[0x53] != 6) {
                    func_00213DE0(m, 6, 0, 0);
                }
            } else if (b[0x53] != 1) {
                func_00213DE0(m, 1, 0, 0);
            }
        } else if (b[0x53] != 1) {
            func_00213DE0(m, 1, 0, 0);
        }
    }
    func_0020DAF8(id, 5, mtx);
    qcopy(m->pos, &mtx[12]);
    func_001FA480(m->mtx, mtx);
    normalize_vector_triplet(m->mtx);
    func_0020EEE8(m);
}

/* func_00225FB8's sibling: refresh the matrix from bone 0x1E (class 0x197) or 0x1D, copy its translation row to the position and re-register. func_0020E3D0 takes the moby, and the bone test is written == 0x197 so the movn picks 0x1D as retail does. */
void func_00226380(PauseMoby *m) {
    float mtx[16];
    int id = *(int *)(*m->cls + 0x44);

    func_0020E3D0(m);
    func_0020ED48(m);
    func_0020DAF8(id, m->oclass == 0x197 ? 0x1E : 0x1D, mtx);
    qcopy(m->pos, &mtx[12]);
    func_001FA480(m->mtx, mtx);
    normalize_vector_triplet(m->mtx);
    func_0020EEE8(m);
}

struct AmmoPreviewMoby;
struct AmmoPreviewOwner {
    u8 pad00[0x44];
    struct AmmoPreviewMoby *source_moby;
};
struct AmmoPreviewVars {
    struct AmmoPreviewOwner *owner;
};
struct AmmoPreviewResource {
    u8 pad00[0x24];
    f32 scale;
};
struct AmmoPreviewMoby {
    u8 pad00[0x10];
    f32 position[4];
    u8 pad20[4];
    struct AmmoPreviewResource *resource;
    u8 pad28[4];
    f32 scale;
    u8 pad30[0x48];
    struct AmmoPreviewVars *preview_vars;
    u8 pad7C[0x40];
    u8 slot;
    u8 padBD[3];
    f32 basis[12];
};
extern f32 D_0015EE6C MACRO_ADDR;
extern short D_0015EE70;
extern f32 D_001D6220[];
extern f32 D_001D6238[];
extern void func_001F9BC0(void *);
extern f32 func_001F9F90(f32);
extern f32 func_001F9FA8(f32);
extern f32 func_001FA748(f32, f32);
extern f32 func_001FA7D8_26410(f32) __asm__("func_001FA7D8");
extern f32 func_001FA888(s32);
extern void func_0020E3D0_26410(void *) __asm__("func_0020E3D0");
extern f32 func_00214D88(f32 *, f32 *, f32, f32, f32, f32);
void func_00226410(struct AmmoPreviewMoby *moby);

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/ui/menus/item_preview/update_ammo_preview_transform.c, update_ammo_preview_transform. */
void func_00226410(struct AmmoPreviewMoby *moby) {
    f32 offset[4];
    f32 rotation_basis[12];
    f32 angles[4];
    struct AmmoPreviewMoby *source_moby;
    f32 *basis;
    s32 phase_index;
    f32 phase;
    f32 orbit_angle;
    f32 phase_angle;
    f32 bob_angle;
    f32 double_phase;
    f32 zero;

    source_moby = moby->preview_vars->owner->source_moby;
    func_0020E3D0_26410(moby);
    func_0020ED48(moby);
    qcopy(moby->position, source_moby->position);
    basis = moby->basis;
    func_001FA480(basis, source_moby->basis);
    normalize_vector_triplet(basis);
    moby->scale = moby->resource->scale;
    if (moby->slot < 3U) {
        phase_index = (moby->slot * 2) % 6;
    } else {
        phase_index = (moby->slot * 2 + 1) % 6;
    }
    orbit_angle = (((f32)(D_0015F538 % 200) / func_001FA888(200)) * 6.28318f) - 3.14159f;
    phase = (f32)phase_index;
    phase_angle = ((phase * 6.28318f) / func_001FA888(6)) - 3.14159f;
    bob_angle = (((f32)(D_0015F538 % 170) / func_001FA888(170)) * 6.28318f) - 3.14159f;
    double_phase = (phase * 12.56636f) / func_001FA888(6);
    func_001FA7D8_26410(double_phase);
    zero = 0.0f;
    orbit_angle = FastAddRots(orbit_angle, phase_angle);
    bob_angle = FastAddRots(bob_angle, double_phase);
    if (D_001D6220[moby->slot] != zero) {
        func_00214D88(&D_001D6220[moby->slot],
                                  &D_001D6238[moby->slot],
                                  zero, 1.0f, (*(f32 *)&D_0015EE70) * 6.0f, D_0015EE6C * 6.0f);
    }
    offset[0] = FastCos(orbit_angle);
    offset[1] = FastSin(orbit_angle);
    offset[2] = zero;
    offset[2] = FastSin(bob_angle) * 0.25f + 0.5f + D_001D6220[moby->slot];
    func_001F9EC0(offset, offset, source_moby->basis);
    FastVecAdd(moby->position, moby->position, offset);
    clear_u64_value(angles);
    angles[2] = FastAddRots(orbit_angle, 1.5707964f);
    func_001FA1F8(rotation_basis, angles);
    func_001FA4F0(moby->basis, rotation_basis, moby->basis);
    func_0020EEE8(moby);
}

/* Hoisted from the func_00227A70 block below so this earlier caller can
   see it -- a second NOT_SDA extern for the same symbol is a hard
   error. */
extern unsigned char D_001B3E40[] NOT_SDA;
extern void *func_0020D348(void);
extern void func_0020E340(void *, int, int, int, int);

extern void *func_0020D348_c(int) __asm__("func_0020D348");

/* Spawns the pickup/marker object for slot arg0, unless the slot is
   disabled (0xFF in D_001B3E40). A fresh object gets 0xFF/0xFF/1 in the
   0x30 block, is registered, tinted mid-grey, and flagged 0x18 at +0x73
   when its descriptor says so. The +0x30 store is through `unsigned
   char` and comes before the halfword store, so the 0xFF stays in one
   saved register. CreateMoby takes oClass: passing arg0 on keeps $a0
   live, which puts the element address in $v0 as retail has it. */
void *func_00226720(int arg0) {
    char *o;

    if (D_001B3E40[arg0] == 0xFF) {
        return 0;
    }
    o = (char *)func_0020D348_c(arg0);
    if (o != 0) {
        *(unsigned char *)(o + 0x30) = 0xFF;
        *(short *)(o + 0x32) = 0xFF;
        *(char *)(o + 0x20) = 0;
        *(char *)(o + 0x31) = 1;
        func_0020ED48(o);
        func_0020E340(o, 0x202020, 0xE, 0xE, 0);
        if (*(unsigned char *)(*(int *)(o + 0x24) + 6) != 0) {
            *(char *)(o + 0x73) = 0x18;
        }
    }
    return o;
}

extern int D_0015F6F0 MACRO_ADDR;
extern void func_0020D678(void *); /* DeleteMoby */

int func_002267C0(int arg0) {
    if (arg0 == 0) {
        return 0;
    }
    DeleteMoby((void *)arg0);
    *(long *)(arg0 + 0x38) = D_0015F6F0;
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/text", func_00226808);

/*
 * Exact once the short-loop padding retail's assembler adds is reproduced
 * (tools/fix_short_loops.py); it was the whole residual. Taking the
 * D_001517D0 base into a local declared after the calls keeps its
 * %hi/%lo out of a callee-saved register, as retail has it.
 */
int func_002268F0(void *arg0) {
    int *p = (int *)((char *)arg0 + 0x44);
    unsigned short *q;
    int i;
    for (i = 0x17; i >= 0; i--) { *p = func_002267C0(*p); p++; }
    *(int *)((char *)arg0 + 0x3C) =
        func_00226F68(*(int *)((char *)arg0 + 0x3C));
    q = (unsigned short *)D_001517D0;
    if ((unsigned int)(q[0x2D] - 6) >= 2) { q[0x2D] = 5; }
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/text", func_00226978);

extern void func_0020E180(int, int);

int func_00226CF8(void *arg0) {
    int *p = (int *)((char *)arg0 + 0x44);
    int i = 0x17;
    do {
        if (*p != 0) {
            DrawMobyList(*p, 1);
        }
        i--;
        p++;
    } while (i >= 0);
    return 4;
}

void func_00226D48(void) {
}

INCLUDE_ASM("asm/nonmatchings/text", func_00226D50);


extern int func_00227018(int handle);

/* Find the first binding that is enabled (bit 0 clear, or set when
   `invert` is given), still has a key and is not already claimed, claim
   it, and scrub its buffer with 0xDEADBEEF. Returns the key, or 0 if
   there is nothing to claim. */
int func_00226EA8(int invert) {
    int i;
    int f;
    int n;

    for (i = 0; i < 5; i++) {
        if (invert != 0) {
            f = D_001D6448_t[i].flags ^ 1;
        } else {
            f = D_001D6448_t[i].flags;
        }
        if ((f & 1) == 0 && D_001D6448_t[i].key != 0 &&
            (D_001D6448_t[i].flags & 2) == 0) {
            D_001D6448_t[i].flags |= 2;
            n = get_stream_buffer_size(D_001D6448_t[i].key);
            FastMemSet(D_001D6448_t[i].key, 0xDEADBEEF, n);
            return D_001D6448_t[i].key;
        }
    }
    return 0;
}

/* Release the binding whose key matches. Bit 1 means "bound"; bit 2 on
   top of that means it also owns the shared 0xCB latch, which has to be
   handed back through func_00217588 first. Always returns 0 so callers
   can assign it straight over their handle. The flags are read from the
   table at every use, not kept in a local. */
int func_00226F68(int key) {
    int i;
    char *g;

    for (i = 0; i < 5; i++) {
        if (D_001D6448_t[i].key == key) {
            if ((D_001D6448_t[i].flags & 2) != 0) {
                if ((D_001D6448_t[i].flags & 4) != 0) {
                    g = D_001D5F70;
                    D_001D6448_t[i].flags ^= 4;
                    if (*(unsigned char *)(g + 0xCB) != 0) {
                        request_audio_stream_break();
                        g[0xCB] = 0;
                    }
                }
                D_001D6448_t[i].flags &= ~2;
                return 0;
            }
        }
    }
    return 0;
}

extern int D_001D6448[];

int func_00227018(int arg0) {
    int *e = D_001D6448;
    int i = 0;
    do {
        if (e[0] == arg0) {
            return (e[1] & 1) ? 0x4F000 : 0x11800;
        }
        i++;
        e += 2;
    } while (i < 5);
    return -1;
}

int func_00227068(int arg0) {
    int i = 0;
    int *base = D_001D6448;
    int *e = base + 1;
    do {
        i++;
        if (e[-1] != arg0) {
            e += 2;
            continue;
        }
        e[0] |= 4;
        return 0;
    } while (i < 5);
    return 1;
}

/* Twin of func_00227068 above, clearing bit 2 instead of setting it.
   Written in exactly that function's shape -- the separate `base` local
   is what stops %lo+4 folding into one addiu, which is what an earlier
   round's revert was missing. */
int func_002270B0(int arg0) {
    int i = 0;
    int *base = D_001D6448;
    int *e = base + 1;
    do {
        i++;
        if (e[-1] != arg0) {
            e += 2;
            continue;
        }
        e[0] &= ~4;
        return 0;
    } while (i < 5);
    return 1;
}

INCLUDE_ASM("asm/nonmatchings/text", func_00227100);

extern int D_00160450 MACRO_ADDR;
typedef struct {
    int unk00;
    int arg[13];
} Rec38;
extern Rec38 D_001D6250[];

int func_00227890(int a0, int a1, int a2, int a3, int a4, int a5, int a6,
                  int a7, int a8, int a9, int a10, int a11, int a12) {
    int n = D_00160450;
    Rec38 *r;
    if (n >= 8) {
        return -1;
    }
    D_00160450 = n + 1;
    r = &D_001D6250[n];
    r->arg[0] = a0;
    r->arg[1] = a1;
    r->arg[2] = a2;
    r->arg[3] = a3;
    r->arg[4] = a4;
    r->arg[5] = a5;
    r->arg[6] = a6;
    r->arg[7] = a7;
    r->arg[8] = a8;
    r->arg[9] = a9;
    r->arg[10] = a10;
    r->unk00 = 0;
    r->arg[11] = a11;
    r->arg[12] = a12;
    return 0;
}

/* Pops the head of the 8-entry queue at D_001D6250. Copying through
   `d = &D_001D6250[i - 1]; *d = d[1];` makes the store the loop's
   master giv; the plain form reverses the loop. */
int func_00227928(void) {
    int i;
    for (i = 1; i < 8; i++) {
        Rec38 *d = &D_001D6250[i - 1];
        *d = d[1];
    }
    D_00160450--;
    return 0;
}

extern int D_001D641C;

int func_002279D0(void) {
    if (D_001517D0[4] != 0) {
        char *g = D_001D5F70;
        if (*(unsigned char *)(g + 0xCB) != 0) {
            request_audio_stream_break();
            g[0xCB] = 0;
        }
    }
    D_001D641C = 0;
    D_00160450 = 0;
    return 0;
}

void func_00227A30(void *arg0, char *src) {
    char *d = (char *)arg0;
    unsigned short v0, v1;
    int w0, w1;

    *(short *)(d + 0) = 0;
    v0 = *(unsigned short *)(src + 0x24);
    *(short *)(d + 4) = 0;
    *(short *)(d + 2) = v0;
    v1 = *(unsigned short *)(src + 0x20);
    *(short *)(d + 6) = v1;
    w0 = *(int *)(src + 0x20);
    w0 >>= 1;
    *(short *)(d + 8) = w0;
    w1 = *(int *)(src + 0x24);
    *(short *)(d + 0x10) = 0x10;
    w1 >>= 1;
    *(short *)(d + 0x12) = 0;
    *(short *)(d + 0xA) = w1;
}

extern char D_001D5D58[] NOT_SDA;
extern char *D_001B3580[] NOT_SDA;

typedef struct {
    u8 pad0[0x30];
    s32 active_items[3];
    u8 pad3C[0x64];
    s32 buffer_address[2];
    s32 resource_first;
    s32 resource_count;
    s32 resource_buffer_address[3];
    s32 read_offset;
    u8 padC0[8];
    u8 loaded_animation[2];
    u8 read_buffer_index;
    u8 pending_buffer;
    u32 streamed_animation_base;
} PreviewAnimationStreamState;
typedef struct {
    s32 class_id;
    s32 animation_index;
} PreviewResourceBinding;
extern u8 D_001B3580_27A70[] __asm__("D_001B3580");
extern u8 D_001B3E40[];
extern u8 D_001D5D58_27A70[] __asm__("D_001D5D58");
extern PreviewAnimationStreamState D_001D5F70_27A70 __asm__("D_001D5F70");
void func_00227A70(void);

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/ui/menus/item_preview/clear_preview_resource_bindings.c, clear_preview_resource_bindings. */
void func_00227A70(void) {
    register s32 class_slot;
    register s32 class_resource_address;
    s32 resource_index;
    register u8 *binding_base;
    PreviewResourceBinding *binding;
    register s32 animation_address;
    register s32 resource_first;
    register s32 resource_count;
    register s32 resource_end;

    resource_index = D_001D5F70_27A70.resource_first;
    if (resource_index < (resource_index + D_001D5F70_27A70.resource_count)) {
        u8 *class_slots = D_001B3E40;
        u8 *class_resources = D_001B3580_27A70;
        u8 *bindings = D_001D5D58_27A70;

        binding_base = bindings;
        binding = (PreviewResourceBinding *) ((resource_index * 8) + binding_base);
        do {
            resource_index += 1;
            class_slot = *((u8 *) (binding->class_id + (s32) class_slots));
            class_resource_address = *(s32 *) ((class_slot * 4) + class_resources);
            animation_address = class_resource_address + (binding->animation_index * 4);
            *(s32 *) (animation_address + 0x48) = 0;
            binding += 1;
            resource_first = D_001D5F70_27A70.resource_first;
            resource_count = D_001D5F70_27A70.resource_count;
            resource_end = resource_first + resource_count;
        } while (resource_index < resource_end);
    }
    D_001D5F70_27A70.resource_count = 0;
}

extern u8 D_001D60C8_27B00[] __asm__("D_001D60C8");
extern u8 D_001B3E40_27B00[] __asm__("D_001B3E40") MACRO_ADDR;
extern s32 func_00234350_27B00() __asm__("func_00234350");
extern void func_00203B18_27B00() __asm__("func_00203B18");
extern s32 func_0020C468_27B00() __asm__("func_0020C468");
extern s32 func_00227018_27B00() __asm__("func_00227018");
extern s32 func_00234238_27B00() __asm__("func_00234238");

/* Streams each resource of the preview's bindings into its buffer (with two bindings the
   second goes to buffer 2), unpacks it and points the class's animation slot at it.
   The class slot table's address is one `la` in retail (lui/addiu back to back), hence its
   MACRO_ADDR declaration; it is also what orders the loop index before the binding pointer.
   The guard tests the index (zero there): with `count > 0` the index lives twice as long.
   Adapted from Lombyte (MIT) for PAL: src/ui/menus/fun_00226848.c, load_preview_resource_bindings. */
void func_00227B00(s32 first_resource, s32 resource_count) {
    s32 count;
    s32 *class_resource_slot;
    s32 read_address;
    s32 buffer_address;
    s32 compressed_size;
    s32 resource_id;
    s32 animation_index;
    s32 buffer_offset;
    s32 buffer_skip;
    s32 resource_offset;
    u8 class_slot;
    PreviewResourceBinding *binding;
    s32 resource_index;

    count = resource_count;
    resource_index = 0;
    D_001D5F70_27A70.resource_first = first_resource;
    D_001D5F70_27A70.resource_count = count;
    if (resource_index < count) {
        resource_offset = first_resource * 4;
        binding = ((PreviewResourceBinding *)D_001D5D58_27A70) + first_resource;
        do {
            buffer_skip = 0;
            class_slot = D_001B3E40_27B00[binding->class_id];
            animation_index = binding->animation_index;
            if (count == 2) {
                if (resource_index == 1) {
                    buffer_skip = 1;
                } else {
                    buffer_skip = 0;
                }
            }
            binding += 1;
            resource_id = *((s32 *)(D_001D60C8_27B00 + resource_offset));
            resource_offset += 4;
            compressed_size = func_00234350_27B00(resource_id) * 0x10;
            buffer_offset = (resource_index + buffer_skip) * 4;
            resource_index += 1;
            buffer_address = *((s32 *)((u8 *)D_001D5F70_27A70.resource_buffer_address +
                                       buffer_offset));
            read_address =
                (buffer_address + func_00227018_27B00(buffer_address)) - compressed_size;
            func_00234238_27B00(read_address, resource_id, 0, -1, 0);
            func_0020C468_27B00(read_address, buffer_address);
            {
                s32 animation_offset = animation_index * 4;
                class_resource_slot = (s32 *)((class_slot * 4) + D_001B3580_27A70);
                *((s32 *)(((u8 *)((*class_resource_slot) + animation_offset)) + 0x48)) =
                    buffer_address;
            }
            func_00203B18_27B00(*class_resource_slot, animation_index);
        } while (resource_index < count);
    }
}

extern char D_0015EF98[] MACRO_ADDR;
extern char D_00141FC0[];
extern void func_00121A80(void *);
extern void func_0012D818(void *);
extern void func_00208FA0(void);
extern void func_00208338(void *);

void func_00227C78(int arg0, int arg1) {
    char *b = D_0013D390;
    func_00121A80(D_0015EF98);
    func_0012D818(D_0015EF98);
    func_00208FA0();
    func_00208338(D_00141FC0 + (D_0015EE84 << 11));
    memcard_MakeWholeSave((char *)arg0);
    *(int *)(b + 0xF4) = arg0;
    *(int *)(b + 0x14) = arg1;
    *(int *)(b + 0xC8) = 0;
    if (*(int *)(b + 0xE4) < 0) {
        *(int *)(b + 0xE8) = 0;
        *(int *)(b + 0xE4) = 0x13;
    }
}

/* func_00209DC0 (memcard.c) takes nothing. */
extern void func_00209DC0(void);

/* func_00227C78's sibling: runs func_00209DC0, the D_0015EF98 pair and
   func_0020BA00(arg0), then records arg0/arg1 in D_0013D390 (+0xF4,
   +0x14), clears +0xC8 and page arg1's first word, and seeds the result
   (+0xE4 = 0x13) when none is pending. func_00209DC0 is called without
   arguments: passing arg0 gives it one more reference and so the first
   callee-saved register, the reverse of retail's. */
void func_00227D20(int arg0, int arg1) {
    func_00209DC0();
    func_00121A80(D_0015EF98);
    func_0012D818(D_0015EF98);
    memcard_MakeWholeSave((char *)arg0);
    {
        char *b = D_0013D390;
        *(int *)(b + 0xC8) = 0;
        *(int *)(b + 0x14) = arg1;
        *(int *)(b + arg1 * 0x1C + 0x20) = 0;
        *(int *)(b + 0xF4) = arg0;
        if (*(int *)(b + 0xE4) < 0) {
            *(int *)(b + 0xE8) = 0;
            *(int *)(b + 0xE4) = 0x13;
        }
    }
}

/* Menu system state at D_001D5F70, as far as this function reads it. */
struct MenuSystem_27DB0 {
    u8 pad_0[0xE0];
    s32 unkE0;
};

extern struct MenuSystem_27DB0 D_001D5F70_27DB0 __asm__("D_001D5F70");
typedef struct {
    u8 pad0[0x14];
    s32 slot;
    u8 pad18[8];
    struct {
        s32 unk0;
        u8 pad4[0x18];
    } entries[5];
    u8 pad[0xC8 - 0x20 - 5 * 0x1C];
    s32 unkC8;
    u8 padCC[0x18];
    s32 unkE4;
    s32 unkE8;
    u8 padEC[8];
    s32 unkF4;
} Checkpoints_27DB0;

extern Checkpoints_27DB0 D_0013D390_27DB0 __asm__("D_0013D390");
extern u8 D_0013D490_27DB0[] __asm__("D_0013D490");
extern s32 D_0013D530_27DB0[] __asm__("D_0013D530");
extern u8 D_0013E620_27DB0[] __asm__("D_0013E620");
extern u8 D_0014BFC0_27DB0[] __asm__("D_0014BFC0");
extern s32 D_00141FA0_27DB0[] __asm__("D_00141FA0");
extern u8 D_0015EED0_27DB0[] __asm__("D_0015EED0") MACRO_ADDR;
extern s32 D_0015EEA0_27DB0 __asm__("D_0015EEA0") MACRO_ADDR;
extern u8 D_0015EEB0_27DB0[] __asm__("D_0015EEB0") MACRO_ADDR;
extern u8 D_0015EEC0_27DB0[] __asm__("D_0015EEC0") MACRO_ADDR;
extern s32 D_0015EE98_27DB0 __asm__("D_0015EE98") MACRO_ADDR;
extern u8 D_0015EF1C_27DB0[] __asm__("D_0015EF1C") MACRO_ADDR;
extern u8 D_0015EF1D_27DB0[] __asm__("D_0015EF1D") MACRO_ADDR;
extern s32 D_0015EF20_27DB0 __asm__("D_0015EF20") MACRO_ADDR;
extern u8 D_0015EF98_27DB0[] __asm__("D_0015EF98") MACRO_ADDR;
extern s32 D_001D5F20_27DB0[] __asm__("D_001D5F20");

extern void func_001F9A00_27DB0(void *dst, void *src, s32 size) __asm__("func_001F9A00");
extern void func_00209DC0_27DB0(void) __asm__("func_00209DC0");
extern void func_0020BA00_27DB0(s32) __asm__("func_0020BA00");
extern s32 func_00121A80_27DB0(u8 *clock) __asm__("func_00121A80");
extern void func_0012D818_27DB0(u8 *clock) __asm__("func_0012D818");

/* Resets the game state (func_00209DC0) while keeping what carries over: copies the kept
   blocks to the scratchpad, resets, copies them back (of D_0013D5C8 only the bytes listed in
   D_001D5F20; a quick-select entry is cleared when its item is gone), restores two flags and
   the counter + 1, re-reads the clock, and for slot >= 0 records the slot in the card state.
   PAL keeps two more 12-byte blocks (D_0015EEB0, D_0015EEC0).
   Adapted from Lombyte (MIT) for PAL: src/textbin/fun_00226b08.c, FUN_00226b08. */
void func_00227DB0(s32 slot) {
    s32 saved;
    u32 count;
    u8 flag4;
    u8 flag5;
    s32 *items = (s32 *)0x70000150;
    u8 *bytes;
    s32 i;
    s32 j;

    func_001F9A00_27DB0((void *)0x70000000, D_0013E620_27DB0, 0x28);
    func_001F9A00_27DB0((void *)0x70000030, D_0013D5C8_1E950, 0x25);
    func_001F9A00_27DB0((void *)0x70000060, D_0013D530_27DB0, 0x94);
    func_001F9A00_27DB0((void *)0x70000100, D_0014BFC0_27DB0, 0x50);
    func_001F9A00_27DB0((void *)0x70000150, D_00141FA0_27DB0, 0x20);
    func_001F9A00_27DB0((void *)0x70000170, D_0015EED0_27DB0, 0xC);
    func_001F9A00_27DB0((void *)0x70000180, D_0013D510_20DF0, 0x20);
    func_001F9A00_27DB0((void *)0x700001A0, D_0015EEB0_27DB0, 0xC);
    func_001F9A00_27DB0((void *)0x700001B0, D_0015EEC0_27DB0, 0xC);
    saved = D_0015EE98_27DB0;
    count = D_0015EF20_27DB0;
    flag4 = D_0013D490_27DB0[4] != 0;
    flag5 = D_0013D490_27DB0[5] != 0;
    bytes = (u8 *)0x70000030;
    func_00209DC0_27DB0();
    func_001F9A00_27DB0(D_0013E620_27DB0, (void *)0x70000000, 0x28);
    for (j = 0; D_001D5F20_27DB0[j] != -1; j++) {
        D_0013D5C8_1E950[D_001D5F20_27DB0[j]] = bytes[D_001D5F20_27DB0[j]];
    }
    func_001F9A00_27DB0(D_0013D530_27DB0, (void *)0x70000060, 0x94);
    func_001F9A00_27DB0(D_0014BFC0_27DB0, (void *)0x70000100, 0x50);
    for (i = 0; i < 8; i++) {
        s32 item = items[i];
        D_00141FA0_27DB0[i] = D_0013D5C8_1E950[item] ? item : 0;
    }
    func_001F9A00_27DB0(D_0015EED0_27DB0, (void *)0x70000170, 0xC);
    func_001F9A00_27DB0(D_0013D510_20DF0, (void *)0x70000180, 0x20);
    func_001F9A00_27DB0(D_0015EEB0_27DB0, (void *)0x700001A0, 0xC);
    func_001F9A00_27DB0(D_0015EEC0_27DB0, (void *)0x700001B0, 0xC);
    D_0015EE98_27DB0 = saved;
    if (flag4) {
        D_0013D490_27DB0[4] = 1;
        D_0015EEA0_27DB0 = 5;
    }
    if (flag5) {
        D_0013D490_27DB0[5] = 1;
        D_0015EEA0_27DB0 = 8;
    }
    D_0015EF20_27DB0 = count + 1;
    D_0015EF1C_27DB0[0] = 0;
    D_0015EF1D_27DB0[0] = 0;
    func_00121A80_27DB0(D_0015EF98_27DB0);
    func_0012D818_27DB0(D_0015EF98_27DB0);
    if (slot >= 0) {
        D_0013D390_27DB0.slot = slot;
        D_0013D390_27DB0.entries[slot].unk0 = 0;
        func_0020BA00_27DB0(D_001D5F70_27DB0.unkE0);
        D_0013D390_27DB0.unkF4 = D_001D5F70_27DB0.unkE0;
        D_0013D390_27DB0.unkC8 = 0;
        if (D_0013D390_27DB0.unkE4 < 0) {
            D_0013D390_27DB0.unkE8 = 0;
            D_0013D390_27DB0.unkE4 = 0x13;
        }
    }
}

extern int D_0015F6D0 MACRO_ADDR;
extern int D_0015EE80 MACRO_ADDR;

void func_00228110(void) {
    int t;
    if (D_0015F6D0 != 0) {
        return;
    }
    t = D_0015EF24;
    D_0015EF24 = t + 1;
    if ((t + 1) % 50 == 0 && D_0015EE80 != 0) {
        D_0015EF24 = t + 0xB;
    }
}

extern int func_00216198(void);
extern int func_00216150(void);
extern char D_001D2B80[];
extern char D_001D2BF8[];

/* Unlock states for the pause menu's reward rows: with at least 15 / 30
   of the D_0013D510 flags (func_00216198) and 10 of the D_0013E620 ones
   (func_00216150), rows 5-8 of the 12-byte item table D_001D2B80 get
   state 3 (10 for row 7) instead of 2, and the menu at D_001D2BF8 its
   text ids; row 8's first half is cleared while D_0015EF90 is set. The
   three tests are kept as 0/1 values. Each item store reads the table
   through its own `char *` (retail's kept %hi); the short selects stay
   branches (HImode has no conditional move), the int ones become
   movz/movn. */
void func_00228160(void) {
    int a = count_nonzero_entries_up_to_30() >= 15;
    int b = count_nonzero_entries_up_to_30() >= 30;
    int c = count_nonzero_entries_up_to_10() >= 10;

    {
        char *g = D_001D2B80;
        *(short *)(g + 0x3E) = a ? 3 : 2;
    }
    {
        char *g = D_001D2B80;
        *(short *)(g + 0x4A) = b ? 3 : 2;
    }
    {
        char *g = D_001D2B80;
        *(short *)(g + 0x56) = c ? 10 : 2;
    }
    {
        char *g = D_001D2B80;
        *(short *)(g + 0x62) = c ? 3 : 2;
    }
    {
        char *h = D_001D2BF8;
        *(int *)(h + 0x14) = a ? 0x4FD3 : 0x4FD9;
        *(int *)(h + 0x18) = b ? 0x4FD4 : 0x4FDA;
        *(int *)(h + 0x1C) = c ? 0x4FD7 : 0x4FDB;
        *(int *)(h + 0x20) = c ? 0x4FD8 : 0x4FDB;
    }
    if (D_0015EF90_m != 0) {
        char *g = D_001D2B80;
        *(short *)(g + 0x60) = 0;
    }
}
__asm__(".section .text\n\tnop\n\tnop\n");

extern int D_001D6860[];
extern int D_001D74C0[];
extern int D_001D6760[];

void func_00228268(void) {
    FastMemSet(D_001D6860, 0, 0xC60);
    FastMemSet(D_001D74C0, 0, 0xC60);
    FastMemSet(D_001D6760, 0, 0x100);
}

LINKER_REMNANT("asm/remnants/text", func_002282B8);

extern float D_00160470[] MACRO_ADDR;
extern float D_00160470_x __asm__("D_00160470");
extern float D_00160474;
extern float D_00160478;
extern float D_00160480[] MACRO_ADDR;
extern float D_00160480_x __asm__("D_00160480") MACRO_ADDR;
extern float D_00160484;
extern float D_00160488;
extern float D_00160490;
extern float D_00160494;
extern float D_00160498;
extern void func_001F9CA0(void *, void *, void *);

typedef struct {
    float v[4];
} __attribute__((aligned(16))) PauseVec;

/* Build a basis from dir: D_00160470 = dir normalised,
   D_00160490 = that scaled, and D_00160480 = the cross product with a
   vector built from dir's components reordered (the smallest moved), so
   the result is perpendicular, then normalised. */
typedef u32 u128_282D0 __attribute__((mode(TI), aligned(16)));
extern f32 D_00160470[] MACRO_ADDR;
extern f32 D_00160480[] MACRO_ADDR;
extern f32 D_00160490_282D0[] __asm__("D_00160490") MACRO_ADDR;
extern void func_001F9DC0_282D0(f32 *, f32 *, f32) __asm__("func_001F9DC0");
typedef union {
    u128_282D0 q;
    f32 v[4];
} PauseVec_282D0;

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/ui/menus/fun_00226fb8.c, FUN_00226fb8. */
void func_002282D0(PauseVec_282D0 *dir, f32 scale) {
    PauseVec_282D0 d;
    f32 *v = d.v;

    d.q = dir->q;
    func_001F9DC0_282D0(D_00160470, v, 1.0f);
    D_00160490_282D0[0] = D_00160470[0] * scale;
    D_00160490_282D0[1] = D_00160470[1] * scale;
    D_00160490_282D0[2] = D_00160470[2] * scale;
    if (v[0] < v[1]) {
        if (v[0] < v[2]) {
            D_00160480[0] = v[0];
            D_00160480[1] = v[2];
            D_00160480[2] = v[1];
        } else {
            D_00160480[0] = v[1];
            D_00160480[1] = v[0];
            D_00160480[2] = v[2];
        }
    } else if (v[1] < v[2]) {
        D_00160480[0] = v[2];
        D_00160480[1] = v[1];
        D_00160480[2] = v[0];
    } else {
        D_00160480[0] = v[1];
        D_00160480[1] = v[0];
        D_00160480[2] = v[2];
    }
    FastVecCross(D_00160480, D_00160480, D_00160470);
    func_001F9DC0_282D0(D_00160480, D_00160480, 1.0f);
}

/*
 * Dispatch on a leading short: 0 and 1 each call a handler and advance
 * the pointer differently, anything else returns it unchanged.
 *
 * An earlier round reverted this at 4 bytes short and read the residual
 * as a delay-slot problem -- retail spends the first jal's delay slot on
 * `addiu $16,$16,0x20`, we emitted a nop. The real cause was one level
 * up: with a `return` inside each arm, gcc folds the advance into the
 * return value (`addu $2,$16,32`, one instruction), and there is then
 * nothing left for the delay slot. Retail updates the pointer and copies
 * it to $v0 separately, three times, which is what a SINGLE `return p`
 * at the join gives: the copy belongs to the join block and the
 * delay-slot filler duplicates it into both branches.
 *
 * So this is the exit-cross-jumping lever used the other way round.
 * The usual reach is to SPLIT exits that gcc merged; here retail really
 * does share one, and the fix was to stop returning early. When a
 * 4-byte shortfall looks like a missing delay-slot fill, check first
 * whether an expression got folded that retail kept in two steps.
 */
void *func_00228400(char *p) {
    short v = *(short *)p;
    if (v == 0) {
        func_00229098(p);
        p += 0x20;
    } else if (v == 1) {
        func_002291E8(p);
        p += 0x30;
    }
    return p;
}

extern int *D_00161000 MACRO_ADDR;
extern char D_001D8120[];
extern short D_00160460;              /* SDA, gp -0x68A0 */
extern void func_002298B0(int, int);

void func_00228458(int arg0, int idx, int n) {
    D_00161000[0] = 0x30000003;
    D_00161000[1] = (int)(D_001D8120 + n * 0x30);
    D_00161000[2] = 0x13000000;
    D_00161000[3] = 0x50000003;
    D_00161000 += 4;
    func_002298B0(arg0, ((int *)&D_00160460)[idx]);
}

INCLUDE_ASM("asm/nonmatchings/text", func_002284E8);

/* The display mode block: only the two sizes this function reads. */
struct DisplayDims_28690 {
    u8 pad_0[0x150];
    s16 display_width; /* 0x150 */
    s16 display_height; /* 0x152 */
};
extern struct DisplayDims_28690 D_00151880_28690 __asm__("D_00151880");
extern s32 *D_00161000_28690 __asm__("D_00161000") MACRO_ADDR;

/* Appends a DMA packet that clears the screen in 32-pixel-wide sprite strips of the given colour.
   Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/textbin/fun_00227378.c, append_fullscreen_clear_strips. */
void func_00228690(s64 color) {
    s32 *base;
    s64 *commands;
    s64 *vertices;
    struct DisplayDims_28690 *dimensions;
    s32 strip_count;
    s32 display_height;
    s32 display_width;
    s32 strip_index;
    s32 left_x;
    s32 top_y;
    s32 right_x;
    s32 bottom_y;
    s32 negative_half_width;
    s32 dividend;
    s64 packed_top_y;
    s64 packed_bottom_y;
    s64 first;
    s64 second;

    dimensions = &D_00151880_28690;
    display_width = dimensions->display_width;
    display_height = dimensions->display_height;
    /* Retail truncates signed display width toward zero before packing strips. */
    dividend = (display_width > -1) ? display_width : (display_width + 0x1F);
    strip_count = dividend >> 5;
    strip_index = 0;
    D_00161000_28690[0] = (strip_count + 5) | 0x10000000;
    D_00161000_28690[1] = 0;
    D_00161000_28690[2] = 0;
    D_00161000_28690[3] = (strip_count + 5) | 0x50000000;
    base = D_00161000_28690;
    commands = (s64 *)(base + 4);
    D_00161000_28690 = (s32 *)commands;
    ((s64 *)base)[2] = 0x1000000000000001;
    commands[1] = 0xE;
    commands[2] = 0x3D801;
    commands[3] = 0x47;
    commands[4] = 0x2400000000000001;
    commands[5] = 0x10;
    commands[6] = 0x146;
    commands[7] = color;
    commands[8] = (s64)(strip_count | 0x8000) | 0x2400000000000000;
    commands[9] = 0x44;
    if (strip_index < strip_count) {
        bottom_y = display_height * 8 + 0x7FF0;
        top_y = 0x8000 - display_height * 8;
        negative_half_width = -(display_width * 8);
        packed_top_y = (s64)top_y << 16;
        left_x = negative_half_width + 0x8000;
        right_x = negative_half_width + 0x8200;
        packed_bottom_y = (s64)bottom_y << 16;
        vertices = (s64 *)(base + 0x18);
        do {
            first = (s64)left_x | packed_top_y;
            second = (s64)right_x | packed_bottom_y;
            *vertices = first;
            strip_index++;
            vertices++;
            right_x += 0x200;
            *vertices = second;
            left_x += 0x200;
            vertices++;
        } while (strip_index < strip_count);
    }
    D_00161000_28690 = (s32 *)((u8 *)D_00161000_28690 + (strip_count * 0x10 + 0x50));
    D_00161000_28690[0] = 0x10000000;
    D_00161000_28690[1] = 0;
    D_00161000_28690[2] = 0x13000000;
    D_00161000_28690[3] = 0;
    D_00161000_28690 += 4;
}
__asm__(".section .text\n\tnop\n");

typedef u32 u128 __attribute__((mode(TI), aligned(16)));
struct GraphicsSetupRecord {
    s32 command_count;
    s32 command_flags;
    f32 second_depth;
    f32 first_depth;
    u128 direction;
};
extern u8 * D_00161000_28860 __asm__("D_00161000") MACRO_ADDR;
struct VideoModeState {
    s32 v;
};
extern struct VideoModeState D_0015EE80_28860 __asm__("D_0015EE80") MACRO_ADDR;
extern u8 D_001D8250[];
extern u8 D_001D81E0[];
extern s32 D_001604A0 MACRO_ADDR;
extern void func_002284E8(void);
extern void func_002282D0_28860(f32 *, f32) __asm__("func_002282D0");
extern u8 *func_00228400_28860(u8 *) __asm__("func_00228400");
extern void func_00229838(f32 *, f32 *);
extern void func_00228D20(u32, s32, s32);
extern void func_00228458(s32, s32, s32);
extern void func_00234C98_28860(s32, s64) __asm__("func_00234C98");
extern void func_00228690(s64);

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/textbin/fun_00227548.c, submit_graphics_setup_command_stream. */
void func_00228860(u8 *command_stream) {
    f32 first_vector[4];
    f32 second_vector[4];
    f32 direction[4];
    struct GraphicsSetupRecord *record;
    s32 command_index;

    func_002284E8();
    record = (struct GraphicsSetupRecord *)command_stream;
    *(u32 *)(D_00161000_28860 + 0) = 0x30000007;
    *(u32 *)(D_00161000_28860 + 4) =
        (u32)(D_0015EE80_28860.v != 0 ? D_001D8250 : D_001D81E0);
    *(u32 *)(D_00161000_28860 + 8) = 0x13000000;
    *(u32 *)(D_00161000_28860 + 12) = 0x50000007;
    /* Remaining vector components are filled from each setup record. */
    second_vector[1] = first_vector[1] = second_vector[0] = first_vector[0] = 0.0f;
    D_00161000_28860 += 16;
    while (record->command_count != 0) {
        command_stream += 0x20;
        *(u128 *)direction = record->direction;
        func_002282D0_28860(direction, 1000.0f);
        first_vector[2] = record->first_depth;
        second_vector[2] = record->second_depth;
        for (command_index = 0; command_index < record->command_count; command_index++) {
            command_stream = func_00228400_28860(command_stream);
            func_00229838(first_vector, second_vector);
            func_00228D20(0x70000000, D_001604A0, record->command_flags);
            func_00228458(2, 1, 2);
            func_00228458(1, 0, 2);
        }
        record = (struct GraphicsSetupRecord *)command_stream;
    }
    if (D_0015EE80_28860.v) {
        func_00234C98_28860(0x4C, 0x80080);
    } else {
        func_00234C98_28860(0x4C, 0x80070);
    }
    func_00234C98_28860(0x42, 0x2000000064LL);
    func_00228690(0);
    func_00234C98_28860(0x47, 0x5360B);
    func_00234C98_28860(0x42, 0x8000000044LL);
}
