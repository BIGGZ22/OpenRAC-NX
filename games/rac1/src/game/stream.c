#include "common.h"
#include "callees.h"
#include "records.h"
#include "structs.h"
#include "pad.h"

/*
 * stream.cpp in the original source; text 0x217588-0x217F68.
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

extern int func_0012EFE8_i(void) __asm__("func_0012EFE8");

/* func_0012EFE8 returns int (989snd.c): with the result in $v0, the
   constant goes to the register retail uses. */
void request_audio_stream_break(void) {
    short *p = D_001517D0;
    if (p[4] != 0) {
        func_0012EFE8_i();
        ((char *)p)[0xA] = 1;
    }
}

typedef struct {
    char b[4];
} StreamHdr;
extern StreamHdr D_00151800;
/* MACRO_ADDR for retail's one-register lbu; the explicit size 4 (over
   -G2) is what keeps the assembler from expanding it through $gp. */
extern unsigned char D_0015EE58[4] MACRO_ADDR;
extern int D_0015EFBC MACRO_ADDR;
extern int D_0015EFC0 MACRO_ADDR;
extern void func_00121750(int, int, int, void *);

int Stream_SubmitRead(int arg0, int arg1, int arg2) {
    StreamHdr hdr = D_00151800;

    hdr.b[1] = D_0015EE58[0];
    D_0015EFBC = 0;
    D_0015EFC0 = 0;
    func_00121750(arg1, arg2, arg0, &hdr);
    return 1;
}

extern char D_001E8980[];
extern int func_0012EE98(int, int, int, void *);

/* Start a stream: refuse if one is already running (+0x8) or the length
   is zero, otherwise hand the request to func_0012EE98 with the header
   block at D_001517D0+0x30 and record it. Returns the length in the
   units the caller wants, arg2 << 11; on a rejected request it reports
   the failure and returns 0. */
int start_audio_stream_read(int arg0, int arg1, int arg2) {
    char *d = (char *)D_001517D0;

    if (*(short *)(d + 8) != 0) {
        return 0;
    }
    if (arg2 == 0) {
        return 0;
    }
    if (func_0012EE98(arg1, arg2, arg0, d + 0x30) == 0) {
        STUB_printf(D_001E8980);
        raise_kernel_trap();
        return 0;
    }
    *(int *)(d + 0x14) = arg0;
    *(short *)(d + 8) = 1;
    *(int *)(d + 0xC) = arg1;
    *(int *)(d + 0x10) = arg2;
    return arg2 << 11;
}

/* This caller sets up no arguments at all; func_00217628 is defined
   just above with three. Reach it through an alias rather than
   redeclaring it. */
extern int func_00217628_v(void) __asm__("func_00217628");
extern void func_00122598(int);
extern void func_00217130(void);
extern void func_0012EC40(void);
extern void func_0012DDC0(void);
extern void func_0012EC30(void);

/*
 * Close, not exact (15/124), same size so inert. Semantics confirmed:
 * call func_00217628, and while it succeeded and the flag at
 * D_001517D0+0x8 stays set, pump the five service calls; return the
 * original result either way.
 *
 * Residual is purely which callee-saved register holds what: retail puts
 * the result in $s0 and the D_001517D0 base in $s1, this compiler the
 * other way round, and the prologue save order follows from that.
 * Tried and did not move it: swapping the two local declarations (the
 * documented declaration-order lever -- it does not apply when a local
 * is initialised straight from a call, since the live range starts at
 * the call regardless), and restructuring the loop as guard + do/while
 * so the base local is assigned late exactly as retail does. Allocator
 * destination-choice question, not source shape.
 */
/* Load: call func_00217628 and, if it succeeded, keep pumping the five
   service calls while the D_001517D0+0x8 state word stays nonzero;
   return func_00217628's result either way.
   As in func_00217748 below, the base pointer is assigned in the loop
   condition: the exit test jump.c duplicates in front of the loop gets
   its own temporary, and the loop's copy goes to $s1 with the result in
   $s0, as in retail. */
