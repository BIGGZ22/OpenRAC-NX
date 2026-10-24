/* MobyUtil_IsShotClear__FPC4VEC3T0b_LEVEL10_034EA0 -- LEVEL10.REL 0x034EA0, 256 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Le vecteur p1 - p0 est normalise ; la longueur normalisee est passee
 *   en 4e argument de MobyUtil_IsShotClear__FPC4VEC3T0fb.
 *   La racine est une variable distincte de `len` : la division divise par
 *   $f0 (la racine) et non par $f12 (la copie passee en argument).
 *   PROFILE : astra-divpad:-O2 -G0.  La chaine epinglee des modules
 *   (gnu-ee-20261015) ne pose pas les deux nop qui precedent le div.s
 *   (regle de division du compilateur retail) : elle s'arrete a 248 octets
 *   (1re difference 0x034F14).  La route astra-divpad de E0/profiles.json
 *   reproduit ces deux nop et rend le corps exact.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (astra-divpad:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct VEC3 {
    float x;
    float y;
    float z;
} VEC3;

extern int MobyUtil_IsShotClear__FPC4VEC3T0fb(VEC3 *a, VEC3 *b, VEC3 *c,
                                              float len);

int MobyUtil_IsShotClear__FPC4VEC3T0b(VEC3 *p0, VEC3 *p1, VEC3 *p2)
{
    VEC3 d;
    float len;
    float iz;

    d.x = p1->x - p0->x;
    d.y = p1->y - p0->y;
    d.z = p1->z - p0->z;
    len = d.x * d.x + d.y * d.y + d.z * d.z;

    if (len < 9.5367431640625e-07f) {
        len = 0;
    } else {
        float r;
        r = sqrtf(len);
        iz = 1.0f / r;
        len = r;
        d.x = d.x * iz;
        d.y = d.y * iz;
        d.z = d.z * iz;
    }

    return MobyUtil_IsShotClear__FPC4VEC3T0fb(p0, &d, p2, len);
}
