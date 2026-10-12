/* func_L00_00401D50 — NON MATCHE, meilleur essai — 212 o attendus, 27 placements.
 * Derniere ligne check.py : "00_aranos_tutorial: 204 bytes, expected 212"
 * puis "func_L00_00401D50: 0 of 27 places match".
 *
 * Corps quasi identique : il manque exactement la paire
 *   beql $s0,$zero, <test de boucle>  ;  lw $s0,0x28($s0)   (delay)
 * que le retail emet AVANT le chargement de node->obj, c'est-a-dire un test
 * `if (node == 0) goto next` redondant a l'interieur du corps. Teste sans
 * l'obtenir : boucle `while`, boucle `for(;node;node=node->next)`, chaine de
 * `&&`, `continue` separes. Le reste (etat 0xFE/0xFD, obj==0, *(s16*)(obj+0x46)
 * ==0x13, appel flottant, duplication de node=node->next sur chaque sortie)
 * est reproduit instruction par instruction.
 *
 * ATTENTION : ce corps doit vivre dans le frere -g8.c — le retail charge le
 * flottant en `lwc1 $f1,0x1CA0($gp)` en flot ordinaire (=> fichier -G8), tout
 * en gardant $gp libre pour lui. Les deux entiers du corps sont en absolu
 * (lui + lw replie), obtenus ici en les declarant comme des TABLEAUX (>8 octets
 * donc hors small-data a -G8), la ou D_L00_001B0C90 (float, 4 o) est en sda.
 */

typedef struct Node Node;
struct Node {
    unsigned char pad0[32];
    unsigned char state;
    unsigned char pad1[3];
    int *obj;
    Node *next;
};

extern int D_L00_001B39C8[4];
extern void *D_L00_001B3EA8[4];
extern float D_L00_001B0C90 __attribute__((sda));
extern float func_002EE2B0(char *a, char *b);
extern void func_L00_00381F80(Node *a, char *b);

void func_L00_00401D50(char *arg)
{
    Node *node;

    if ((D_L00_001B39C8[0] & 1) == 0)
        return;
    node = D_L00_001B3EA8[0];
    while (node != 0) {
        if (node->state == 0xFE)
            return;
        if (node->state == 0xFD)
            return;
        if (node->obj != 0 && *(short *)((char *)node->obj + 0x46) == 0x13 &&
            func_002EE2B0((char *)node + 16, arg + 16) < D_L00_001B0C90)
            func_L00_00381F80(node, arg);
        node = node->next;
    }
}
