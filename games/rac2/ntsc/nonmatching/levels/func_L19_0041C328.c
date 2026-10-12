/* Draft (1 essai, 160 o sur 176 attendus) - func_L19_0041C328.
   Derniere ligne check.py : "19_grelbin: 160 bytes, expected 176".
   Code et flux reconstruits ; seul l ORDRE DES BLOCS differe : le retail place
   le corps (0x41C358) AVANT le bloc de tests (0x41C380) et l entree saute aux
   tests ; ma version place les tests en tete. Le reste (chargements, $gp, tables)
   est conforme. A reprendre : forme de boucle qui fait sortir ce layout. */

extern char *D_L19_001FB5A0[];

extern int D_L19_001B2A1C;

extern int D_L19_001B2A38 __attribute__((sda));

void func_L19_0041C328(char *a0)
{
    unsigned char v = *(unsigned char *)(a0 + 0x21);
    char *e;
    int y;
    unsigned short x;

    if (v == 0xFF) {
        return;
    }
    e = D_L19_001FB5A0[v];
    for (;;) {
        v = *(unsigned char *)(a0 + 0x21);
        if (v == 0xFF) {
            return;
        }
        if (D_L19_001B2A38 < v) {
            return;
        }
        if (e == 0) {
            return;
        }
        if (*(short *)(e - 2) >= 0) {
            x = *(unsigned short *)e;
        } else if (e == D_L19_001FB5A0[v]) {
            x = *(unsigned short *)e;
        } else {
            return;
        }
        y = D_L19_001B2A1C + ((x & 0x7FFF) << 8);
        if (y == 0) {
            return;
        }
        *(int *)(*(int *)(y + 0x68) + 0x240) = 1;
        e = e + 2;
    }
}
