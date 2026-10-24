/* ThrowTieTether_GetClosestHighPriorityOnScreen__FPC4VEC3f -- LEVEL1.REL 0x0010FA78, 272 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Candidate for ThrowTieTether_GetClosestHighPriorityOnScreen__FPC4VEC3f --
 *   0x0010F978).
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015-gprfirst) compiles this source to the
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

typedef struct Priority {
    char pad_00[0x0C];
    unsigned char priority;
} Priority;

typedef struct ThrowTieTetherCable {
    char pad_00[0x30];
    VEC3 pos;
    char pad_3C[0x18];
    Priority *owner;
    char pad_58[0x0C];
    unsigned int flags;
    char pad_68[0x18];
} ThrowTieTetherCable;

extern void *D_26CB1C;
extern void *D_26CB20;

extern int GU_IsOnScreen__FPC4VEC3(VEC3 *pos);

void *ThrowTieTether_GetClosestHighPriorityOnScreen__FPC4VEC3f(VEC3 *pos,
                                                             float radius)
{
    ThrowTieTetherCable *cable = (ThrowTieTetherCable *)D_26CB1C;
    void *best = 0;
    float radius2 = radius * radius;

    while (cable < (ThrowTieTetherCable *)D_26CB20) {
        if ((cable->flags & 0x431) == 1) {
            VEC3 *at = &cable->pos;

            if (cable->owner->priority != 0) {
                if (GU_IsOnScreen__FPC4VEC3(at)) {
                    float dx = pos->x - at->x;
                    float dy = pos->y - at->y;
                    float dz = pos->z - at->z;
                    float distance2 = dx * dx + dy * dy + dz * dz;

                    if (distance2 <= radius2)
                        best = cable;
                }
            }
        }
        cable++;
    }
    return best;
}
