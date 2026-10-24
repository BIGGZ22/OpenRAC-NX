/* PAUSEMENU_Update__FP10tPAUSEMENUUi -- FE.REL 0x035178, 128 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Le menu de pause n'a pas d'element courant (champ 0x04 nul) : la mise a
 *   jour renvoie 0. Sinon elle repositionne la selection, puis, si le bit 1 du
 *   drapeau 0x14 est pose, elle cumule le deplacement renvoye par
 *   func_0359B8 dans le compteur 0x68.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct tPAUSEMENU {
    unsigned char unknown_000[0x04];
    unsigned int field_04;
    unsigned char unknown_008[0x0C];
    unsigned int field_14;
    unsigned char unknown_018[0x50];
    unsigned int field_68;
} tPAUSEMENU;

extern void PAUSEMENU_HideAllCustomCursors__FP10tPAUSEMENU(tPAUSEMENU *self);
extern unsigned int PAUSEMENU_SeekSelection__FP10tPAUSEMENUUi(tPAUSEMENU *self,
                                                                unsigned int index);
extern void func_0358C0(tPAUSEMENU *self);
extern unsigned int func_0359B8(tPAUSEMENU *self);

unsigned long long PAUSEMENU_Update__FP10tPAUSEMENUUi(tPAUSEMENU *self,
                                                        unsigned int arg1)
{
    int selection;

    PAUSEMENU_HideAllCustomCursors__FP10tPAUSEMENU(self);

    if (self->field_04 == 0) {
        return 0;
    }

    selection = PAUSEMENU_SeekSelection__FP10tPAUSEMENUUi(self, arg1);
    func_0358C0(self);
    if (self->field_14 & 2) {
        self->field_68 += func_0359B8(self);
    }

    return selection;
}
