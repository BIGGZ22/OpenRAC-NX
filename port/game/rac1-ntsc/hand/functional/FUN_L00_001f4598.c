/* Carried from the PAL decompilation's nonmatching/shared/func_L00_001F4918.c (games/rac1/pal) through the version
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
/* NON_MATCHING FUN_L00_001f4598 -- src/overlays/shared/draw_001F3A78.c
 * Best so far: BYTES 169/4932 (96.6% of the bytes match), checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   $s0; ctx high in $s2 shared with the x4 block, ctx in $s1; x4 block reloads x and y (no forwarding).
 *   staged (view=&ctx in both arms): ctx high in arms (wrong). q3/q4 (o=&options in arms): options high in arms
 *   (right) but y forwarded into the x4 block and ctx in $s0. q5/q6 (both pointers in arms): both highs in arms, +
 *   3. case 10: retail tests the loaded byte in $v0 and builds the value in $a1 (bnez; daddu $a1,0 in slot;
 *   addiu $a1,0xFF). Every form with a separate test pseudo becomes movz (c10a..e, n, p); forms that keep a branch
 *   test the value's own pseudo (lbu $a1) (q2, c10i..m) or do not fold (~*p: nor+andi, c10f/g/h/o).
 *   4. case 3: 0x18 constant and D_L00_001C40B0 address swapped ($a0/$a1) (local alloc order).
 *   5. second func_00116248 call of case 5: arg setup order (t0/t1/a0 before a1/a2 in ours).
 */
