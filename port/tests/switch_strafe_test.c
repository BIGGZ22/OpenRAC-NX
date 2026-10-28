/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "strafe.h"
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "%d: %s\n", __LINE__, #x); exit(1); } } while (0)
int main(void) {
    rac1_strafe_input in = {1, RAC1_STRAFE_STATE_RUN, 0, 0,
        RAC1_STRAFE_SLOT_READY, 0x0F, RAC1_PAD_L2, 0.0f, 0.0f, 0.0f};
    rac1_strafe_hold hold = {0};
    rac1_strafe_hold_update(RAC1_STRAFE_HOLD, &in, &hold);
    CHECK(rac1_strafe_kind(RAC1_STRAFE_HOLD, &in, &hold) != RAC1_STRAFE_NONE);
    in.held = 0;
    rac1_strafe_hold_update(RAC1_STRAFE_HOLD, &in, &hold);
    CHECK(rac1_strafe_kind(RAC1_STRAFE_HOLD, &in, &hold) == RAC1_STRAFE_NONE);
    in.held = RAC1_PAD_R2;
    rac1_strafe_hold_update(RAC1_STRAFE_HOLD, &in, &hold);
    CHECK(rac1_strafe_kind(RAC1_STRAFE_HOLD, &in, &hold) == RAC1_STRAFE_NONE);
    in.held = RAC1_PAD_L2;
    in.playing = 0;
    rac1_strafe_hold_update(RAC1_STRAFE_HOLD, &in, &hold);
    CHECK(rac1_strafe_kind(RAC1_STRAFE_HOLD, &in, &hold) == RAC1_STRAFE_NONE);
    in.playing = 1;
    in.vehicle_mode = 1;
    rac1_strafe_hold_update(RAC1_STRAFE_HOLD, &in, &hold);
    CHECK(rac1_strafe_kind(RAC1_STRAFE_HOLD, &in, &hold) == RAC1_STRAFE_NONE);
    CHECK(RAC1_STRAFE_BUTTONS == RAC1_PAD_L2);
    return 0;
}
