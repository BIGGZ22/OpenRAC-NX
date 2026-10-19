#include "common.h"
#include "callees.h"
#include "records.h"
#include "structs.h"

/*
 * skyfunc.cpp in the original source; text 0x22BEB0-0x22CEB8.
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
extern int D_0015EF90;
extern char D_001D4B90[];
extern char D_001D4BC0[];
extern char D_001D5F70[] NOT_SDA;
extern char D_001D603B[];
extern int D_001A0414;
extern int D_001CFBF4;
extern int D_001CFAD8;
extern void func_0020C7A0(void *);
extern int func_0020CA50(void *, void *, void *, int);
extern int D_00141FA0[];
extern char D_001D0A50[];
extern char D_001D0A88[];
extern int D_001A0418 NOT_SDA;
extern float func_001FA748(float, float);
extern char *D_001D5F74 NOT_SDA;
extern void func_0020E180(int, int);
extern char D_00187040[];
extern void func_00220128(void *);
extern void *func_00226720_a(int) __asm__("func_00226720");
extern int func_002267C0(int);
extern void func_00234C98(int, int);
extern void func_00205E70(void);
extern void func_001F4630(int);
extern void func_001F4748(void);
extern void func_001F68E8_c(int, int, long, void *, int)
    __asm__("func_001F68E8");
extern void *func_001FE540_id(int) __asm__("func_001FE540");
extern short D_001602B0;
extern void func_00201640(int, int, int, int, long, long);
extern int func_00200198(int, int);
extern void func_00200468(int, int, int, int, int, int);
extern void func_001F5800(int, int, int, int, int, int, int, int, long,
                          long);
extern short D_00151880[];
extern long D_001A0448;
extern int func_00226EA8(int);
extern int func_00226F68(int);
extern int D_0013CC04 NOT_SDA;
extern char D_001D2678[];
extern char *D_001D5F78 NOT_SDA;
extern unsigned char D_001B3E40[] NOT_SDA;
extern void *func_0020D348(void);
extern void func_0020E340(void *, int, int, int, int);
typedef struct {
    int key;
    int flags;
} PadBind;
extern PadBind D_001D6448_t[] __asm__("D_001D6448");
extern int func_00227018(int handle);
extern int D_001D6448[];
extern char D_001D5D58[] NOT_SDA;
extern char *D_001B3580[] NOT_SDA;
extern int D_001D6860[];
extern int D_001D74C0[];
extern int D_001D6760[];

LINKER_REMNANT("asm/remnants/text", func_0022BEB0);

/* Sky shell draw loop (a variant of func_0022C5A0): shell 0 re-reads the
   base transform; shells 1-5 scroll their float by D_00160500 times a
   per-shell rate, get a per-shell spin and scale, rebuild the sky transform
   from D_001D9A70, scale its three rows, copy D_00160560 into the
   translation, and every shell is drawn by func_0022C9A8. Each case does
   its own add and store, and the shell table is indexed through its own
   pointer, which gives retail's base-first address add. */
extern char *D_0016055C_2BEB0 __asm__("D_0016055C") MACRO_ADDR;
extern f32 D_00160500_2BEB0 SDATA(D_00160500);
extern u8 D_001D9A70_2BEB0[] __asm__("D_001D9A70");
extern u8 D_00160560_2BEB0[] __asm__("D_00160560") MACRO_ADDR;
extern void func_001FA190_2BEB0(void *) __asm__("func_001FA190");
extern void func_001F9BC0_2BEB0(void *) __asm__("func_001F9BC0");
extern void func_001FA238_2BEB0(void *, void *) __asm__("func_001FA238");
extern void func_001F9C48_2BEB0(void *, void *, f32) __asm__("func_001F9C48");
extern void func_0022C9A8_2BEB0(s32) __asm__("func_0022C9A8");

