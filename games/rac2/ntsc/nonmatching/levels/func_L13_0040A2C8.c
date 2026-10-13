/* Meilleur essai pour func_L13_0040A2C8 (232 o, 2 places) — NON MARQUE.
   Derniere ligne de check.py : "13_boldan: 228 bytes, expected 232" (1 instruction
   d'ecart, fonction 4 octets trop courte).

   Variante essayee ensuite, PIRE (220 o, "13_boldan: 220 bytes, expected 232") :
   locales declarees (t, v, q) au lieu de (q, v, t) pour que le vecteur nul tombe
   en sp+0x20, plus un TI128 local `z = 0;` avant le magasin. L'allocation de
   registres se rapproche (s3 = a0+0x10, a1 et v restent inverses : retail
   s1 = a1, s2 = v) mais la longueur chute de 8 octets de plus — la variante
   ci-dessous reste la meilleure.
   Ce qui reste a resoudre :
   - le magasin 128 bits nul : le retail garde la forme registre
     `por $v0,$zero,$zero ; sq $v0,0x20($sp)` alors que `*(TI128*)q = 0;`
     est replie par cc1 en `sq $zero,0x20($sp)` (1 instruction de moins).
     La passe rac2_fold_zero_ti_store ne replie que si le registre est mort apres
     le magasin ET si la materialisation est un (set (reg:TI) 0) a moins de 12
     instructions en arriere : il faut donc une source ou le zero reste vivant,
     ou materialise dans un autre mode. Non trouve dans le temps du lot.
   - l'ordre de declaration des locales : le retail place les trois vecteurs
     16 octets en sp+0x00 (passe a 3 appels), sp+0x10 (copie du precedent, tenu
     dans $s2) et sp+0x20 (le vecteur nul). L'ordre de declaration essaie ici
     (q, v, t) donne l'ordre inverse (q en sp+0), a inverser en (t, v, q).

   Le reste de la forme (prologue, sequence d'appels, adresses D_L13_001BFD00
   +0x18/+0x20/+0x30, ordre des magasins) est conforme au retail. */

#include "common.h"

typedef int TI128 __attribute__((mode(TI)));

extern void func_00283098(char *, char *, char *);
extern void func_00282CC0(char *, char *, char *);
extern int func_00277258(void *, int, void *, float);
extern void func_00282CF0(void *, void *, void *);
extern char D_L13_001BFD00[];

int func_L13_0040A2C8(char *a0, char *a1)
{
    float q[4];
    float v[4];
    float t[4];
    TI128 w;

    *(TI128 *)q = 0;
    q[2] = *(float *)(a1 + 0) + *(float *)(a1 + 4);
    qcopy(v, q);
    func_00283098((char *)v, (char *)v, a0 + 0xC0);
    func_00282CC0((char *)t, a0 + 0x10, (char *)v);
    if (func_00277258((char *)t, *(int *)(a1 + 0x38) & 2, a0, *(float *)(a1 + 0)) == 0) {
        return 0;
    }
    w = *(TI128 *)(D_L13_001BFD00 + 0x20);
    *(int *)(a1 + 0x84) = *(int *)(D_L13_001BFD00 + 0x18);
    *(TI128 *)(a1 + 0x60) = w;
    *(TI128 *)(D_L13_001BFD00 + 0x30) = w;
    *(TI128 *)t = w;
    func_00282CF0(a0 + 0x10, (char *)t, (char *)v);
    return 1;
}
