#include "types.h"
extern s32 create_point_light(s32) __asm__("FUN_00201ba8");
extern s32 detach_point_light(s32) __asm__("FUN_00201f88");
void refresh_point_light(s32 arg0) __asm__("FUN_00201f58");

void refresh_point_light(s32 arg0) {
    detach_point_light(arg0);
    create_point_light(arg0);
}
