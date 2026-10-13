/* func_L07_003F8C38 -- meilleur essai (vague 3, worker w05, fichier src/levels/07_siberius/L07_003F4578.c)
 * Residuel: taille ET allocation de registres IDENTIQUES (s0=a0, s2=0x68, s1=resultat,
 * obtenus en declarant le resultat avant le pointeur). Il ne reste que des transpositions
 * d'ordonnancement de quelques instructions independantes:
 *   * addiu $a0,0x5 / daddu $a1,0 inverse pour l'appel func_002E5CF8(5,0,a0)
 *   * la base de table est chargee dans $a1 au lieu de $v0 (d'ou addu $v1,$v1,$a1 vs $v0)
 *   * daddu $a0,$s1 place apres le chargement de la table au lieu d'avant
 */
typedef struct { u8 pad38[0x38]; f32 f38; u8 rest[128 - 0x3C]; } SibRec128;
extern SibRec128 *D_L07_001B23CC;
extern u8 *func_L07_003F6B88(u8 *a);
extern void func_L07_003F6C40(u8 *a, u8 *b, void *c, f32 d);
extern void func_0029FCC0(u8 *a, s32 b, void *c);
extern void func_002E5CF8(s32 a, s32 b, u8 *c);

void func_L07_003F8C38(u8 *a0)
{
    u8 *s1;
    u8 *s2 = *(u8 **)(a0 + 0x68);
    u8 buf[16];

    s1 = func_L07_003F6B88(*(u8 **)(s2 + 0x184));
    if (s1 == 0)
        return;
    func_0029FCC0(a0, 0, buf);
    func_002E5CF8(5, 0, a0);
    func_L07_003F6C40(s1, a0, buf, D_L07_001B23CC[*(s32 *)(s2 + 0x174)].f38 + 0.5f);
}
