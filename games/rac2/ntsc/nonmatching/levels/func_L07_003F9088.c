/* func_L07_003F9088 -- meilleur essai (vague 3, worker w05, fichier src/levels/07_siberius/L07_003F4578.c)
 * Jumeau de 003F9028 (constantes 3/9 au lieu de 2/8). Meme residuel: lbu dans $a1 au lieu de $v1.
 */
extern void func_002A7470(u8 *a0, s32 a1, s32 a2, s32 a3);

void func_L07_003F9088(u8 *a0)
{
    u8 c = *(u8 *)(a0 + 0x43);
    u8 *p = *(u8 **)(a0 + 0x68);

    if (*(s32 *)(p + 0x420) == 0) {
        if (c == 3)
            return;
        func_002A7470(a0, 3, 0, 10);
    } else {
        if (c == 9)
            return;
        func_002A7470(a0, 9, 0, 10);
    }
}