/* Load */
int Load(int arg0, int arg1, int arg2) {
    char *d;
    int r = start_audio_stream_read(arg0, arg1, arg2);
    if (r != 0) {
        while (d = (char *)D_001517D0, *(short *)(d + 0x8) != 0) {
            func_00122598(0);
            music_Update();
            func_0012EC40();
            func_0012DDC0();
            func_0012EC30();
        }
    }
    return r;
}

/* Same pump loop as func_002176C8 above, gated on the argument instead
   of on a call's result: with arg0 set, keep servicing while the
   D_001517D0+8 state word stays nonzero; with it clear, run the four
   service calls exactly once. Either way return the state word. */
/* With arg0 set, keep pumping the five service calls while the
   D_001517D0+8 state word stays nonzero; with it clear, run the four that
   are not func_00122598 once. Either way return the state word.
   The base pointer is assigned IN the loop condition: the exit test that
   jump.c duplicates in front of the loop then gets its own pseudo (retail's
   $a0 for the guard), loop.c hoists the loop's copy into $s0 and CSE turns
   it into `daddu $s0,$a0,$0`. A second, separate local for the return keeps
   its base a temp ($v1) sharing the %hi with the guard through $s1. */
struct MusicStreamState {
    u8 pad_0[0x8];
    s16 read_state;            /* 0x08: 0 idle, 1 reading, 2 done */
};

short update_audio_stream_until_idle(int arg0) {
    if (arg0 != 0) {
        struct MusicStreamState *d;
        while (d = (struct MusicStreamState *)D_001517D0, d->read_state != 0) {
            func_00122598(0);
            music_Update();
            func_0012EC40();
            func_0012DDC0();
            func_0012EC30();
        }
    } else {
        music_Update();
        func_0012EC40();
        func_0012DDC0();
        func_0012EC30();
    }
    {
        struct MusicStreamState *d = (struct MusicStreamState *)D_001517D0;
        return d->read_state;
    }
}

extern int func_0012F030(void);

void finish_audio_stream_read(int callback_value) {
    struct MusicStreamState *state;
    if (callback_value == 1) {
        state = (struct MusicStreamState *)D_001517D0;
        state->read_state = 0;
        if (func_0012F030() != 0) {
            state->read_state = 2;
        }
    }
}

void music_channel_ready_callback(int arg0, long arg1) {
    short *p = (short *)(int)arg1;
    if (p != 0 && arg0 != 0 && p[5] == 2) {
        p[5] = 3;
    }
}

extern void func_002167C0(short, short, short);

void music_secondary_start_callback(int arg0, long arg1) {
    short *p = (short *)(int)arg1;
    if (p == 0) {
        return;
    }
    *(int *)p = arg0;
    if (arg0 != 0) {
        if (p[5] == 1) {
            p[5] = 2;
        }
    } else {
        short *b = D_001517D0;
        music_start_track_by_id(b[0x2A], b[0x2C], b[0x2B]);
    }
}

extern void func_002169B8(short, short, short);

void music_primary_preseek_callback(int arg0, long arg1) {
    short *p = (short *)(int)arg1;
    if (p == 0) {
        return;
    }
    *(int *)p = arg0;
    if (arg0 != 0) {
        if (p[5] == 1) {
            p[5] = 2;
        }
    } else {
        short *b = D_001517D0;
        music_PreseekTrack(b[0x1C], b[0x1E], b[0x1D]);
    }
}

extern short D_001517F0 NOT_SDA;

void music_transition_start_callback(int arg0, long arg1) {
    short *p = (short *)(int)arg1;
    if (p != 0) {
        *(int *)p = arg0;
        if (arg0 != 0) {
            short state = p[5];
            if (state == 1) {
                p[5] = 4;
                if (p[8] != 0) {
                    D_001517F0 = state;
                }
            }
        } else {
            p[5] = 0;
        }
    }
}

void music_primary_replace_callback(int arg0, long arg1) {
    short *p = (short *)(int)arg1;
    if (p != 0) {
        if (arg0 < 0) {
            *(int *)p = arg0;
        }
        if (arg0 != 0) {
            if (p[5] == 9) {
                p[5] = 4;
                if (p[8] != 0) {
                    D_001517F0 = 1;
                }
            }
        } else {
            p[5] = 0;
        }
    }
}

