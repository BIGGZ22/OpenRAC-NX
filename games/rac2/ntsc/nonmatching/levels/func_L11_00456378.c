/* func_L11_00456378 -- brouillon w17 (vague 6, fichier src/levels/11_joba/L11_00453358.c)
 * MEILLEUR ESSAI (log2) : 11_joba: 184 bytes, expected 192 -- allocation de registres
 * CONFORME au retail (p->$s1, q->$s0) et tout le corps identique SAUF la forme de branche
 * (moi `beq $v0,$s2` + nop, retail `bnel $v0,$s2` avec delay slot utile) et le bloc
 * "then" place du mauvais cote. Variante 1 appel = 160 o, variante 2 appels = 200 o (2 jal).
 * Retail = 192 o avec UN SEUL jal partage par les deux chemins.
 */

void func_L11_00456378(char *p)
{
    char *q = *(char **)(p + 0x68);
    int v;

    if (*(unsigned char *)(p + 0x20) < 4) {
        *(int *)(q + 0x240) = 0;
    } else {
        if (*(short *)(q + 0x256) != 1) {
            if (func_L11_00441868(*(int *)(p + 0xB8), *(short *)(p + 0xAA)) == 0) {
                *(short *)(q + 0x256) = 0;
                return;
            }
            *(short *)(q + 0x256) = 1;
        }
        v = func_002B3BA8(p, *(int *)(q + 0x24C), 0, 0, 100.0f, 100.0f);
        *(int *)(q + 0x240) = *(int *)(q + 0x220);
        *(short *)(q + 0x254) = v;
    }
}
