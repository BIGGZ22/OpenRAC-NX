/* Navigate_IsValidPosInt__FPC8NavigatePC4VEC4bf_LEVEL11_00007E38 -- LEVEL11.REL 0x00007E38, 216 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   (216 octets)
 *   Controle de validite d'une position entiere.
 *   Le vecteur de position est copie dans le cadre (4 flottants a 0($sp)),
 *   puis ecrase au sol par GU_ClampToFloor quand le drapeau "flag" est pose.
 *   Le resultat est vrai si le moby est marque invalide, si le drapeau n'est
 *   pas pose, ou si l'ecrasement au sol a reussi.
 *   La forme "||" (evaluation court-circuitante) est celle du retail : elle
 *   garde le phi sur le resultat et les trois branches distinctes, la ou une
 *   cascade if/else est aplatie par le compilateur.
 *   Preuve : chaine gnu-ee-20261015-gprfirst:-O2 -G0, comparaison octet pour
 *   octet au retail au placement vise.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015-gprfirst:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct VEC4 { float x, y, z, w; } VEC4;

typedef struct Navigate {
    char pad0[0x10];
    int f10;                     /* 0x10 : un pointeur, teste avant l'appel */
    char pad1[4];
    int f18;                     /* 0x18 : un pointeur, teste avant l'appel */
    char pad2[0x31 - 0x1C];
    unsigned char f31;           /* 0x31 : un drapeau */
} Navigate;

extern int func_0683B0(int a0);
extern int func_123DB8(int a0, const VEC4 *pos);
extern int GU_ClampToFloor__FP4VEC3fPfb(float *pos, float height,
                                        int *out, int flag);

int Navigate_IsValidPosInt__FPC8NavigatePC4VEC4bf(
    const Navigate *self, const VEC4 *pos, int flag, float f)
{
    VEC4 v;
    int r;

    if (self->f10) {
        if (!func_0683B0(self->f10)) {
            return 0;
        }
    }
    if (self->f18) {
        if (func_123DB8(self->f18, pos)) {
            return 0;
        }
    }

    v.x = pos->x;
    v.y = pos->y;
    v.z = pos->z;
    v.w = pos->w;

    r = 0;
    if (self->f31 || !flag ||
        (unsigned char)GU_ClampToFloor__FP4VEC3fPfb(&v.x, f, 0, 1)) {
        r = 1;
    }
    return r;
}
