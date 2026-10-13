/* DRAFT w49 — meilleur essai, NE MATCHE PAS : 52 o attendus, 52 o produits,
   seules les DEUX premieres instructions sont permutees.
   Derniere ligne check.py : func_L00_003ECB00: 0 of 27 places match
   00_aranos_tutorial: instruction at +0x0 differs
   retail  : addiu $v1,$zero,0x3 / lui $v0,0x1B / addiu $v0,$v0,0x7A4
   produit : lui $v0,0x1B / addiu $v1,$zero,0x3 / ori $v0,$v0,0x7A4

   Le JUMEAU de src/levels/shared/L00_003F8E08.c (func_L00_003FF7B8, 0x003FF7B8,
   matche) a exactement la meme boucle descendante `*w = c; w--; c -= 64;` et
   emet bien le compteur EN PREMIER (addiu $a0,$zero,0x3 puis le pointeur) :
   c'est la forme source copiee ici. La difference vient de l'adresse absolue :
   le jumeau initialise son pointeur depuis un registre (addiu $v0,$s1,0x2C),
   ici il faut lui+addiu sur une constante, et cc1 hisse cette paire avant
   l'initialisation du compteur quel que soit l'ordre des affectations source.
   Essaye sans succes : affectations simples puis `for (; i >= 0; i--)`,
   initialiseurs de declaration, forme do/while, ordre de declaration inverse,
   variable intermediaire `base`. */
void func_L00_003ECB00(void)
{
    int i;
    int *p;

    i = 3;
    p = (int *)0x1B07A4;
    for (; i >= 0; i--)
    {
        *p = 0;
        p--;
    }
}