void DrawSkyShellsScrolling(void) {
    f32 rot[4];
    f32 scale;
    f32 *p;
    f32 d;
    u8 *t;
    u8 **tab;
    s32 i;

    *(s16 *)(D_0016055C_2BEB0 + 4) = 0;
    func_001FA190_2BEB0(D_001D9A70_2BEB0);
    func_001F9BC0_2BEB0(rot);
    for (i = 0; i < *(s16 *)(D_0016055C_2BEB0 + 6); i++) {
        tab = (u8 **)(D_0016055C_2BEB0 + 0x20);
        p = (f32 *)(tab[i] + 8);
        scale = 1.0f;
        switch (i) {
        case 1:
            rot[1] = 0.02f;
            scale = 3.0f;
            *p += D_00160500_2BEB0 * 0.55f;
            break;
        case 2:
            rot[1] = -0.02f;
            scale = 2.5f;
            *p += D_00160500_2BEB0 * 0.6f;
            break;
        case 3:
            rot[1] = 0.01f;
            scale = 2.0f;
            *p += D_00160500_2BEB0 * 0.7f;
            break;
        case 4:
            rot[1] = -0.01f;
            scale = 1.5f;
            *p += D_00160500_2BEB0 * 0.75f;
            break;
        case 5:
            scale = 1.25f;
            *p += D_00160500_2BEB0 * 0.8f;
            break;
        default:
            func_001FA190_2BEB0(D_001D9A70_2BEB0);
            break;
        }
        if (i > 0) {
            f32 v = (f32)i * 2000.0f;
            s32 n = (s32)(*p + v);
            rot[2] = (f32)(n % 10000) * 0.00062831853f - 3.1415927f;
            func_001FA238_2BEB0(D_001D9A70_2BEB0, rot);
            t = D_001D9A70_2BEB0;
            func_001F9C48_2BEB0(t, t, scale);
            func_001F9C48_2BEB0(t + 0x10, t + 0x10, scale);
            func_001F9C48_2BEB0(t + 0x20, t + 0x20, scale);
            qcopy(t + 0x30, D_00160560_2BEB0);
        }
        func_0022C9A8_2BEB0(i);
    }
}

