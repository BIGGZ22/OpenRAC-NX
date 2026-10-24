/* CAMERA_PushSystemUpdate__F14CameraSystemIDPFPfPf14CameraSystemIDPv_P7MATRIX4Pv_LEVEL1_00A078 -- LEVEL1.REL 0x00A078, 64 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Retail: appends a pair of pointers to the sample list of the camera
 *   system id and increments its counter.  The samples of one camera system
 *   live in a 0x30 byte slot; the counter is at offset 0x20 of the slot.
 *   The slot is reached twice through the global array: that is what keeps
 *   the address arithmetic of the retail (base + 0x20 + id*0x30 for the
 *   counter, base + id*0x30 + 8*count for the samples).
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct CameraSample {
    void *first;
    void *second;
} CameraSample;

typedef struct CameraSlot {
    CameraSample samples[4];
    int count;
    unsigned char field_24[12];
} CameraSlot;

extern CameraSlot g_cameraSlots[];

void CAMERA_PushSystemUpdate__F14CameraSystemIDPFPfPf14CameraSystemIDPv_P7MATRIX4Pv(
    int id,
    float *first,
    float *second,
    int *third,
    void *fourth,
    void *matrix,
    void *sixth)
{
    int index = ++g_cameraSlots[id].count;

    g_cameraSlots[id].samples[index].first = first;
    g_cameraSlots[id].samples[index].second = second;
}
