/* GU_IsOnScreen__FPC4VEC3 -- LEVEL10.REL 0x0001D818, 244 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   GU_IsOnScreen__FPC4VEC3 -- LEVEL10.REL 0x01D818, 244 bytes.
 *   Body translated from the retail module.
 *   GU_IsOnScreen__FPC4VEC3  (LEVEL10.REL 0x01D818, 244 octets)
 *   Retail: addiu $sp,-0x50 / sd $s0,0x10 / daddu $s0,$a0 / sd $s1,0x18
 *   swc1 $f22,0x40 / daddu $s1,$zero,$zero / sd $ra,0x20
 *   swc1 $f21,0x38 / jal CAMDRIVER_GetWorldMatrix__Fv
 *   swc1 $f20,0x30 / lwc1 $f0,0x30($v0) / addiu $v1,$v0,0x30
 *   lwc1 $f2,0x0($s0) / addiu $a0,$v0,0x20 / lwc1 $f3,0x4($s0)
 *   sub.s $f2,$f2,$f0 / lwc1 $f1,0x8($s0) / mtc1 $zero,$f7
 *   swc1 $f2,0x0($sp) / mul.s $f5,$f2,$f2 / lwc1 $f0,0x4($v1) ...
 *   c.lt.s $f7,$f2 / nop / bc1f .Lend / daddu $v0,$s1,$zero
 *   jal CAMERA_GetCosHalfFovX__Fv / mul.s $f21,$f2,$f2
 *   jal CAMERA_GetCosHalfFovX__Fv / mov.s $f20,$f0
 *   mul.s $f20,$f20,$f0 / mul.s $f20,$f20,$f22
 *   c.lt.s $f20,$f21 / nop / bc1tl .Lend / addiu $s1,$zero,1
 *   .Lend: daddu $v0,$s1,$zero / ld $s0 / ld $s1 / ld $ra
 *   lwc1 $f22,0x40 / lwc1 $f21,0x38 / lwc1 $f20,0x30
 *   jr $ra / addiu $sp,$sp,0x50
 *   The point is put in the camera space: the difference with the camera
 *   position is squared into a distance, and the dot product with the camera
 *   axis says whether the point is in front of the camera.  The half field of
 *   view is read twice and the two results are multiplied (the retail calls
 *   the same getter twice); the answer is 1 when the point is visible.
 *   The two rows of the matrix are reached through pointers: the retail
 *   computes the address of the third row once ($v1) and of the second row
 *   once ($a0), then indexes them with the small displacements of the three
 *   components.  That is the form that gives the base+displacement loads.
 *   Placement: module-relative offset of the disc module.
 *   Proof: the route gprfirst (gnu-ee-20261015-gprfirst:-O2 -G0) compiles this
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

typedef struct VEC4 {
    float x;
    float y;
    float z;
    float w;
} VEC4;

typedef struct MATRIX4 {
    VEC4 r0;
    VEC4 r1;
    VEC4 r2;
    VEC4 r3;
} MATRIX4;

extern MATRIX4 *CAMDRIVER_GetWorldMatrix__Fv(void);
extern float CAMERA_GetCosHalfFovX__Fv(void);

int GU_IsOnScreen__FPC4VEC3(VEC3 *pos)
{
    MATRIX4 *m = CAMDRIVER_GetWorldMatrix__Fv();
    VEC4 *r2 = &m->r2;
    VEC4 *r3 = &m->r3;
    VEC3 d;
    float cosA, cosB, dist2, dot, dot2, v;
    int res;

    res = 0;

    d.x = pos->x - r3->x;
    d.y = pos->y - r3->y;
    d.z = pos->z - r3->z;

    dist2 = d.x * d.x + d.y * d.y + d.z * d.z;
    dot = d.x * r2->x + d.y * r2->y + d.z * r2->z;

    if (0 < dot) {
        dot2 = dot * dot;
        cosA = CAMERA_GetCosHalfFovX__Fv();
        cosB = CAMERA_GetCosHalfFovX__Fv();
        v = cosA * cosB * dist2;
        if (dot2 > v)
            res = 1;
    }

    return res;
}
