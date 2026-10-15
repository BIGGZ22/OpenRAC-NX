#include "include_asm.h"

#include "common.h"

/* Small-data unit: the bodies whose state lives at $gp-relative offsets. */

extern float D_L11_001B12C8 __attribute__((sda));

/* Bleeds the ten rotation timers the level keeps beside its state: each armed
   entry loses a sixtieth of the shared rate and clamps at zero, disarming. */
INCLUDE_ASM("asm/levels", func_L11_004395C8);
