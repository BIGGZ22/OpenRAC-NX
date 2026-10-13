/* func_L07_003F9028 -- meilleur essai (vague 3, worker w05, fichier src/levels/07_siberius/L07_003F4578.c)
 * Residuel: 92 o, 3 instructions sur 23. L'octet de 0x43($a0) est lu dans $a1 au lieu de $v1
 * (le retail reutilise le registre du drapeau, mort apres le bnez). Branche et agencement OK.
 * Essayes: u8 c, s32 c, declaration avant le pointeur -- toujours $a1.
 */
extern void func_002A7470(u8 *a0, s32 a1, s32 a2, s32 a3);

void func_L07_003F9028(u8 *a0)
{
    u8 *p = *(u8 **)(a0 + 0x68);
    s32 c = *(u8 *)(a0 + 0x43);

    if (*(s32 *)(p + 0x420) == 0) {
        if (c == 2)
            return;
        func_002A7470(a0, 2, 0, 10);
    } else {
        if (c == 8)
            return;
        func_002A7470(a0, 8, 0, 10);
    }
}