void music_primary_start_callback(int arg0, long arg1) {
    short *p = (short *)(int)arg1;
    if (p != 0) {
        *(int *)p = arg0;
        if (arg0 != 0) {
            if (p[5] == 1) {
                p[5] = 8;
            }
        } else {
            p[5] = 0;
        }
    }
}

extern char D_00160168[];

/* `*p = arg0` happens on the 0xFFFFFFFF path too (the older decode made
   it conditional), and D_001E8C60's message is printed. */
void set_sound_handle_id(int arg0, long arg1) {
    short *p = (short *)(int)arg1;
    if (p != 0) {
        if (*(unsigned int *)p != 0xFFFFFFFF) {
            STUB_printf(D_00160168);
        } else {
            *(int *)p = arg0;
            if (arg0 == 0) {
                p[5] = 7;
            }
        }
    }
}

void music_remaining_time_callback(int arg0, long arg1) {
    char *p = (char *)(int)arg1;
    short *b;
    if (p == 0) {
        return;
    }
    *(int *)(p + 0x18) = arg0;
    if (*(short *)(p + 0x10) == 0) {
        return;
    }
    b = D_001517D0;
    if (b[0x10] != 1) {
        return;
    }
    if (arg0 == 0) {
        return;
    }
    b[0x10] = 2;
    *(int *)((char *)b + 0x24) = *(int *)(p + 0x18);
    *(int *)((char *)b + 0x28) = *(int *)(p + 0x18) / 4;
}

LINKER_REMNANT("asm/remnants/text", func_00217AD0);

/* Adapted from Lombyte (GPL-3.0): src/ui/menus/fun_00216c48.c. */

typedef struct {
    s32 unk0;
    u16 unk4;
    s16 unk6;
    s16 unk8;
    s16 unkA;
    s16 unkC;
    s16 unkE;
    u16 unk10;
} MenuItem;

typedef struct {
    u8 pad0[0x4];
    u16 unk4;
    u8 pad6[0x2];
    u8 unk8;
    u8 pad9[0x2D];
    s16 unk36;
    s32 unk38;
    u8 *unk3C;
} Menu;

typedef struct {
    s32 unk0;
    u8 pad4[0x10];
    u16 unk14;
    u8 pad16[0x2];
} Weapon;

typedef struct {
    char *s[6];
} Suffixes;

extern s32 D_00160170 SDATA(D_00160170);
extern s32 D_0015F6F0 SDATA(D_0015F6F0);
extern s32 D_0015F6F0_far __asm__("D_0015F6F0") MACRO_ADDR;
extern s32 D_001994C0[];
extern Suffixes D_001E89B0;
extern char D_00160188[];
extern char D_00160190[];
extern s32 D_0015EE88 MACRO_ADDR;
extern Weapon D_001E02B0[];
extern s32 D_0015EE98 MACRO_ADDR;

extern u8 stream_item_flags[] __asm__("D_0013D5C8");
extern u8 D_0013D5F0[];
extern u8 D_0013D490[];

extern void func_001E97D8(s32);
extern s32 func_001FFCB0(s32);
extern void func_001E97E0(s32);
extern void *func_001FE540(s32);

extern char *func_00116CC0(char *, char *);
extern s32 func_00116248(char *, const char *, ...);
extern void func_001FFE88(void *);
extern s32 func_00216098(void);

void func_00217AE8(void *arg0, Menu *menu, s32 arg2);

