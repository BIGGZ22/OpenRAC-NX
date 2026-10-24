/* GAMETARGET_CheckInsideTrigger__FPC7Trigger_LEVEL18_025A70 -- LEVEL18.REL 0x025A70, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   La fonction demande au declencheur s'il contient le point fixe global
 *   situe a l'offset de module 0x2B9224 (lui 0x2C / addiu -0x6DDC).
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct Trigger Trigger;
typedef struct VEC3 {
    float x;
    float y;
    float z;
} VEC3;

extern int TRIGGER_CheckPointInside__FPC7TriggerPC4VEC3(const Trigger *trigger,
                                                         const VEC3 *point);
extern VEC3 D_2B9224;

int GAMETARGET_CheckInsideTrigger__FPC7Trigger(Trigger *trigger)
{
    return TRIGGER_CheckPointInside__FPC7TriggerPC4VEC3(trigger, &D_2B9224);
}
