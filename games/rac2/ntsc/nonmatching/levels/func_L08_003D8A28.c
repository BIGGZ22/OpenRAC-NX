/* func_L08_003D8A28 (132 o) — meilleur essai, NON MATCHÉ (vague 6, w04, 2026-10-12)
 *
 * Dernière ligne de check.py : "08_tabora: instruction at +0x68 differs"
 * 128 des 132 octets identiques. Unique écart : le store final
 *   mine  00422B?8/003D8A8C : addiu $v0,$zero,1 ; sb $v0,0xBC($s1) ; ld $s0,0x0($sp)
 *   retail                   : addiu $v0,$zero,1 ; ld $s0,0x0($sp) ; sb $v0,0xBC($s1)
 * (ordonnancement sched2 du store face au restore d'épilogue : une seule position).
 * Le corps jumeau func_L08_00422AD8 est identique à un champ près (0x1D8/0x1DC au
 * lieu de 0x484/0x488) et échoue au même endroit (+0x68).
 * À noter : la forme `if (v0 == -1) v0 = 1; else { call; v0 = 1; }` est celle qui
 * reproduit le mieux le retail (beq + addiu dans le slot de délai) ; l'échange
 * if/else a été essayé et ne change pas la position du sb.
 *
 * Contexte : placement 1 (08_tabora), à définir dans src/levels/08_tabora/L08_003D8348.c.
 */
typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef int s32;
typedef unsigned int u32;
typedef float f32;

typedef struct {
    u8 pad00[0x20];
    u8 f20;
    u8 f21;
    u8 pad22[0x24 - 0x22];
    void *f24;
    u8 pad28[0x2C - 0x28];
    f32 f2C;
    u8 pad30[0x68 - 0x30];
    void *f68;
    u8 pad6C[0x95 - 0x6C];
    u8 f95;
    u16 f96;
    u8 pad98[0xBC - 0x98];
    u8 fBC;
    u8 padBD[0x100 - 0xBD];
} Ent;

typedef struct {
    u8  pad00[0x1CC];
    s32 f1CC;
    f32 f1D0;
    f32 f1D4;
    s32 f1D8;
    s32 f1DC;
    u8  f1E0[0x254 - 0x1E0];
    s32 f254;
    u8  pad258[0x40E - 0x258];
    u16 f40E;
    u8  pad410[0x440 - 0x410];
    u8  f440[0x448 - 0x440];
    s32 f448;
    u8  f44C[0x484 - 0x44C];
    s32 f484;
    s32 f488;
    u8  f48C[8];
} Moby;

extern void func_328A38(s32, s32);

void func_L08_003D8A28(Ent *a0)
{
    Moby *s0 = a0->f68;
    s32 v0;

    if (a0->f21 != 0xFF) func_328A38(a0->f21, 1);
    if (s0->f484 != -1) func_328A38(s0->f484, 1);
    v0 = s0->f488;
    if (v0 == -1) {
        v0 = 1;
    } else {
        func_328A38(s0->f484, 1);
        v0 = 1;
    }
    a0->fBC = v0;
}
