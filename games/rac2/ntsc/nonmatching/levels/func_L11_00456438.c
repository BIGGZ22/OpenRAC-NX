/* func_L11_00456438 -- brouillon w17 (vague 6, fichier src/levels/11_joba/L11_00453358.c)
 * Derniere ligne de tools/check.py : 11_joba: 268 bytes, expected 264 (1 instruction de trop: la constante 1 est materialisee deux fois ($t0 pour les sb, $a3 pour l'argument) alors que le retail reutilise $a3)
 */

void func_L11_00456438(char *p, int a1)
{
    char *q = *(char **)(p + 0x68);
    int v0;

    *(float *)(q + 0x20) = (float)*(short *)(q + 0x24);
    *(short *)(q + 0x256) = 0;
    *(short *)(q + 0x88) = 0;
    func_002B60B0(p, 0.004444444552063942f, 0.13333334028720856f,
                  0.0011635529808700085f, 0.10471977293491364f);
    func_002A7750(p, 3, 0);
    v0 = *(int *)(p + 0x24);
    *(short *)(q + 0x258) = a1;
    *(int *)(p + 0x98) = *(int *)(v0 + 0x10);
    *(char *)(q + 0x6A) = 0;
    *(char *)(q + 0x2E) = 1;
    *(char *)(p + 0x31) = 1;
    *(char *)(p + 0x7F) = 0;
    *(unsigned short *)(p + 0x34) = (*(unsigned short *)(p + 0x34) | 0x1000) & 0xFFB8;
    *(int *)(q + 0x200) = 0;
    func_002B3380(p, q + 0x1F0, q + 0x20, 1);
    if (*(short *)(q + 0x50) != 0) {
        *(short *)(q + 0x50) = 0;
        func_002A0488(p, *(unsigned char *)(q + 0x54), *(unsigned char *)(q + 0x55),
                      *(unsigned char *)(q + 0x56));
        *(short *)(q + 0x52) = 1;
    }
}
