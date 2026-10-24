/* ICONHORMENU_AddItem__FP12tICONHORMENUUiUiUiUiUiUiUiUiUiUiUiUiUiUiUi -- LEVEL2.REL 0x0B1FE8, 400 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   (400 octets)
 *   Forme retail : pas de cadre (la fonction n'appelle personne), `this` dans
 *   $t4, le premier parametre recopie dans $t5, la valeur de retour 1
 *   materialisee tot dans $v0.  Les arguments 8..15 sont empiles avec un pas
 *   de 8 octets a partir de 0x00($sp) (convention o32 de ce compilateur).
 *   Ordre des ecritures (celui du retail) : l'octet d'etat 0x00, puis les
 *   champs 0x24..0x3C, puis 0x04..0x1C, et enfin 0x20.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct tICONHORMENU tICONHORMENU;
typedef struct tICONHORMENUITEM tICONHORMENUITEM;

struct tICONHORMENUITEM {
    unsigned char m_state;
    unsigned char m_pad_001[3];
    unsigned int m_f04;
    unsigned int m_f08;
    unsigned int m_f0c;
    unsigned int m_f10;
    unsigned int m_f14;
    unsigned int m_f18;
    unsigned int m_f1c;
    unsigned int m_f20;
    unsigned int m_f24;
    unsigned int m_f28;
    unsigned int m_f2c;
    unsigned int m_f30;
    unsigned int m_f34;
    unsigned int m_f38;
    unsigned int m_f3c;
};

struct tICONHORMENU {
    tICONHORMENUITEM *m_items;
    unsigned char m_pad_004[0xC4];
    unsigned int m_adding;
    unsigned int m_count;
};

int ICONHORMENU_AddItem__FP12tICONHORMENUUiUiUiUiUiUiUiUiUiUiUiUiUiUiUi(
    tICONHORMENU *menu, unsigned int p1, unsigned int p2, unsigned int p3,
    unsigned int p4, unsigned int p5, unsigned int p6, unsigned int p7,
    unsigned int p8, unsigned int p9, unsigned int p10, unsigned int p11,
    unsigned int p12, unsigned int p13, unsigned int p14, unsigned int p15)
{
    if (menu->m_adding != 0) {
        return 0;
    }
    menu->m_items[menu->m_count].m_state = 1;
    menu->m_items[menu->m_count].m_f24 = p9;
    menu->m_items[menu->m_count].m_f28 = p10;
    menu->m_items[menu->m_count].m_f2c = p11;
    menu->m_items[menu->m_count].m_f30 = p12;
    menu->m_items[menu->m_count].m_f34 = p13;
    menu->m_items[menu->m_count].m_f38 = p14;
    menu->m_items[menu->m_count].m_f3c = p15;
    menu->m_items[menu->m_count].m_f04 = p1;
    menu->m_items[menu->m_count].m_f08 = p2;
    menu->m_items[menu->m_count].m_f0c = p3;
    menu->m_items[menu->m_count].m_f10 = p4;
    menu->m_items[menu->m_count].m_f14 = p5;
    menu->m_items[menu->m_count].m_f18 = p6;
    menu->m_items[menu->m_count].m_f1c = p7;
    menu->m_items[menu->m_count].m_f20 = p8;
    menu->m_count = menu->m_count + 1;
    return 1;
}
