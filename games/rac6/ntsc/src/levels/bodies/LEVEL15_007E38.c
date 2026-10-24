/* LEVEL15_007E38 -- LEVEL15.REL 0x007E38, 216 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Navigate_IsValidPosInt__FPC8NavigatePC4VEC4bf -- LEVEL15.REL 0x00007E38, 216 bytes.
 *   Body translated from the retail module (LEVEL15.REL).
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015-gprfirst) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct Navigate Navigate;
typedef struct Trigger Trigger;
typedef struct Moby Moby;

struct Navigate {
    unsigned char pad00[0x10];
    Trigger *trigger;
    unsigned char pad14[4];
    Moby *moby;
    unsigned char pad1C[0x15];
    unsigned char f31;
};

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

extern int TRIGGER_CheckPointInside__FPC7TriggerPC4VEC3(
    Trigger const *trigger, VEC3 const *point);
extern int TriggerCollection_CheckPointInside__FPC4MobyPC4VEC3(
    Moby const *moby, VEC3 const *point);
extern int GU_ClampToFloor__FP4VEC3fPfb(
    VEC3 *point, float height, void *owner, int mode);

int Navigate_IsValidPosInt__FPC8NavigatePC4VEC4bf(
    Navigate const *self, VEC4 const *point, int flag, float height)
{
    int ok;
    VEC4 v;

    if (self->trigger
        && !TRIGGER_CheckPointInside__FPC7TriggerPC4VEC3(
            self->trigger, (VEC3 const *)point))
        return 0;
    if (self->moby
        && TriggerCollection_CheckPointInside__FPC4MobyPC4VEC3(
            self->moby, (VEC3 const *)point))
        return 0;

    v.x = point->x;
    v.y = point->y;
    v.z = point->z;
    v.w = point->w;

    ok = 0;
    if (self->f31 || !flag
        || (unsigned char)GU_ClampToFloor__FP4VEC3fPfb(
            (VEC3 *)&v, height, 0, 1))
        ok = 1;
    return ok;
}
