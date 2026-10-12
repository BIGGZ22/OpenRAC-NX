#include "include_asm.h"

unsigned int func_0012EA70(unsigned int *param_1, int param_2)
{
  unsigned int uVar1;
  
  uVar1 = *(param_1 + 2) + (param_2 >> 3);
  if (*(param_1 + 9) <= uVar1) {
    uVar1 = uVar1 - *(param_1 + 10);
  }
  return uVar1;
}

INCLUDE_ASM("asm/core", func_0012EA9C);
