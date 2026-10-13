/*
 func_L06_003E4F08 (152 o, -g8) — reste 4 octets : la constante est gardée
   dans un registre FPR callee-saved ($f20 + sauvegarde) alors que le retail
   la re-matérialise à chaque appel.

   Corps conforme mot pour mot (allocations $s1/$s0, ordre des appels, les 5
   flottants atteints par $gp avec __attribute__((sda)), le littéral
   0.000290888245f en lui+ori+mtc1) SAUF ceci :
     retail : lui/ori/mtc1 $f13 avant l'appel 1, puis lui/ori/mtc1 $f14 avant
              l'appel 2 (la constante sert deux fois dans l'appel 2) ; pas de
              sauvegarde FPR ; cadre -0x20 ; 38 mots.
     nôtre  : une seule matérialisation en $f20 conservée à travers l'appel 1
              (le CSE hisse le constant_double), + swc1/lwc1 $f20, cadre -0x30 ;
              37 mots.
   Essayé sans effet : littéral à gauche dans la multiplication, ordre des
   arguments, attribut sda. Même famille que le mur de hissage constaté sur
   func_L06_003E4FA0.

   Dernière ligne de check.py :
     func_L06_003E4F08: 0 of 5 places match (src/levels/shared/L06_0035EA68-g8.c)
     06_notak: 148 bytes, expected 152

#include "include_asm.h"
*/

typedef unsigned char u8;
typedef unsigned int u32;

extern float D_L06_001AF510 __attribute__((sda));
extern float D_L06_001AF514 __attribute__((sda));
extern float D_L06_001AF518 __attribute__((sda));
extern float D_L06_001AF51C __attribute__((sda));
extern float D_L06_001AF520 __attribute__((sda));

extern void func_002AEE70(u32 a0, void *a1, void *a2, float f0, float f1, float f2);
extern void func_002AEF28(u32 a0, void *a1, void *a2, float f0, float f1, float f2);

void func_L06_003E4F08(char *a0)
{
    if (*(u8 *)(a0 + 0x20) != 5 && *(u8 *)(a0 + 0x20) != 7) {
        char *s0 = *(char **)(a0 + 0x68);
        func_002AEE70((u32)a0, s0 + 0xEC, s0 + 0xE8, D_L06_001AF510,
                      D_L06_001AF514 * 0.000290888245f, *(float *)(s0 + 0x108));
        func_002AEF28((u32)a0, s0 + 0xF0, s0 + 0xF4, D_L06_001AF518,
                      D_L06_001AF51C * 0.000290888245f, D_L06_001AF520 * 0.000290888245f);
    }
}
