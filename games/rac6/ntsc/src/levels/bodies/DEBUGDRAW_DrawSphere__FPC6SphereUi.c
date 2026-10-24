/* DEBUGDRAW_DrawSphere__FPC6SphereUi -- LEVEL13.REL 0x0004DB78, 116 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   DEBUGDRAW_DrawSphere__FPC6SphereUi -- LEVEL13.REL 0x0004DB78, 116 bytes.
 *   Body translated from the retail module.
 *   DEBUGDRAW_DrawSphere__FPC6SphereUi  (LEVEL13.REL 0x0004DB78, 116 octets)
 *   Retail: addiu $sp,-0x50 / daddu $v1,$a0 / sd $s0,0x30 / daddu $a0,$sp
 *   sd $ra,0x38 / daddu $s0,$a1 / swc1 $f20,0x40 / addiu $a1,$sp,0x10
 *   lwc1 $f2,0x0($v1) / lwc1 $f1,0x4($v1) / lwc1 $f0,0x8($v1)
 *   swc1 $f2,0x20 / swc1 $f1,0x24 / swc1 $f0,0x28 / lq $v0,0x20
 *   lwc1 $f20,0xC($v1) / jal SetCenter / sq $v0,0x10
 *   mov.s $f12,$f20 / jal SetRadius / daddu $a0,$sp / daddu $a1,$s0
 *   jal SetARGB / addiu $a0,$sp,0x20 / ld $s0,0x30 / ld $ra,0x38
 *   lwc1 $f20,0x40 / jr $ra / addiu $sp,0x50
 *   The sphere of the debug layer is described on the stack: the centre is
 *   copied in a sixteen byte aligned vector (a quadword move, `lq`/`sq` on the
 *   R5900), the radius is read after the centre and crosses the call of
 *   `SetCenter` in the callee saved register $f20, and the colour is a four
 *   byte RGBA that shares the slot of the dead vector.
 *   Placement: module-relative offset of the disc module.
 *   Proof: the gprfirst-divctx-sqrt route
 *   (gnu-ee-20261015-gprfirst + contextual division padding + square root
 *   slots, :-O2 -G0) compiles this source to the retail bytes of that
 *   placement, byte for byte.  The program-level gate is still pending, so the
 *   body is proved, not promoted.
 *   The layout of the structures is the one this body reads; it is not
 *   confirmed by the data of the disc yet.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gprfirst-divctx-sqrt:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef unsigned int quad __attribute__((mode(TI)));

typedef struct { float x, y, z; } VEC3;
typedef struct { VEC3 center; float radius; } SPHERE;

/* The sixteen byte aligned vector of the retail: the copy of the centre is a
 * quadword move (lq/sq), so the object is a 128 bit quantity. */
typedef union {
    quad q;
    struct { float x, y, z; } f;
} AVEC3;

typedef struct { VEC3 center; float radius; } APESPHERE;
typedef struct { unsigned char c[4]; } APERGBA;

extern void SetCenter__9ApeSphereRC5AVec3(APESPHERE *sphere, const AVEC3 *v);
extern void SetRadius__9ApeSpheref(APESPHERE *sphere, float radius);
extern void SetARGB__7ApeRGBAUi(APERGBA *colour, unsigned int argb);

void DEBUGDRAW_DrawSphere__FPC6SphereUi(const SPHERE *src, unsigned int color)
{
    APESPHERE sphere;
    AVEC3 v;
    float radius;

    {
        AVEC3 tmp;
        tmp.f.x = src->center.x;
        tmp.f.y = src->center.y;
        tmp.f.z = src->center.z;
        radius = src->radius;
        v.q = tmp.q;
    }

    SetCenter__9ApeSphereRC5AVec3(&sphere, &v);
    SetRadius__9ApeSpheref(&sphere, radius);

    {
        APERGBA rgba;
        SetARGB__7ApeRGBAUi(&rgba, color);
    }
}
