/* DRAFT w49 — meilleur essai, NE MATCHE PAS : 140 o attendus, 140 o produits,
   une seule instruction de place differente (voir note).
   Derniere ligne check.py : func_L00_003EA7D0: 0 of 27 places match
   00_aranos_tutorial: instruction at +0x34 differs

   L'adresse 0x1B2BA0 doit etre un SYMBOLE (le retail utilise `addiu` pour la
   moitie basse, ori serait emis pour une constante litterale). Toute forme
   symbolique essayee (scalaire ou tableau, `char*` ou `unsigned int`) fait
   tomber l'adresse dans un registre separe ($s0/$s1) et fait grossir la trame
   a 144/148/152 o ; le litteral garde 140 o mais emet `ori` ET ordonnance
   `lui $a1,0x1B` un cran trop tot dans le PREMIER appel seulement (le second
   appel est identique au retail). Reste a resoudre : placer la paire
   lui/addiu du 2e argument dans le registre d'argument sans registre
   intermediaire.

   Le reste du corps (prologue, beql+delay slot, appels 1 et 3) est identique
   au retail mot pour mot. */
void func_L00_0037B508(unsigned int a, unsigned long b);

extern void func_L00_003EA8C8(char *a, unsigned int b, unsigned int c, float d);

void func_L00_003EA7D0(char *p)
{
    if (*(unsigned char *)(p + 0x31) == 0) {
        return;
    }
    func_L00_0037B508(0x47, 0x53001UL);
    func_L00_003EA8C8(p, (unsigned int)0x1B2BA0, 0x7F10307F, 0.25f);
    func_L00_003EA8C8(p, (unsigned int)0x1B2BA0, 0x3F08183F, 0.5f);
    func_L00_0037B508(0x47, 0x5360bUL);
}
