/* MobyUtil_IsShotClear__FPC4VEC3T0b -- LEVEL11.REL 0x034EA0, 256 bytes.
 *
 * Tells whether the shot is clear: the two points are compared, the
 * distance is normalised, and the result is handed to the helper.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the division route (the pinned chain plus the division
 * padding) compiles this source to the retail bytes of that
 * placement, byte for byte; the route is measured, its provenance
 * is known and its control keeps every other body exact.
 * The program-level gate is still pending, so the body is proved,
 * not promoted.
 */

/* LEVEL11.REL 0x034EA0 - MobyUtil_IsShotClear__FPC4VEC3T0b (256 octets)
 *
 * GARDE : ce corps est octet pour octet identique au retail avec la chaine
 * "divpad" du kit (cc1 + patch de division 0002), mais la chaine epinglee
 * gnu-ee-20261015 produit 248 octets au lieu de 256 : il manque les deux
 * `nop` de delai que le retail pose avant le `div.s`.
 * Voir abandonnes/NOTE-034EA0.md.
 *
 * Retail :

 *   0x034F50  nop
 *   0x034F54  nop

 */

typedef struct VEC3 {
    float x;
    float y;
    float z;
} VEC3;

extern float sqrtf(float x);

extern int MobyUtil_IsShotClear__FPC4VEC3T0fb(VEC3 *a, VEC3 *b, VEC3 *c,
                                              float len);

int MobyUtil_IsShotClear__FPC4VEC3T0b(VEC3 *p0, VEC3 *p1, VEC3 *p2)
{
    VEC3 d;
    float len;
    float r;
    float iz;

    d.x = p1->x - p0->x;
    d.y = p1->y - p0->y;
    d.z = p1->z - p0->z;
    len = d.x * d.x + d.y * d.y + d.z * d.z;

    if (len < 9.5367431640625e-07f) {
        len = 0;
    } else {
        r = sqrtf(len);
        iz = 1.0f / r;
        len = r;
        d.x = d.x * iz;
        d.y = d.y * iz;
        d.z = d.z * iz;
    }

    return MobyUtil_IsShotClear__FPC4VEC3T0fb(p0, &d, p2, len);
}