struct LevelSkyEffectData {
    u8 pad_0[4];
    s16 relocation_state;
    u8 pad_6[2];
    s16 effect_count;
    u8 pad_A[0x12];
    struct SkyEffect *effects;
};
union SkyEffectColorOrAngles {
    struct SkyAngles {
        s16 azimuth;
        u16 elevation;
    } angles;
    u32 base_color;
};
struct SkyEffect {
    u8 randomize_color;
    u8 pad1[1];
    u8 texture_index;
    u8 flags;
    u32 color;
    f32 angle;
    union SkyEffectColorOrAngles state;
    f32 position_x;
    f32 position_y;
    f32 position_z;
    f32 size;
};
extern struct LevelSkyEffectData * D_0016055C_2C188 __asm__("D_0016055C") MACRO_ADDR;
extern u8 D_001D9A70[];
extern void func_001160C8(s32);
extern f32 func_001F9B88(f32);
extern f32 func_001F9F90(f32);
extern f32 func_001F9FA8(f32);
extern void func_001FA190(u8 *);
extern f32 func_001FA748(f32, f32);
extern f32 func_001FA888(s32);
extern s32 func_002140B0(s32);
extern f32 func_00214158();
extern void func_0022C9A8(s32);
extern void func_0022CEB8();
extern void func_00234C98_2C188(s32, u64) __asm__("func_00234C98");
extern s32 func_001160D8();

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/rendering/sky/update_sky_effects.c, update_sky_effects. */
void SkyLevelGeneric___maybe(void) {
    f32 trig_product;
    f32 elevation;
    f32 azimuth;
    s32 color_delta;
    u32 base_color;
    s32 initialization_index;
    s32 effect_index;
    s32 alpha_delta;
    s32 random_bits;
    s32 random_color_enabled;
    s32 effect_flags;
    f32 radius;
    struct SkyEffect *effect;
    struct SkyAngles *orbit;
    s16 *angles;

    D_0016055C_2C188->relocation_state = 0;
    func_001FA190(D_001D9A70);
    SkyDrawShell(0);
    SkyDrawShell(1);
    if (D_0016055C_2C188->effect_count == 0) {
        D_0016055C_2C188->effect_count = 0x100;
        func_001160C8(0x3039);
        for (initialization_index = 0; initialization_index < D_0016055C_2C188->effect_count;
             initialization_index++) {
            radius = 50.0f;
            random_color_enabled = 1;
            effect_flags = 0x48;
            base_color = 0x30505050;
            effect = &D_0016055C_2C188->effects[initialization_index];
            if (initialization_index >= 0xF6) {
                effect->randomize_color = 0;
                angles = &effect->state.angles.azimuth;
                effect->state.angles.azimuth = (s16)(func_001160D8() >> 0x10);
                angles[1] = (s16)(func_001160D8() >> 0x10);
                /* Retail stores the float bits 0x3E23D70A. */
                effect->size = 0.16f;
                effect->flags = effect_flags;
                effect->texture_index = random_color_enabled;
            } else {
                effect->randomize_color = random_color_enabled;
                color_delta = random_integer_below(0x100);
                effect->texture_index = random_color_enabled;
                effect->flags = effect_flags;
                effect->state.angles.azimuth = color_delta;
                effect->angle = random_angle_radians();
                effect->size =
                    IntToFloat(random_integer_below(0x18) + 0x20) * 0.00390625f;
                azimuth = FastAddRots(-3.0f, random_angle_radians() * 0.2f);
                elevation = random_angle_radians();
                elevation = elevation * 0.09f;
                elevation = elevation + 1.2f;
                trig_product = FastCos(azimuth);
                trig_product = trig_product * FastSin(elevation);
                trig_product = trig_product * radius;
                effect->position_x = trig_product;
                trig_product = FastSin(azimuth);
                trig_product = trig_product * FastSin(elevation);
                trig_product = trig_product * radius;
                effect->position_y = trig_product;
                effect->position_z = FastCos(elevation) * radius;
                color_delta = random_integer_below(0x18);
                alpha_delta = random_integer_below(0x20) << 0x18;
                if ((func_001160D8() >> 0x10) & 1) {
                    effect->state.base_color = alpha_delta + ((color_delta << 0x10) + base_color);
                } else {
                    effect->state.base_color =
                        (alpha_delta + ((color_delta << 8) + base_color)) | color_delta;
                }
            }
        }
    }
    for (effect_index = 0; effect_index < D_0016055C_2C188->effect_count; effect_index++) {
        effect = &D_0016055C_2C188->effects[effect_index];
        if (effect->randomize_color == 0) {
            orbit = &effect->state.angles;
            angles = &effect->state.angles.azimuth;
            effect->state.angles.azimuth = (s16)(effect->state.angles.azimuth + 1);
            angles[1] = (u16)(angles[1] + 1);
            azimuth = IntToFloat((orbit->azimuth & 0xFFF) - 0x800) * 0.0015339808f;
            elevation = IntToFloat((angles[1] & 0xFFF) - 0x800) * 0.0015339808f;
            trig_product = FastCos(azimuth);
            trig_product = trig_product * FastSin(elevation);
            trig_product = trig_product * 50.0f;
            effect->position_x = trig_product;
            trig_product = FastSin(azimuth);
            trig_product = trig_product * FastSin(elevation);
            trig_product = trig_product * 50.0f;
            effect->position_y = trig_product;
            effect->position_z = FastAbsF(FastCos(elevation)) * 50.0f;
            if ((u32)(orbit->azimuth & 0x3F) < 8U) {
                effect->color = 0x702020F0;
            } else {
                effect->color = 0x202020F0;
            }
        } else {
            u32 color_value;
            u32 color_offset;
            random_bits = func_001160D8() >> 0x10;
            color_offset = ((random_bits & 0x1F00) << 0xA) + 0xFFDFDFE0;
            color_value = effect->state.base_color + color_offset;
            color_value += (random_bits & 0x1F0) << 6;
            color_value += (random_bits & 0x1F) << 2;
            effect->color = color_value;
        }
    }
    SkySpriteProc();
    func_00234C98_2C188(0x42, (0x8000ULL << 0x18) | 0x44);
    SkyDrawShell(2);
    SkyDrawShell(3);
}

