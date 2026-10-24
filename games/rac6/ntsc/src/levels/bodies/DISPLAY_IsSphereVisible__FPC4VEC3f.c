/* DISPLAY_IsSphereVisible__FPC4VEC3f -- FE.REL 0x0000E408, 148 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   DISPLAY_IsSphereVisible__FPC4VEC3f -- FE.REL 0x00E408, 148 bytes.
 *   Body translated from the retail module.
 *   DISPLAY_IsSphereVisible__FPC4VEC3f  (FE.REL 0x00E408, 148 octets)
 *   Retail: addiu $sp,-0x20 / sd $s0 / swc1 $f20 / daddu $s0,$a0 / sd $ra
 *   jal DISPLAY_GetVisibilityPlanes__Fv / mov.s $f20,$f12
 *   neg.s $f20,$f20 ...
 *   lwc1 $f5,0x0($s0) / lwc1 $f4,0x4($s0) / lwc1 $f6,0x8($s0)
 *   lwc1 $f0,0x0($v0) / lwc1 $f2,0x4($v0) / mul.s $f0,$f5,$f0 ...
 *   c.lt.s $f0,$f20 / bc1f / bnel $v0,$v1 / lwc1 $f0,0x0($v0)
 *   The five visibility planes of the clip plane list are tested against the
 *   sphere: the sphere is hidden as soon as it lies entirely behind one of
 *   them (the signed distance is below the negated radius).
 *   Placement: module-relative offset of the disc module; the other
 *   placements of the same body are listed in config/levels/bodies.tsv.
 *   Proof: the gprfirst route (gnu-ee-20261015-gprfirst:-O2 -G0) compiles this
 *   source to the retail bytes of that placement, byte for byte.  The program-
 *   level gate is still pending, so the body is proved, not promoted.
 *   The layout of the structures is the one this body reads; it is not
 *   confirmed by the data of the disc yet.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015-gprfirst:-O2 -G0) compiles this source to the
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

typedef struct ClipPlanes ClipPlanes;

extern ClipPlanes *DISPLAY_GetVisibilityPlanes__Fv(void);

int DISPLAY_IsSphereVisible__FPC4VEC3f(VEC3 *center, float radius)
{
    ClipPlanes *planes = DISPLAY_GetVisibilityPlanes__Fv();
    float *plane = (float *)planes + 0x60 / 4;
    float *end = (float *)planes + 0xB0 / 4;
    float negRadius = -radius;

    do {
        float distance = center->x * plane[0] + center->y * plane[1]
                       + center->z * plane[2] + plane[3];

        if (!(distance < negRadius)) {
            plane += 4;
        } else {
            return 0;
        }
    } while (plane != end);

    return 1;
}
