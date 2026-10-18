/* functional: high. Retail calls FUN_L00_0020d498 with the slot it was given in $4 (never set here), then returns
 * whether the result minus 0x1C is below 2 (sltiu on the unsigned difference). The port's generated wrapper passed 0 for the slot. */
#include "types.h"

int FUN_L00_0020d498(int) __asm__("FUN_L00_0020d498");

int FUN_L00_0020d540(int slot) {
    return (unsigned)(FUN_L00_0020d498(slot) - 0x1c) < 2;
}