typedef struct { s32 v[6]; } __attribute__((packed)) MenuCounts_1F4918_c1f4598;
typedef struct {
    s32 f0, bits, draw_bits, group, item, camera_mode, display_mode, occl_mode;
    s32 f20, f24, f28, f2C, f30, f34, selection, mission;
    char pad40[0x44];
    float rot, angle, distance, height;
    s32 x, y, z;
    char padA0[4];
    s32 buffer_mode;
    char padA8[0x48];
    s32 fF0, fF4;
    u8 saved_timer, point_timer;
} DebugMenu_1F4918_c1f4598;
extern DebugMenu_1F4918_c1f4598 debug_menu_1F4918 __asm__("D_L00_0016C058");
typedef struct { char pad[0x1A0]; union { u64 all; struct { s32 held, pressed; } word; } keys; } Input_1F4918_c1f4598;
typedef struct { char pad[0x2460]; Input_1F4918_c1f4598 ctl; } InputBase_1F4918_c1f4598;
extern Input_1F4918_c1f4598 input_1F4918 __asm__("D_0013C940");
extern MenuCounts_1F4918_c1f4598 D_L00_001E7C70_1F4918 __asm__("D_L00_001E7920");
typedef union { float f; s32 i; } ViewField_1F4918_c1f4598;
typedef struct { char pad[0x200]; ViewField_1F4918_c1f4598 x,y,x4,y4; char pad210[0x2C]; s32 a,b,c; } Context_1F4918_c1f4598;
extern Context_1F4918_c1f4598 ctx_1F4918 __asm__("D_L00_0016CA40");
typedef struct { char pad[0x140]; float x,y,z; char pad14C[12]; float angle; } Camera_1F4918_c1f4598;
extern Camera_1F4918_c1f4598 cam_1F4918 __asm__("D_L00_00166C80");
typedef struct { char pad[0x80]; float x,y,z,w; char pad90[8]; float angle; } Hero_1F4918_c1f4598;
extern Hero_1F4918_c1f4598 hero_1F4918 __asm__("D_0013F350");
extern char weapon_order_1F4918[] __asm__("D_0013E05A");
extern char ammo_1F4918[] __asm__("D_0013D407");
extern u8 mission_1F4918[][16] __asm__("D_0014C050");
typedef struct { char pad[0x150]; s16 x,y; } Options_1F4918_c1f4598;
extern Options_1F4918_c1f4598 options_1F4918 __asm__("D_00151780");
typedef struct { char pad[14]; u16 limit; char rest[8]; } Weapon_1F4918_c1f4598;
extern Weapon_1F4918_c1f4598 D_L00_001C43B0_1F4918[] __asm__("D_L00_001C40B0");
typedef struct { char pad[62]; union { s16 word; u8 byte; } limit; char rest[12]; } Item_1F4918_c1f4598;
extern Item_1F4918_c1f4598 D_L00_00179BC0_1F4918[] __asm__("D_L00_00179AC0");
extern short weapon_ids_1F4918 SDATA(D_L00_0015EFD0);
extern char D_L00_0015F0E0_1F4918 SDATA(D_L00_0015EFE0);
extern char point_format_1F4918[] __asm__("D_L00_0015EFE0");
extern char D_L00_0015F0F0_1F4918[] __asm__("D_L00_0015EFF0") NOT_SDA;
extern char D_L00_0015F0F8_1F4918[] __asm__("D_L00_0015EFF8");
extern char D_L00_001E7C88_1F4918[] __asm__("D_L00_001E7938") MACRO_ADDR;
extern char saved_prefix_1F4918[] __asm__("D_L00_001E7938");
extern char D_L00_001E7958[], D_L00_001E7970[], D_L00_001E7CE0_1F4918[] __asm__("D_L00_001E7990");
extern u8 D_L00_0015FD48_1F4918[] __asm__("D_L00_0015FC88");
extern s32 D_0015EE84_1F4918 __asm__("D_0015ED84") MACRO_ADDR;
extern s32 mission_level_1F4918 __asm__("D_0015ED84") NOT_SDA;
extern s32 D_0015EE98_1F4918 __asm__("D_0015ED98") MACRO_ADDR;
extern s32 D_L00_0015F6F4_1F4918 __asm__("D_L00_0015F614") NOT_SDA;
extern s32 count_store_1F4918 __asm__("D_L00_0015F614") MACRO_ADDR;
extern float *D_L00_00173F20_1F4918 __asm__("D_L00_00173E20") NOT_SDA;
extern float D_L00_00161040_1F4918 __asm__("D_L00_00160F80") MACRO_ADDR;
extern float D_L00_001610A0_1F4918 __asm__("D_L00_00160FE0") MACRO_ADDR;
extern float D_L00_00160564_1F4918 __asm__("D_L00_001604A4") MACRO_ADDR;
extern s32 D_L00_001600B0_1F4918 __asm__("D_L00_0015FFF0") MACRO_ADDR;
extern s32 D_L00_0016023C_1F4918 __asm__("D_L00_0016017C") MACRO_ADDR;
extern float D_L00_0015F548_1F4918 __asm__("D_L00_0015F448") MACRO_ADDR;
extern float D_L00_0015F54C_1F4918 __asm__("D_L00_0015F44C") MACRO_ADDR;
extern float D_L00_0015F550_1F4918 __asm__("D_L00_0015F450") MACRO_ADDR;
extern float D_L00_0015F554_1F4918 __asm__("D_L00_0015F454") MACRO_ADDR;
extern s32 D_L00_0015F544_1F4918 __asm__("D_L00_0015F444") MACRO_ADDR;
extern s32 D_L00_0015F545_1F4918 __asm__("D_L00_0015F445") MACRO_ADDR;
extern s32 D_L00_0015F546_1F4918 __asm__("D_L00_0015F446") MACRO_ADDR;
extern char *D_L00_0015F6E0_1F4918 __asm__("D_L00_0015F600") MACRO_ADDR;
extern s32 D_L00_0015F6A8_1F4918 __asm__("D_L00_0015F5C4") MACRO_ADDR;
extern s32 func_0022DD68_1F4918(void) __asm__("func_0022CA50");
extern void func_001FB448_1F4918(s32, s32, s32) __asm__("func_001FB280");
extern void func_001F9BF0_1F4918(void *, void *, void *) __asm__("func_001F9A28");
extern float func_001F9CE8_1F4918(void *) __asm__("func_001F9B20");
extern float func_L00_001FF860_1F4918(float, float) __asm__("func_L00_001FF8B0");
extern float func_001FA790_1F4918(float, float) __asm__("func_001FA5C8");
extern float func_001FA888_1F4918(s32) __asm__("func_001FA6C0");
extern void func_001F3140_1F4918(void) __asm__("func_001F2D98");
extern void draw_context_1F4918(void) __asm__("func_001F2588");
extern s32 func_00116248_1F4918(char *, const char *, ...) __asm__("func_00116248");
extern double promote_float_1F4918(float) __asm__("func_00120478");
extern s32 func_0011BF80_1F4918(char *, s32, ...) __asm__("func_0011BC80");
extern s32 func_0011C388_1F4918(s32, s32, s32) __asm__("func_0011C088");
extern s32 string_length_1F4918(char *) __asm__("func_001166CC");
extern s32 func_0011C820_1F4918(s32, void *, s32) __asm__("func_0011C520");
extern s32 func_0011C208_1F4918(s32) __asm__("func_0011BF08");
extern s32 func_0011C5C0_1F4918(s32, void *, s32) __asm__("func_0011C2C0");

