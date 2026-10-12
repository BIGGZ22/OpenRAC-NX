/* Draft (2 essais, 460 o sur 508 attendus) - func_L19_00423CC0.
   Derniere ligne check.py : "19_grelbin: 460 bytes, expected 508".
   Corps reconstruit complet mais l arbre de dispatch du switch (ordre/forme des
   4 branches de tete) ne se reproduit pas avec switch a 2 cas ni avec les cas
   permutes. A reprendre : forme exacte de test de tete. */

extern void func_30DFE8();

extern int func_31E128(char *, int);

extern void func_318240(char *, int, int);

extern unsigned int D_001A79F0 __attribute__((sda));

extern unsigned char D_00189EA0[];

extern unsigned char D_L19_0019B4A8[];

extern unsigned char D_L19_0021EA30[];

void func_L19_00423CC0(char *a0)
{
    int obj = *(int *)(a0 + 0x68);
    unsigned short h;
    unsigned char *e;
    char *q;

    if (obj == 0) {
        func_30DFE8();
        return;
    }
    switch (*(unsigned char *)(a0 + 0x20)) {
    case 1:
        if (func_31E128(D_00189EA0, *(int *)obj) != 0) {
            h = *(unsigned short *)(a0 + 0xB2);
            e = &D_L19_0019B4A8[((short)h >> 1) + (D_001A79F0 << 10)];
            if (h & 1) {
                if (((*e >> 4) & 7) != 7) {
                    *e += 0x10;
                }
            } else {
                if ((*e & 7) != 7) {
                    *e += 1;
                }
            }
            *(unsigned short *)(a0 + 0x34) = (*(unsigned short *)(a0 + 0x34) & 0xFFBE) | 0x1000;
            q = *(char **)(a0 + 0x24);
            *(int *)(a0 + 0x98) = *(int *)(q + 0x10);
            *(int *)(a0 + 0x98) = *(int *)(q + 0x10);
            func_318240(a0, 2, -1);
        }
        h = *(unsigned short *)(a0 + 0xB2);
        if (D_L19_0021EA30[(short)h + 0x464] == 0) {
            e = &D_L19_0019B4A8[((short)h >> 1) + (D_001A79F0 << 10)];
            if (h & 1) {
                if (((*e >> 4) & 7) == 0) {
                    return;
                }
            } else {
                if ((*e & 7) == 0) {
                    return;
                }
            }
        }
        *(unsigned short *)(a0 + 0x34) = (*(unsigned short *)(a0 + 0x34) & 0xFFBE) | 0x1000;
        q = *(char **)(a0 + 0x24);
        *(int *)(a0 + 0x98) = *(int *)(q + 0x10);
        *(int *)(a0 + 0x98) = *(int *)(q + 0x10);
        func_318240(a0, 2, -1);
        break;
    case 0:
        *(int *)(a0 + 0x98) = 0;
        *(unsigned short *)(a0 + 0x34) = (*(unsigned short *)(a0 + 0x34) | 0x41) & 0xEFFF;
        func_318240(a0, 1, -1);
        *(unsigned char *)(a0 + 0x30) = 0xFF;
        break;
    }
}
