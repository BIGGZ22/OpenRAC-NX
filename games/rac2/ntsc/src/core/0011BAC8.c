#include "common.h"
#include "structs.h"

/*
 * Close but not exact, same open-question category as r1_func_001160D8/
 * r1_func_00115578 (scratch-register/scheduling choice) but manifesting as
 * store reordering instead: retail schedules `self->field8 = ...;
 * self->field4 = 0;` before the branch and puts `self->fieldC = ...` in
 * the delay slot; this compiler schedules fieldC and field8 before the
 * branch and puts field4's store in the delay slot instead. Confirmed
 * source-order independent -- tried every permutation of the 3
 * assignments, all four produced the identical instruction sequence, so
 * this is the scheduler's own choice, not something this source
 * controls. Logic (r1_D_00154A40's first field = arg0, then fields at
 * 0x4/0x8/0xC of the pointed-to struct get 0/self+0x10/self+0x10, return
 * self) is fully understood and correct either way.
 *
 * extern void *r1_D_00154A40;
 *
 * void *func_0011BAC8(void *arg0) {
 *     char *self = (char *)&r1_D_00154A40;
 *     r1_D_00154A40 = arg0;
 *     *(void **)(self + 0x8) = self + 0x10;
 *     *(int *)(self + 0x4) = 0;
 *     *(void **)(self + 0xC) = self + 0x10;
 *     return self;
 * }
 */
extern void *r1_D_00154A40 NOT_SDA;

/* rac1-decomp src/core/00119868.c: func_00119868 */
void *func_0011BAC8(void *arg0) {
    char *self = (char *)&r1_D_00154A40;
    r1_D_00154A40 = arg0;
    *(int *)(self + 0x4) = 0;
    *(void **)(self + 0xC) = self + 0x10;
    *(void **)(self + 0x8) = self + 0x10;
    return self;
}
