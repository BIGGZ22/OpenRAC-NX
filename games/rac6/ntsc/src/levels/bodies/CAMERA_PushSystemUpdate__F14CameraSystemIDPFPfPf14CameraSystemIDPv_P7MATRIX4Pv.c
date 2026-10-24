/* CAMERA_PushSystemUpdate__F14CameraSystemIDPFPfPf14CameraSystemIDPv_P7MATRIX4Pv -- LEVEL13.REL 0x00A078, 64 bytes.
 *
 * Body translated from the retail module.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct CameraUpdate {
    float *pos;
    float *target;
} CameraUpdate;

typedef struct CameraSystem {
    CameraUpdate updates[4];
    int count;
    void *matrix;
    int pad[2];
} CameraSystem;

/* the camera system table lives at 0x2C8FD8 in the module (anonymous) */
extern CameraSystem D_2C8FD8[];

void CAMERA_PushSystemUpdate__F14CameraSystemIDPFPfPf14CameraSystemIDPv_P7MATRIX4Pv(
    int id, float *pos, float *target)
{
    int n = D_2C8FD8[id].count + 1;
    D_2C8FD8[id].count = n;
    D_2C8FD8[id].updates[n].pos = pos;
    D_2C8FD8[id].updates[n].target = target;
}
