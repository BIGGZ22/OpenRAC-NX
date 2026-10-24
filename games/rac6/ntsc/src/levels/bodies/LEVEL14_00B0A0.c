/* LEVEL14_00B0A0 -- LEVEL14.REL 0x00B0A0, 128 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Trois flottants (f1, f2, f3) sont recus en $f12, $f13, $f14 et
 *   transmis a func_00AF38 ; le resultat de la chaine de transformations
 *   est ajoute composante par composante au vecteur source.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct VEC4 VEC4;

struct VEC4 {
    float x;
    float y;
    float z;
    float w;
};

typedef struct MATRIX4 MATRIX4;

struct MATRIX4 {
    float m[16];
};

extern void func_00AF38(float f1, float f2, float f3, VEC4 *out);
extern void func_1D6638(VEC4 *out, MATRIX4 *m, VEC4 *v);

void func_00B0A0(float f1, float f2, float f3, VEC4 *dst, MATRIX4 *m, VEC4 *src)
{
    VEC4 t1;
    VEC4 t2;

    func_00AF38(f1, f2, f3, &t1);
    func_1D6638(&t2, m, &t1);

    dst->x = src->x + t2.x;
    dst->y = src->y + t2.y;
    dst->z = src->z + t2.z;
}