struct SkyShellSet_2C5A0 {
    u8 pad_0[4];
    u16 relocation_state;
    s16 shell_count;
};
struct SkyTransform_2C5A0 {
    u8 pad[0x30];
    u64 translation;
};
extern f32 D_00160504_2C5A0 SDATA(D_00160504);
extern struct SkyShellSet_2C5A0 * D_0016055C_2C5A0 __asm__("D_0016055C") MACRO_ADDR;
extern u8 D_00160560_2C5A0[] __asm__("D_00160560") MACRO_ADDR;
extern struct SkyTransform_2C5A0 D_001D9A70_2C5A0 __asm__("D_001D9A70");
extern s32 D_0015EF88_2C5A0 __asm__("D_0015EF88") MACRO_ADDR;
extern void func_001F9BC0(f32 *);
extern void func_001F9C48(void *, void *, f32);
extern void func_001FA190_2C5A0(void *) __asm__("func_001FA190");
extern void func_001FA238(void *, f32 *);
extern void func_0022C7E0(void);
extern void func_0022C870(void);
extern void func_00234C98_2C5A0(s32, s64) __asm__("func_00234C98");

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/rendering/sky/draw_sky_shells.c, draw_sky_shells. */
void DrawSkyShells(void) {
    f32 rotation_angles[4];
    f32 shell_scale;
    u8 *transform_row;
    s32 shell_index;

    shell_index = 0;
    SetupSkyGifPaging();
    D_0016055C_2C5A0->relocation_state = 0;
    func_001FA190_2C5A0(&D_001D9A70_2C5A0);
    clear_u64_value(rotation_angles);
    if (D_0016055C_2C5A0->shell_count > 0) {
        do {
            shell_scale = 1.0f;
            switch (shell_index) {
            case 0:
                *(s32 *)&rotation_angles[1] = 0;
                rotation_angles[2] = D_00160504_2C5A0;
            case 1:
                *(s32 *)&rotation_angles[1] = 0;
                rotation_angles[2] = FastAddRots(D_00160504_2C5A0, rotation_angles[1]);
                shell_scale = 1.0f;
                break;
            case 2:
                rotation_angles[1] = -0.075f;
                rotation_angles[2] = FastAddRots(D_00160504_2C5A0, -0.15f);
                shell_scale = 1.25f;
                break;
            case 3:
                rotation_angles[1] = 0.05f;
                rotation_angles[2] = FastAddRots(D_00160504_2C5A0, 0.125f);
                shell_scale = 1.5f;
                break;
            case 4:
                rotation_angles[1] = 0.1f;
                rotation_angles[2] = FastAddRots(D_00160504_2C5A0, -0.05f);
                shell_scale = 1.75f;
                break;
            case 5:
                rotation_angles[1] = -0.15f;
                rotation_angles[2] = FastAddRots(D_00160504_2C5A0, 0.1f);
                shell_scale = 2.0f;
                break;
            }
            func_001FA238(&D_001D9A70_2C5A0, rotation_angles);
            transform_row = (u8 *)&D_001D9A70_2C5A0;
            func_001F9C48(transform_row, transform_row, shell_scale);
            func_001F9C48(transform_row + 0x10, transform_row + 0x10, shell_scale);
            func_001F9C48(transform_row + 0x20, transform_row + 0x20, shell_scale);
            transform_row += 0x30;
            qcopy(transform_row, D_00160560_2C5A0);
            SkyDrawShell(shell_index);
            shell_index++;
        } while (shell_index < D_0016055C_2C5A0->shell_count);
    }
    DoSkyGifPaging();
    func_00234C98_2C5A0(0x47, 0x5360B);
    func_00234C98_2C5A0(0x4E, 0x1000000 | (D_0015EF88_2C5A0 >> 13));
}

