/* func_L08_003D9208 (208 o) et func_L08_004237F0 (172 o) — meilleurs essais, NON MATCHÉS
 * (vague 6, w04, 2026-10-12)
 *
 * Dernières lignes de check.py :
 *   func_L08_003D9208 : "08_tabora: instruction at +0x4 differs"
 *   func_L08_004237F0 : "08_tabora: 176 bytes, expected 172"
 *
 * Cause identifiée, la même pour les deux : l'allocation « registre callee-saved ».
 *   retail : $s1 = la copie du paramètre (l'entrée), $s0 = le pointeur de moby (a0->f68)
 *   mon code : $s0 = le paramètre, $s1 = le pointeur de moby
 * Le corps est identique partout ailleurs (mêmes instructions, mêmes offsets) ; tout
 * l'écart découle de l'échange s0/s1 (le premier diff est au +0x4, le `sd` du
 * prologue). Les variantes essayées sans effet : déclarer la copie du paramètre en
 * premier (`Ent *a1 = a0;`), la déclarer après le pointeur de moby, utiliser le
 * paramètre directement. La même forme avait pourtant donné le bon ordre dans
 * func_L08_003D8A28 (matchée) : c'est un arbitrage de priorité d'allocno, pas une
 * règle d'ordre de déclaration.
 *
 * Autre écart réel corrigé au passage dans func_L08_004237F0 : la constante du test
 * de saturation est 0x7FFD (retail `sltiu $v0,$v1,0x7FFD`), pas 0xFFFD.
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
    u8 pad28[0x68 - 0x28];
    void *f68;
    u8 pad6C[0x95 - 0x6C];
    u8 f95;
    u16 f96;
    u8 pad98[0xBC - 0x98];
    u8 fBC;
    u8 padBD[0x100 - 0xBD];
} Ent;

typedef struct {
    u8  pad00[0x20];
    f32 f20;
    u8  pad24[0x50 - 0x24];
    u8  f50[0x88 - 0x50];
    u16 f88;
    u8  pad8A[0xE4 - 0x8A];
    f32 fE4;
    u8  padE8[0x144 - 0xE8];
    s32 f144;
    u8  pad148[0x18C - 0x148];
    u8  f18C;
    u8  pad18D[0x1B0 - 0x18D];
    f32 f1B0;
    u8  f1B8[0x1BA - 0x1B8];
    s16 f1BA;
    u8  f1BC[0x1BE - 0x1BC];
    u16 f1BE;
    u8  pad1C0[0x1CC - 0x1C0];
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

extern s32 func_2F9A60();
extern s32 func_327898(s32, s32);
extern void func_2F9A90();

/* func_L08_003D9208 : seul écart = $s0/$s1 échangés (offset 0x7FFD correct ici) */

void func_L08_003D9208(Ent *a0)
{
    Moby *s0 = a0->f68;
    u16 v1;

    if (a0->fBC != 0) {
        a0->fBC = 0;
        if (s0->f448 < 200) {
            s0->f448 = func_327898(300, 360);
            s0->f40E = 12;
        }
    }
    v1 = a0->f96;
    if (s0->f254 == 2) {
        if (func_2F9A60(&s0->f448)) s0->f40E = 3;
        v1 = a0->f96;
    }
    if (v1 <= 0xFFFDu) a0->f96 = v1 + 1;
    if (a0->f20 == 8) {
        if (a0->f95 == 0) func_2F9A60(&s0->f48C);
        func_2F9A60(&s0->f440);
    }
    func_2F9A60(&s0->f44C);
}
