/* GAMETARGET_CheckInsideTrigger__FPC7Trigger_LEVEL11_025B58 -- LEVEL11.REL 0x025B58, 32 bytes.
 *
 * Asks a trigger whether a fixed point lies inside it.
 *
 * Evidence:
 *   retail: TRIGGER_CheckPointInside(self, &VEC3_333F24).
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
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

typedef struct Trigger Trigger;

extern VEC3 D_333F24;

extern int TRIGGER_CheckPointInside__FPC7TriggerPC4VEC3(Trigger *trigger,
                                                         VEC3 *point);

int GAMETARGET_CheckInsideTrigger__FPC7Trigger(Trigger *self)
{
    return TRIGGER_CheckPointInside__FPC7TriggerPC4VEC3(self, &D_333F24);
}