extern int D_00161000 MACRO_ADDR;
extern int D_00160570 MACRO_ADDR;
extern int D_0015EF78 MACRO_ADDR;
extern int D_0015EF74 MACRO_ADDR;
extern int D_0015F558 MACRO_ADDR;
extern char D_00160550[];
extern void func_001F2560(void *, int);
typedef struct {
    long tag;
    long unk_08;
} SkyGifPage;
typedef struct {
    char unk_00[6];
    short count;          /* 0x6: shells */
    char unk_08[4];
    short npages;         /* 0xC: GIF pages */
    short unk_0E;
    SkyGifPage *pages;    /* 0x10 */
    char unk_14[0xC];
    void *shells[1];      /* 0x20 */
} SkyDef;
extern SkyDef *D_0016055C MACRO_ADDR;

/*
 * SetupSkyGifPaging(void): reserves 0x10 bytes of the VU1 chain at
 * D_00160570 and clears the tag of each of the sky's GIF pages. Retail
 * loads the page count before the two global stores: read as a struct
 * field through a pointer it cannot alias a scalar global (gcc's
 * fixed_scalar_and_varying_struct_p), where a cast access would keep it
 * behind them.
 */
void SetupSkyGifPaging(void) {
    int p;
    SkyDef *s;
    int i;

    p = D_00161000;
    D_00160570 = p;
    p += 0x10;
    D_00161000 = p;
    ProfileMarker_Begin(D_00160550, 1);
    s = D_0016055C;
    D_0015EF74 = D_0015EF78;
    D_0015F558 = 0;
    for (i = 0; i < s->npages; i++) {
        s->pages[i].tag = 0;
    }
}

extern int *D_00161000_p __asm__("D_00161000") MACRO_ADDR;
extern int *D_00160570_p __asm__("D_00160570") MACRO_ADDR;
extern int *D_00160574 MACRO_ADDR;
extern void func_0020C2F8(void);
extern void func_00234E80(void);

/* DoSkyGifPaging: DoGifPaging's twin (func_001F4748) for the sky. Pushes
   two 4-word GIF tags (0x20000000 in the first word) onto the D_00161000
   packet, D_00160574 marking where it started and D_00160570's tag
   pointing at the second; in between, when D_0018A3B0[1] is set,
   func_0020C2F8 and func_00234E80 add their own. Then restores
   D_0015EF74 from D_0015EF78. The first advance goes through a local
   advanced in place, which keeps the old and new pointer in one register
   as retail does. */
void DoSkyGifPaging(void) {
    int *p = D_00161000_p;

    D_00160574 = p;
    p += 4;
    D_00161000_p = p;
    D_00160570_p[0] = 0x20000000;
    D_00160570_p[1] = (int)D_00161000_p;
    D_00160570_p[2] = 0;
    D_00160570_p[3] = 0;
    if (D_0018A3B0[1] != 0) {
        func_0020C2F8();
        VU1_texFlush();
    }
    D_00161000_p[0] = 0x20000000;
    D_00161000_p[1] = (int)(D_00160570_p + 4);
    D_00161000_p[2] = 0;
    D_00161000_p[3] = 0;
    D_00161000_p += 4;
    D_00160574[0] = 0x20000000;
    D_00160574[1] = (int)D_00161000_p;
    D_00160574[2] = 0;
    D_00160574[3] = 0;
    D_0015EF74 = D_0015EF78;
}

/* Retail carries 4 bytes of inter-function padding after this endlabel. */
__asm__(".section .text\n\tnop\n");

LINKER_REMNANT("asm/remnants/text", func_0022C9A0);

extern void func_0022CA00(void *);
extern void func_0022CC40(void *);

/* SkyDrawShell(int) */
void SkyDrawShell(int idx) {
    SkyDef *def = D_0016055C;

    if (idx < def->count) {
        void *shell = def->shells[idx];
        if (*(int *)((char *)shell + 4) != 0) {
            SkyDrawShellGouraud(shell);
        } else {
            SkyDrawShellTextured(shell);
        }
    }
}

