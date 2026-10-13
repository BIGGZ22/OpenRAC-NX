/* func_L11_0044C220 (68 o, a placer dans le frere L11_004465F8-g8.c) -- MEILLEUR ESSAI
 * check.py : 11_joba: instruction at +0x14 differs
 * Ecart restant = ordonnancement des 8 arguments entiers et allocation de la
 * base 0x68 (retail $v0, nous $a3). La carte argument->registre est correcte :
 *   a0=&D_L11_001B1C10 (gp+0x2C20), a1=4, a2=v0+0x270, a3=v0+0x370,
 *   a4(t0)=*(4B4), a5(t1)=*(4B0), a6(t2)=D_L11_001B1C50, a7(t3)=*(0x1B1C54),
 *   f12=*(4B8).
 * Consequence utile : l'ABI retail est n64/n32 -- 8 registres d'arguments
 * entiers a0-a3 + t0-t3, plus f12-f19 pour les flottants (flux independants).
 * C'est ce qui expliquait les "chargements morts" t2/t3/t0/t1.
 * gp = 0x001AEFF0 (tools/build.py), donc gp+0x2C60 -> D_L11_001B1C50 (sda),
 * gp+0x2C20 -> adresse de D_L11_001B1C10 (sda), 0x001B1C54 -> D_L11_001B1C54
 * (nosda : le retail le charge en lui/lw absolu, pas en $gp).
 * Le fichier -g8 a ete retire du depot (marqueur remis dans le fichier principal),
 * le marqueur de func_L11_0044C220 est restaure.
 */
#include "common.h"

extern void func_2FD7E8(void *a0, int a1, void *a2, void *a3, int a4, int a5,
                        int a6, int a7, float f12);

extern s32 D_L11_001B1C50 __attribute__((sda));
extern s32 D_L11_001B1C54 __attribute__((nosda));
extern s32 D_L11_001B1C10 __attribute__((sda));

void func_L11_0044C220(char *a0)
{
    char *v0 = *(char **)(a0 + 0x68);

    func_2FD7E8(&D_L11_001B1C10, 4, v0 + 0x270, v0 + 0x370,
                *(s32 *)(v0 + 0x4B4), *(s32 *)(v0 + 0x4B0),
                D_L11_001B1C50, D_L11_001B1C54,
                *(f32 *)(v0 + 0x4B8));
}
