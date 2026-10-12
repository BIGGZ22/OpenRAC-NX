/* w13 LABANDON (presque) — func_L01_00397950, 212 o, 23 places.
 * Derniere ligne de check.py (5e compilation) :
 *   func_L01_00397950: 0 of 23 places match
 *   (retail) "instruction at +0x8 differs"
 * Corps identique au retail instruction par instruction (deux boucles, ordre
 * des 4 stores flottants 0x50/0x54/0x58/0x5C corrige), SAUF l'allocation des
 * deux callee-saved : le retail met le pointeur d'entite ($68) dans $s1 et le
 * compteur de boucle dans $s2 ; ce candidat fait l'inverse. $s0 (float *) est
 * correct dans les deux. Aucun levier de source trouve : l'ordre de
 * declaration (compteur en premier, en dernier) ne change rien a l'allocation.
 *
 * Ordre des stores 0x50..0x5C (regle mesuree) : la source [50, 54, 5C, 58]
 * donne l'emission retail [5C, 58, 50, 54] (rotation de 2) ; l'ordre naif
 * [50,54,58,5C] et l'ordre inverse donnaient tous deux un desaccord.
 */
extern int func_002A77E0(int);

void func_L01_00397950(char *a0)
{
    int i;
    char *s1 = *(char **)(a0 + 0x68);
    float *s0 = (float *)(s1 + 0x20);
    int *q;
    int v;

    for (i = 3; i >= 0; i--)
        *s0++ = (float)func_002A77E0(0xFF);

    *(float *)(s1 + 0x30) = -1.0f;
    *(float *)(s1 + 0x34) = -2.25f;
    *(float *)(s1 + 0x38) = 1.25f;
    *(float *)(s1 + 0x3C) = 2.5f;

    q = (int *)(s1 + 0x4C);
    v = 0xFF;
    for (i = 3; i >= 0; i--) {
        *q = v;
        q--;
        v -= 0x40;
    }

    *(float *)(s1 + 0x50) = 2.5f;
    *(float *)(s1 + 0x54) = 3.0f;
    *(float *)(s1 + 0x5C) = 2.5f;
    *(float *)(s1 + 0x58) = 3.0f;
}