typedef u32 u128_2CA00 __attribute__((mode(TI), aligned(16)));
struct DmaTag_2CA00 {
    u32 tag;
    u32 addr;
    u32 vif0;
    u32 vif1;
};
struct GifTag_2CA00;
union PacketCursor_2CA00 {
    struct DmaTag_2CA00 *tag;
    struct GifTag_2CA00 *gif;
    s32 *words;
    u8 *bytes;
    s32 addr;
};
extern union PacketCursor_2CA00 D_00161000_2CA00 __asm__("D_00161000") MACRO_ADDR;
struct SkyTile_2CA00 {
    u128_2CA00 bounds; /* 16-byte aligned: tile fields are addressed from the tile base */
    s32 address;
    s16 unk4;
    s16 unk6;
    s16 unk8;
    s16 unkA;
    s16 unkC;
    s16 unkE;
};
struct Shell_2CA00 {
    s32 count;
    u8 pad4[0xC];
    struct SkyTile_2CA00 tiles[1];
};
extern short D_00160508;
extern s32 D_00160510_2CA00 SDATA(D_00160510);
extern u8 D_0013D260[];
extern void func_0022D7E0_2CA00(struct SkyTile_2CA00 *tiles, s32 count, u8 *visibility) __asm__("func_0022D7E0");
extern void func_0020C210_2CA00(s32 address, s32 qwc, s32 destination) __asm__("func_0020C210");
extern void func_0020C230_2CA00(void) __asm__("func_0020C230");
extern s32 func_0022D2AC_2CA00(s32, s32, s32, s32) __asm__("func_0022D2AC");
extern void func_0022D520_2CA00(s32, s32, s32, s32) __asm__("func_0022D520");

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/rendering/sky/sky_draw_shell_textured.c, sky_draw_shell_textured. */
void func_0022CA00_r(struct Shell_2CA00 *shell) __asm__("func_0022CA00");
/* SkyDrawShellTextured */
void func_0022CA00_r(struct Shell_2CA00 *shell) {
    u8 visibility[shell->count];
    s32 i;
    s32 dest;
    s32 c_dest;
    s32 a_dest;
    s32 next;

    if (shell->count == 0) {
        return;
    }

    func_0022D7E0_2CA00(shell->tiles, shell->count, visibility);

    D_00161000_2CA00.words[0] = 0x30000007;
    *(s32 *)((u32)D_00161000_2CA00.words + 4) = (s32)D_0013D260;
    *(s32 *)((u32)D_00161000_2CA00.words + 8) = 0;
    *(s32 *)((u32)D_00161000_2CA00.words + 12) = 0x50000007;
    D_00161000_2CA00.words += 4;

    D_00160510_2CA00 = 1 - D_00160510_2CA00;
    if (visibility[0] == 1) {
        func_0020C210_2CA00(shell->tiles[0].address, shell->tiles[0].unkE >> 4,
                                 (((s32 *)&D_00160508))[D_00160510_2CA00]);
    }

    for (i = 0; i < shell->count; i++) {
        if (visibility[i] == 1) {
            func_0020C230_2CA00();
        }

        next = i + 1;
        D_00160510_2CA00 = 1 - D_00160510_2CA00;
        if (next < shell->count && visibility[next] == 1) {
            func_0020C210_2CA00(shell->tiles[next].address, shell->tiles[next].unkE >> 4,
                                     (((s32 *)&D_00160508))[D_00160510_2CA00]);
        }

        if (visibility[i] == 1) {
            dest = (((s32 *)&D_00160508))[1 - D_00160510_2CA00];
            c_dest = dest + shell->tiles[i].unkC;
            a_dest = dest + shell->tiles[i].unkA;
            if (func_0022D2AC_2CA00(dest + shell->tiles[i].unk8, 0x70002000, shell->tiles[i].unk4,
                             (&shell->tiles[i])->unkC) == 0) {
                func_0022D520_2CA00(shell->tiles[i].unk6, c_dest, a_dest, 0x70002000);
            }
        }
    }
}

