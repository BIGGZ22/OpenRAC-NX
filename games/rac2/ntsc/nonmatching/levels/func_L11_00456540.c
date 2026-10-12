/* func_L11_00456540 -- brouillon w17 (vague 6, fichier src/levels/11_joba/L11_00453358.c)
 * Derniere ligne de tools/check.py : 11_joba: instruction at +0x8 differs (meme taille 148 o; seuls les deux registres callee-saved sont echanges: retail p->s0, q->s1, moi p->s1, q->s0)
 */

void func_L11_00456540(char *p)
{
    char *q = *(char **)(p + 0x68);

    if (*(short *)(q + 0x256) == 1) {
        func_L11_00441978(*(int *)(p + 0xB8), 0xDF9);
        *(int *)(q + 0x240) = 0;
        *(short *)(q + 0x256) = 0;
    } else {
        *(int *)(q + 0x240) = 0;
    }
    func_002A7470(p, 0, 0, 3);
    func_002A05A8(p);
    func_002A7750(p, 1, 0);
    *(int *)(p + 0x98) = 0;
    *(unsigned short *)(p + 0x34) = (*(unsigned short *)(p + 0x34) & 0xEFFF) | 0x47;
    *(char *)(p + 0x31) = 0;
}
