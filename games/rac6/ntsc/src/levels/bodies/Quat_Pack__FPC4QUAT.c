/* Quat_Pack__FPC4QUAT -- LEVEL24.REL 0x00162260, 200 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Quat_Pack__FPC4QUAT -- LEVEL24.REL 0x00162260, 200 bytes.
 *   The retail packs a quaternion into one 32-bit word.
 *   The retail copies the sixteen bytes of the quaternion (four floats) to
 *   the frame with the unaligned pairs ldl/ldr and sdl/sdr, so the
 *   structure is packed.  It tests the sign of the fourth component
 *   (c.lt.s against 0.0) and negates the three first ones in the frame
 *   copy.  Then it scales the three components by the constant
 *   0x447FF99A (1023.9f) -- the second one by half of it
 *   (0x3F000000, 0.5) -- converts them to integers (cvt.w.s) and packs
 *   them on eleven bits each, with the biases 0x400, 0x200 and 0x400.
 *   Placement: module-relative offset of the disc module.
 *   Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to
 *   the retail bytes of that placement, byte for byte.  The program-level
 *   gate is still pending, so the body is proved, not promoted.
 *   The layout of the structures is the one this body reads; it is not
 *   confirmed by the data of the disc yet.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct __attribute__((packed)) {
    float x;
    float y;
    float z;
    float w;
} QUAT;

#define QUAT_SCALE 1023.9f

unsigned int Quat_Pack__FPC4QUAT(const QUAT *q)
{
    QUAT t = *q;
    int ix, iy, iz;

    if (t.w < 0.0f) {
        t.x = -t.x;
        t.y = -t.y;
        t.z = -t.z;
    }

    ix = (int)(t.x * QUAT_SCALE);
    iy = (int)(t.y * QUAT_SCALE * 0.5f);
    iz = (int)(t.z * QUAT_SCALE);

    return (unsigned int)(ix + 0x400)
         | ((unsigned int)(iy + 0x200) << 11)
         | ((unsigned int)(iz + 0x400) << 21);
}