void process_menu_entry(void *arg0, Menu *menu, s32 arg2) {
    char buf[0x100];
    Suffixes sfx;
    char num[0x20];
    MenuItem *e;
    char *p;
    s32 ok;
    s32 next;
    u16 flags;
    s16 n;
    s16 kind;
    s32 t;

    e = (MenuItem *)(menu->unk3C + menu->unk36 * 0x1C);
    if (menu->unk3C == 0) {
        return;
    }
    if (arg2 != 0) {
        menu->unk4 = menu->unk36;
        menu->unk38 = D_0015F6F0;
        n = e->unk6;
        if (menu->unk36 != n) {
            flags = e->unk10;
            menu->unk36 = e->unk6;
            e = (MenuItem *)(menu->unk3C + n * 0x1C);
            menu->unk8 = *(u8 *)&e->unk10 & 1;
            if (flags & 4) {
                D_001994C0[2] = (s32)arg0;
                D_001994C0[3] = (s32)menu;
                if (e->unk4 & 0x4000) {
                    noop_callback_n((s16)(e->unk4 ^ 0x4000));
                } else {
                    remove_hud_item(D_00160170);
                    D_00160170 = -1;
                    noop_callback_o((s16)e->unk4);
                }
            }
        }
    }
    if (menu->unk36 == -1) {
        return;
    }
    D_001994C0[5] = e->unk0;
    kind = e->unk8;
    if (kind == 6) {
        sfx = D_001E89B0;
        func_001166FC((Cfg13*)((char *)0x70000000), msg_string(e->unk0));
        p = func_00116CC0(((char *)0x70000000), D_00160188);
        if (p != 0) {
            p[1] = 's';
        }
        t = vendor_item_prices[e->unkA].unk14;
        func_00116248(num, D_00160190, t / 1000, sfx.s[D_0015EE88 % 6], t % 1000);
        func_00116248(buf, ((char *)0x70000000), num);
    } else {
        func_00116248(buf, msg_string(e->unk0));
    }
    if (D_001994C0[5] != 0) {
        copy_text_to_shared_buffer(buf);
    } else if (D_00160170 != -1) {
        remove_hud_item(D_00160170);
        D_00160170 = -1;
    }
    if (e->unk8 == 0) {
        return;
    }
    ok = 0;
    switch (e->unk8) {
    case 1:
        ok = !(gBolts < vendor_item_prices[e->unkA].unk0);
        break;
    case 2:
        ok = stream_item_flags[e->unkA] != 0;
        break;
    case 3:
        ok = 0;
        if (stream_item_flags[e->unkA] != 0) {
            ok = items_acquired[e->unkA] == 0;
        }
        break;
    case 4:
        ok = gSpecialItems[e->unkA] != 0;
        break;
    case 5:
        ok = !(compute_clamped_count_difference() < e->unkA);
        break;
    case 6:
        ok = gBolts >= vendor_item_prices[e->unkA].unk14 && compute_clamped_count_difference() >= 4;
        break;
    default:
        ok = 0;
        break;
    }
    if (ok) {
        next = e->unkC;
    } else {
        next = e->unkE;
    }
    if (next != menu->unk36) {
        menu->unk36 = next;
        menu->unk38 = D_0015F6F0_far;
        e = (MenuItem *)(menu->unk3C + next * 0x1C);
        menu->unk8 = *(u8 *)&e->unk10 & 1;
        process_menu_entry(arg0, menu, 0);
    }
}

extern char D_0013CA40[];

void pad_resetState(void) {
    char *p = D_0013CA40;
    *(short *)(p + 0x18E) = 0;
    *(int *)(p + 0x190) = 0;
}

LINKER_REMNANT("asm/remnants/text", func_00217ED8);

extern int D_001CDAE0 NOT_SDA;
extern void func_00124650(void);
extern void func_00124B88(int);
extern int func_00124BC8(void *, void *);

/* Minimal layouts for init_pads: the port/slot socket parameter (NTSC name unknown)
   and the leading part of PadState from pad.h, up to the fields init_pads writes. */
typedef struct {
    s32 port;
    s32 slot;
} PadSocketParam;

void init_pads(void) {
    PadSocketParam *p = (PadSocketParam *)&D_001CDAE0;

    func_00124650();
    func_00124B88(0);
    D_001CDAE0 = 2;
    p->slot = 0;
    ((struct PadState *)D_0013CA40)->socket = func_00124BC8(p, D_0013CA40);
    D_001CDAE0 = 2;
    p->slot = 1;
    ((struct PadState *)D_0013CA40)->profile_state = 0;
    ((struct PadState *)D_0013CA40)->device_state = 0;
}