// Handles rendering debug menu navigation, display settings and point files.
void FUN_L00_001f4598(void) {
    MenuCounts_1F4918_c1f4598 counts;
    union {
        struct { float vec[4]; char path[64], text[32]; } points;
        struct { char path[64], text[64]; } saved;
    } buf;
    s32 buttons;
    func_0022DD68_1F4918();
    buttons = input_1F4918.keys.word.pressed;
    if (buttons & 0x500) {
        D_L00_0015F6A8_1F4918 = debug_menu_1F4918.buffer_mode;
        func_001FB448_1F4918(ctx_1F4918.a, ctx_1F4918.b, ctx_1F4918.c);
        debug_menu_1F4918.saved_timer = 110;
        debug_menu_1F4918.point_timer = 110;
        debug_menu_1F4918.x = (s32)cam_1F4918.x >> 2;
        debug_menu_1F4918.y = (s32)cam_1F4918.y >> 2;
        debug_menu_1F4918.z = (s32)cam_1F4918.z >> 2;
        return;
    }
    counts = D_L00_001E7C70_1F4918;
    if (buttons & 0x2000) {
        if (++debug_menu_1F4918.group == 3) debug_menu_1F4918.group = 0;
        else if (debug_menu_1F4918.group == 6) debug_menu_1F4918.group = 3;
    } else if (buttons & 0x8000) {
        if (--debug_menu_1F4918.group < 0) debug_menu_1F4918.group = 2;
        else if (debug_menu_1F4918.group == 2) debug_menu_1F4918.group = 5;
    }
    if ((input_1F4918.keys.word.pressed & 0x1000) && --debug_menu_1F4918.item < 0) {
        if (debug_menu_1F4918.group >= 3) debug_menu_1F4918.group = (debug_menu_1F4918.group + 3) % 6;
        debug_menu_1F4918.item = counts.v[debug_menu_1F4918.group] - 1;
    } else if (input_1F4918.keys.word.pressed & 0x4000) {
        if (++debug_menu_1F4918.item >= counts.v[debug_menu_1F4918.group]) {
            if (debug_menu_1F4918.group < 3) debug_menu_1F4918.group = (debug_menu_1F4918.group + 9) % 6;
            debug_menu_1F4918.item = 0;
        }
    }
    if (debug_menu_1F4918.item >= counts.v[debug_menu_1F4918.group]) debug_menu_1F4918.item = counts.v[debug_menu_1F4918.group] - 1;
    switch (debug_menu_1F4918.group) {
    case 0:
        if (input_1F4918.keys.word.pressed & 0x40) {
            s32 bit = 1 << debug_menu_1F4918.item;
            debug_menu_1F4918.draw_bits ^= bit;
            if (bit == 16) {
                if (debug_menu_1F4918.draw_bits & 16) debug_menu_1F4918.draw_bits = bit;
                else debug_menu_1F4918.draw_bits = 15;
            }
        }
        break;
    case 1:
        switch (debug_menu_1F4918.item) {
        case 0:
            if (input_1F4918.keys.word.pressed & 0x50) {
                if (input_1F4918.keys.word.pressed & 0x40) {
                    if (++debug_menu_1F4918.camera_mode >= 4) debug_menu_1F4918.camera_mode = 0;
                }
                if (input_1F4918.keys.word.pressed & 0x10) {
                    if (--debug_menu_1F4918.camera_mode < 0) debug_menu_1F4918.camera_mode = 3;
                }
                switch (debug_menu_1F4918.camera_mode) {
                case 0: debug_menu_1F4918.draw_bits = 15; break;
                case 1: debug_menu_1F4918.draw_bits = 0; break;
                case 2:
                    debug_menu_1F4918.draw_bits = 0;
                    func_001F9BF0_1F4918(buf.points.vec, &hero_1F4918.x, &cam_1F4918.x);
                    debug_menu_1F4918.distance = func_001F9CE8_1F4918(buf.points.vec);
                    debug_menu_1F4918.angle = func_001FA790_1F4918(func_L00_001FF860_1F4918(buf.points.vec[0],buf.points.vec[1]), cam_1F4918.angle);
                    debug_menu_1F4918.height = hero_1F4918.z - cam_1F4918.z;
                    debug_menu_1F4918.rot = func_001FA790_1F4918(hero_1F4918.angle, cam_1F4918.angle);
                    break;
                case 3: debug_menu_1F4918.draw_bits = 6; break;
                }
            }
            break;
        case 1:
            if (input_1F4918.keys.word.pressed & 0x40) { if (++debug_menu_1F4918.display_mode >= 8) debug_menu_1F4918.display_mode = 0; }
            if (input_1F4918.keys.word.pressed & 0x10) { if (--debug_menu_1F4918.display_mode < 0) debug_menu_1F4918.display_mode = 7; }
            break;
        case 2:
            if (!D_L00_0015F6E0_1F4918) { debug_menu_1F4918.occl_mode = 0; break; }
            if (input_1F4918.keys.word.pressed & 0x40) { if (++debug_menu_1F4918.occl_mode >= 3) debug_menu_1F4918.occl_mode = 0; }
            if (input_1F4918.keys.word.pressed & 0x10) { if (--debug_menu_1F4918.occl_mode < 0) debug_menu_1F4918.occl_mode = 2; }
            break;
        case 3:
            if (input_1F4918.keys.word.pressed & 0x50) debug_menu_1F4918.f20 = !debug_menu_1F4918.f20;
            break;
        case 4:
            if (input_1F4918.keys.word.pressed & 0x50) debug_menu_1F4918.f24 = !debug_menu_1F4918.f24;
            break;
        case 5:
            if (input_1F4918.keys.word.pressed & 0x50) {
                debug_menu_1F4918.f28 = !debug_menu_1F4918.f28;
                if (debug_menu_1F4918.f28) {
                    D_L00_00161040_1F4918=65536.0f; D_L00_001610A0_1F4918=64.0f; D_L00_00160564_1F4918=40.0f; D_L00_001600B0_1F4918=64; D_L00_0016023C_1F4918=0x40000;
                } else {
                    D_L00_00161040_1F4918=512000.0f; D_L00_001610A0_1F4918=720.0f; D_L00_00160564_1F4918=500.0f; D_L00_001600B0_1F4918=500; D_L00_0016023C_1F4918=0x1F4000;
                }
            }
            break;
        case 6:
            if (input_1F4918.keys.word.pressed & 0x40) { if (++debug_menu_1F4918.f2C >= 3) debug_menu_1F4918.f2C = 0; }
            if (input_1F4918.keys.word.pressed & 0x10) { if (--debug_menu_1F4918.f2C < 0) debug_menu_1F4918.f2C = 2; }
            break;
        case 7:
            if (input_1F4918.keys.word.pressed & 0x50) debug_menu_1F4918.f30 = !debug_menu_1F4918.f30;
            break;
        case 8:
            if (input_1F4918.keys.word.pressed & 0x50) {
                float scale;
                Context_1F4918_c1f4598 *view;
                debug_menu_1F4918.f34 = !debug_menu_1F4918.f34;
                if (debug_menu_1F4918.f34) { scale=0.125f; view=&ctx_1F4918; }
                else { scale=0.5f; view=&ctx_1F4918; }
                view->x.f = func_001FA888_1F4918(options_1F4918.x) * scale;
                view->y.f = func_001FA888_1F4918(options_1F4918.y) * scale;
                {
                    float x=ctx_1F4918.x.f;
                    float y=ctx_1F4918.y.f;
                    ctx_1F4918.x4.f=x*4.0f;
                    ctx_1F4918.y4.f=y*4.0f;
                }
                func_001F3140_1F4918();
            }
            break;
        case 9:
            if (input_1F4918.keys.word.pressed & 0x50) {
                if (input_1F4918.keys.word.pressed & 0x40) { if (++debug_menu_1F4918.selection == 36) debug_menu_1F4918.selection = -1; }
                else { if (--debug_menu_1F4918.selection < -1) debug_menu_1F4918.selection = 35; }
            }
            if (debug_menu_1F4918.selection >= 0) debug_menu_1F4918.buffer_mode=2; else debug_menu_1F4918.buffer_mode=0;
            break;
        case 10:
            if (input_1F4918.keys.word.pressed & 0x50) {
                if (input_1F4918.keys.word.pressed & 0x40) { if (++debug_menu_1F4918.mission == 16) debug_menu_1F4918.mission = 0; }
                else { if (--debug_menu_1F4918.mission < 0) debug_menu_1F4918.mission = 15; }
            } else if (input_1F4918.keys.word.pressed & 0x20) {
                s32 slot = debug_menu_1F4918.mission;
                s32 offset = slot + (D_0015EE84_1F4918 << 4);
                u8 *p = (u8 *)mission_1F4918 + offset;
                u8 value=*p;
                if (value == 0) value=~value;
                else value=0;
                *p=value;
                D_L00_0015FD48_1F4918[slot]=value;
            }
            break;
        }
        break;
    case 2:
        if (input_1F4918.keys.word.pressed & 0x40) debug_menu_1F4918.bits ^= 1 << debug_menu_1F4918.item;
        break;
    case 3: {
        s32 *value;
        s32 limit = 999999;
        if (debug_menu_1F4918.item < 13) {
            s32 id=((u8 *)&weapon_ids_1F4918)[debug_menu_1F4918.item];
            value=(s32 *)(ammo_1F4918 + 0x21) + id;
            limit=D_L00_001C43B0_1F4918[id].limit;
        } else value=&D_0015EE98_1F4918;
        if (input_1F4918.keys.word.held & 8) {
            if (input_1F4918.keys.word.pressed & 0x40) *value += 60;
            if (input_1F4918.keys.word.pressed & 0x20) { if (*value > 60) *value-=60; else *value=0; }
        } else if (input_1F4918.keys.word.held & 2) {
            if (input_1F4918.keys.word.pressed & 0x40) *value += 600;
            if (input_1F4918.keys.word.pressed & 0x20) { if (*value > 600) *value-=600; else *value=0; }
        } else {
            if (input_1F4918.keys.word.pressed & 0x40) *value += 1;
            if (input_1F4918.keys.word.pressed & 0x20) { if (*value > 0) *value-=1; }
        }
        if (*value > limit) *value = limit;
        if (input_1F4918.keys.word.pressed & 0x10) ((u8 *)weapon_order_1F4918 + 0x4C6)[((u8 *)&weapon_ids_1F4918)[debug_menu_1F4918.item]]++;
        if (input_1F4918.keys.word.pressed & 0x80) ((u8 *)weapon_order_1F4918 + 0x4C6)[((u8 *)&weapon_ids_1F4918)[debug_menu_1F4918.item]]--;
        {
            s32 id=((u8 *)&weapon_ids_1F4918)[debug_menu_1F4918.item];
            Item_1F4918_c1f4598 *record=D_L00_00179BC0_1F4918+id;
            u8 *p=(u8 *)weapon_order_1F4918 + 0x4C6 + id;
            if (*p > record->limit.word) *p=record->limit.byte;
        }
        break;
    }
    case 4:
        if (debug_menu_1F4918.item == 0) {
            u64 keys=input_1F4918.keys.all;
            if (keys & 0x800000040ULL) D_L00_0015F548_1F4918 += 1024.0f;
            if (keys & 0x400000010ULL) D_L00_0015F548_1F4918 -= 1024.0f;
            if (D_L00_0015F548_1F4918 > D_L00_0015F54C_1F4918) D_L00_0015F548_1F4918 = D_L00_0015F54C_1F4918;
            else if (D_L00_0015F548_1F4918 < 0.0f) D_L00_0015F548_1F4918=0.0f;
        }
        if (debug_menu_1F4918.item == 2) {
            u64 keys=input_1F4918.keys.all;
            if (keys & 0x800000040ULL) D_L00_0015F54C_1F4918 += 1024.0f;
            if (keys & 0x400000010ULL) D_L00_0015F54C_1F4918 -= 1024.0f;
            if (D_L00_0015F54C_1F4918 > 524288.0f) D_L00_0015F54C_1F4918 = 524288.0f;
            else if (D_L00_0015F54C_1F4918 < D_L00_0015F548_1F4918) D_L00_0015F54C_1F4918=D_L00_0015F548_1F4918;
        }
        if (debug_menu_1F4918.item == 1) {
            u64 keys=input_1F4918.keys.all;
            if (keys & 0x800000040ULL) D_L00_0015F550_1F4918 -= 2.55f;
            if (keys & 0x400000010ULL) D_L00_0015F550_1F4918 += 2.55f;
            if (D_L00_0015F550_1F4918 > 255.0f) D_L00_0015F550_1F4918 = 255.0f;
            else if (D_L00_0015F550_1F4918 < 0.0f) D_L00_0015F550_1F4918=0.0f;
        }
        if (debug_menu_1F4918.item == 3) {
            u64 keys=input_1F4918.keys.all;
            if (keys & 0x800000040ULL) D_L00_0015F554_1F4918 -= 2.55f;
            if (keys & 0x400000010ULL) D_L00_0015F554_1F4918 += 2.55f;
            if (D_L00_0015F554_1F4918 > 255.0f) D_L00_0015F554_1F4918 = 255.0f;
            else if (D_L00_0015F554_1F4918 < 0.0f) D_L00_0015F554_1F4918=0.0f;
        }
        if (debug_menu_1F4918.item == 4) {
            if ((*(u8 *)&D_L00_0015F544_1F4918) < 255 && (input_1F4918.keys.all&0x800000040ULL)) ++(*(u8 *)&D_L00_0015F544_1F4918);
            if ((*(u8 *)&D_L00_0015F544_1F4918) > 0 && (input_1F4918.keys.all&0x400000010ULL)) --(*(u8 *)&D_L00_0015F544_1F4918);
        }
        if (debug_menu_1F4918.item == 5) {
            if ((*(u8 *)&D_L00_0015F545_1F4918) < 255 && (input_1F4918.keys.all&0x800000040ULL)) ++(*(u8 *)&D_L00_0015F545_1F4918);
            if ((*(u8 *)&D_L00_0015F545_1F4918) > 0 && (input_1F4918.keys.all&0x400000010ULL)) --(*(u8 *)&D_L00_0015F545_1F4918);
        }
        if (debug_menu_1F4918.item == 6) {
            if ((*(u8 *)&D_L00_0015F546_1F4918) < 255 && (input_1F4918.keys.all&0x800000040ULL)) ++(*(u8 *)&D_L00_0015F546_1F4918);
            if ((*(u8 *)&D_L00_0015F546_1F4918) > 0 && (input_1F4918.keys.all&0x400000010ULL)) --(*(u8 *)&D_L00_0015F546_1F4918);
        }
        draw_context_1F4918();
        break;
    case 5:
        if (debug_menu_1F4918.item == 0 && (input_1F4918.keys.word.pressed&0x50)) debug_menu_1F4918.fF0=!debug_menu_1F4918.fF0;
        if (debug_menu_1F4918.item == 1 && (input_1F4918.keys.word.pressed&0x50)) debug_menu_1F4918.fF4=!debug_menu_1F4918.fF4;
        if (debug_menu_1F4918.item == 2 && (input_1F4918.keys.word.pressed&0xF0)) {
            s32 i=0, found, fd, size;
            char *extension;
            {
            float *p; s32 count=count_store_1F4918;
            found=0;
            if (count > 0) {
            p=D_L00_00173F20_1F4918;
            do {
                if (p[0] < cam_1F4918.x+0.5f && p[0] > cam_1F4918.x-0.5f &&
                    p[1] < cam_1F4918.y+0.5f && p[1] > cam_1F4918.y-0.5f &&
                    p[2] < cam_1F4918.z+0.5f && p[2] > cam_1F4918.z-0.5f) { found=1; break; }
                p++;
            } while (++i < count);
            }
            }
            if (found) break;
            extension=D_L00_0015F0F0_1F4918;
            func_00116248_1F4918(buf.points.path,&D_L00_0015F0E0_1F4918,D_L00_001E7C88_1F4918,D_0015EE84_1F4918,D_L00_001E7958,extension);
            func_00116248_1F4918(buf.points.text,D_L00_001E7970,promote_float_1F4918(cam_1F4918.x),promote_float_1F4918(cam_1F4918.y),promote_float_1F4918(cam_1F4918.z));
            fd=func_0011BF80_1F4918(buf.points.path,0x303);
            if (fd >= 0) {
                func_0011C388_1F4918(fd,0,2);
                func_0011C820_1F4918(fd,buf.points.text,string_length_1F4918(buf.points.text));
                func_0011C208_1F4918(fd);
                debug_menu_1F4918.point_timer=121;
            }
            {
            float *p=D_L00_00173F20_1F4918;
            func_00116248_1F4918(buf.points.path,&D_L00_0015F0E0_1F4918,D_L00_001E7C88_1F4918,D_0015EE84_1F4918,D_L00_0015F0F8_1F4918,extension);
            fd=func_0011BF80_1F4918(buf.points.path,0x303);
            if (fd >= 0) {
                func_0011C388_1F4918(fd,0,2);
                func_0011C820_1F4918(fd,&cam_1F4918.x,4);
                func_0011C820_1F4918(fd,&cam_1F4918.y,4);
                func_0011C820_1F4918(fd,&cam_1F4918.z,4);
                func_0011C388_1F4918(fd,0,0);
                size=func_0011C388_1F4918(fd,0,2);
                if (size <= 0x100000) {
                    func_0011C388_1F4918(fd,0,0);
                    func_0011C5C0_1F4918(fd,p,size);
                    count_store_1F4918=size/12;
                } else count_store_1F4918=0;
                func_0011C208_1F4918(fd);
            }
        }
        }
        if (debug_menu_1F4918.item == 3 && (input_1F4918.keys.word.pressed&0xF0)) {
            s32 fd;
            func_00116248_1F4918(buf.saved.path,point_format_1F4918,saved_prefix_1F4918,D_0015EE84_1F4918,D_L00_001E7958,D_L00_0015F0F0_1F4918);
            func_00116248_1F4918(buf.saved.text,D_L00_001E7CE0_1F4918,promote_float_1F4918(cam_1F4918.x),promote_float_1F4918(cam_1F4918.y),promote_float_1F4918(cam_1F4918.z));
            fd=func_0011BF80_1F4918(buf.saved.path,0x303);
            if (fd >= 0) {
                func_0011C388_1F4918(fd,0,2);
                func_0011C820_1F4918(fd,buf.saved.text,string_length_1F4918(buf.saved.text));
                func_0011C208_1F4918(fd);
                debug_menu_1F4918.saved_timer=121;
            }
        }
        break;
    }
}
