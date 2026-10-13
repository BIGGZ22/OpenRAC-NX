/* DRAFT w49 — meilleur essai, NE MATCHE PAS : 236 o attendus, 236 o produits,
   seule difference = permutation des registres $s1/$s2 (pa/i).
   Derniere ligne check.py : func_L00_003ECC90: 0 of 27 places match
   00_aranos_tutorial: instruction at +0x18 differs

   TOUT le reste est identique au retail mot pour mot, y compris les paires
   lui/addiu des symboles, le movn, la boucle et le bloc de queue.
   Retail : $s1 = i, $s2 = a/pa. Ici : $s1 = pa, $s2 = i. Le compteur de boucle
   doit gagner le registre callee-saved le plus bas avant le pointeur ; ni
   l'ordre de declaration (done, i, pa, bv, base) ni des copies explicites des
   parametres (pa = a; bv = b;) ni la forme do/while ne l'ont change.
   Piste restante : le compteur gagne si son pseudo est cree avant celui du
   parametre (priorite local-alloc), p.ex. en utilisant `a` directement sans
   copie locale ET en declarin i avant tout usage de a.

   Les adresses 0x18A020, 0x189EA0 (resident) et 0x1D9F00, 0x1DA860 (donnees de
   niveau) DOIVENT etre des symboles : le retail emet `addiu` sur la moitie
   basse (ori serait emis pour un litteral). Les valeurs ci-dessus sont celles
   du niveau 0. */
extern char D_0018A020[];
extern char D_00189EA0[];
extern char D_L00_001D9F00[];
extern char D_L00_001DA860[];

int func_L00_003D3DE0(char *a, char *b);
int func_L00_003EB610(char *a, char *b);
void func_L00_0035F180(char *a, int b, int c, char *d, int e);

void func_L00_003ECC90(char *a, int b)
{
    int done = 0;
    int i;
    char *pa;
    int bv;
    char *base;

    i = 3;
    pa = a;
    bv = b;
    base = D_0018A020;
    do {
        if (bv == 0xB02) {
            if (func_L00_003D3DE0(*(char **)(base + 0x2290), pa) != 0)
                done = 1;
        } else {
            if (func_L00_003EB610(*(char **)(base + 0x2290), pa) != 0)
                done = 1;
        }
    } while (--i >= 0);
    if (done != 0)
        return;
    func_L00_0035F180((char *)(*(int *)(*(int *)(D_L00_001D9F00 + *(unsigned char *)(D_L00_001DA860 + bv) * 4) + 0x28) + 0xC0),
                      0, 0, D_00189EA0, 0x400);
}
