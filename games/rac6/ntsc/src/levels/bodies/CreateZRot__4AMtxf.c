/* CreateZRot__4AMtxf -- FE.REL 0x0009C240, 48 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   La matrice est Tournee autour de l'axe Z constant (UNIT_Z, donnee
 *   module-locale en 0x229670) ; l'angle recu est transmis en $f12, l'objet
 *   est rendu en $v0.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct AMtx AMtx;
typedef struct AVec3 AVec3;

extern const AVec3 D_229670;          /* _10AVec3Const$UNIT_Z */

extern void MtxFromAxisAngle__5AMathR4AMtxRC5AVec3f(AMtx *self, const AVec3 *axis,
                                                     float angle);

AMtx *CreateZRot__4AMtxf(AMtx *self, float angle)
{
    MtxFromAxisAngle__5AMathR4AMtxRC5AVec3f(self, &D_229670, angle);

    return self;
}
