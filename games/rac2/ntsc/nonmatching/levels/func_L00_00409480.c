/* func_L00_00409480 — NON MATCHE, meilleur essai — 212 o attendus, 27 placements.
 * Derniere ligne check.py : "00_aranos_tutorial: 216 bytes, expected 212"
 * puis "func_L00_00409480: 0 of 27 places match".
 *
 * Seule difference : la valeur `c` nait dans $v1 puis est copiee dans $a2 pour
 * l'appel (un `daddu $a2,$v1,$zero` de trop), alors que le retail la fait naitre
 * dans $a2 (`addiu $a2,$zero,0x80; subu $a2,$a2,$v0`). Tout le reste — y compris
 * l'ordre des shifts/or et les 4 appels — est identique.
 * (Mettre l'appel avant le calcul de rgba fait vivre `c` a travers l'appel :
 * cc1 la place alors dans $s0 et le corps passe a 224 o. Pire.)
 */

extern int func_002EF250(float x);
extern void func_0037B508(int a, int b, int c);
extern void func_002E7DD8(int a, int b, int c, int d, int e);
extern short D_001A6480[];

void func_L00_00409480(char *arg)
{
    int *o = *(int **)(arg + 104);
    short *t;
    int c;
    int rgba;

    c = 128 - func_002EF250((float)(o[2] * 2));
    rgba = c << 24;
    rgba |= c << 16;
    rgba |= c << 8;
    rgba |= c;
    func_0037B508(66, 72, c);
    t = D_001A6480;
    func_002E7DD8(0, t[0xA9], 0, t[0xA8], rgba);
    func_002E7DD8(0, t[0xA9], 0, t[0xA8], rgba);
    func_002E7DD8(0, t[0xA9], 0, t[0xA8], rgba);
    func_002E7DD8(0, t[0xA9], 0, t[0xA8], rgba);
}