typedef u32 u128_2CC40 __attribute__((mode(TI), aligned(16)));
struct DmaTag_2CC40 {
    u32 tag;
    u32 addr;
    u32 vif0;
    u32 vif1;
};
struct GifTag_2CC40;
union PacketCursor_2CC40 {
    struct DmaTag_2CC40 *tag;
    struct GifTag_2CC40 *gif;
    s32 *words;
    u8 *bytes;
    s32 addr;
};
extern union PacketCursor_2CC40 D_00161000_2CC40 __asm__("D_00161000") MACRO_ADDR;
struct SkyTile_2CC40 {
    u128_2CC40 bounds; /* 16-byte aligned: tile fields are addressed from the tile base */
    s32 address;
    s16 unk4;
    s16 unk6;
    s16 unk8;
    s16 unkA;
    s16 unkC;
    s16 unkE;
};
struct Shell_2CC40 {
    s32 count;
    u8 pad4[0xC];
    struct SkyTile_2CC40 tiles[1];
};
extern s32 D_00160510_2CC40 SDATA(D_00160510);
extern u8 D_0013D1F0[];
extern void func_0022D7E0_2CC40(struct SkyTile_2CC40 *tiles, s32 count, u8 *visibility) __asm__("func_0022D7E0");
extern void func_0020C210_2CC40(s32 address, s32 qwc, s32 destination) __asm__("func_0020C210");
extern void func_0020C230_2CC40(void) __asm__("func_0020C230");
extern s32 func_0022D2AC_2CC40(s32, s32, s32, s32) __asm__("func_0022D2AC");
extern void func_0022D3F8_2CC40(s32, s32, s32, s32) __asm__("func_0022D3F8");
extern s32 func_00234C98_2CC40(s32, s64) __asm__("func_00234C98");

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/rendering/sky/sky_draw_shell_gouraud.c, sky_draw_shell_gouraud. */
void func_0022CC40_r(struct Shell_2CC40 *shell) __asm__("func_0022CC40");
/* SkyDrawShellGouraud */
void func_0022CC40_r(struct Shell_2CC40 *shell) {
    u8 visibility[shell->count];
    s32 i;
    s32 dest;
    s32 c_dest;
    s32 a_dest;
    s32 next;

    if (shell->count == 0) {
        return;
    }

    func_0022D7E0_2CC40(shell->tiles, shell->count, visibility);

    D_00161000_2CC40.words[0] = 0x30000007;
    *(s32 *)((u32)D_00161000_2CC40.words + 4) = (s32)D_0013D1F0;
    *(s32 *)((u32)D_00161000_2CC40.words + 8) = 0;
    *(s32 *)((u32)D_00161000_2CC40.words + 12) = 0x50000007;
    D_00161000_2CC40.words += 4;

    D_00160510_2CC40 = 1 - D_00160510_2CC40;
    if (visibility[0] == 1) {
        func_0020C210_2CC40(shell->tiles[0].address, shell->tiles[0].unkE >> 4,
                                 (((s32 *)&D_00160508))[D_00160510_2CC40]);
    }

    for (i = 0; i < shell->count; i++) {
        if (visibility[i] == 1) {
            func_0020C230_2CC40();
        }

        next = i + 1;
        D_00160510_2CC40 = 1 - D_00160510_2CC40;
        if (next < shell->count && visibility[next] == 1) {
            func_0020C210_2CC40(shell->tiles[next].address, shell->tiles[next].unkE >> 4,
                                     (((s32 *)&D_00160508))[D_00160510_2CC40]);
        }

        if (visibility[i] == 1) {
            dest = (((s32 *)&D_00160508))[1 - D_00160510_2CC40];
            c_dest = dest + shell->tiles[i].unkC;
            a_dest = dest + shell->tiles[i].unkA;
            if (func_0022D2AC_2CC40(dest + shell->tiles[i].unk8, 0x70002000, shell->tiles[i].unk4,
                             (&shell->tiles[i])->unkC) == 0) {
                func_0022D3F8_2CC40(shell->tiles[i].unk6, c_dest, a_dest, 0x70002000);
            }
        }
    }

    func_00234C98_2CC40(0x47, 0x3180B);
    func_00234C98_2CC40(0x4E, (D_0015EF88_2C5A0 >> 13) | 0x1000000 | ((s64)1 << 32));
}
