#include "common.h"

typedef int TI128 __attribute__((mode(TI)));

extern char D_00189E20[];

extern char D_L20_00139568[];

extern char D_L20_0026D970[];

extern s32 func_2C0C30(s32 a0);

/* which of the three screens the moon-level front end should hand back: two sub-states answer
   with a fixed code, the rest read the current selection and its entry */
/*W23B*/
INCLUDE_ASM("asm/levels", func_L20_002B7A48);
/*W23E*/

INCLUDE_ASM("asm/levels", func_L20_002B93E0);

INCLUDE_ASM("asm/levels", func_L20_002C57D0);

INCLUDE_ASM("asm/levels", func_L20_002C5A48);

INCLUDE_ASM("asm/levels", func_L20_002C5D00);

INCLUDE_ASM("asm/levels", func_L20_002C63F0);

INCLUDE_ASM("asm/levels", func_L20_002C7588);
