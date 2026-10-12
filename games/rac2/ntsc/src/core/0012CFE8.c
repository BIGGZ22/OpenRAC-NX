#include "include_asm.h"

typedef int s32;

typedef unsigned char u8;

/* Copied mechanically from Lombyte sdk/library/update_temp_track_data.c; current source and actual
 * SDK-built full object/ref body pinned in donor-proof.json. Algorithm,
 * layout, signedness, expressions and source order preserved. Function
 * rename and duplicate EE scalar-type/include context removal only.
 * The donor SDK compiler b9aef69 differs; RAC2 acceptance requires its own qualification. */
/*
MIT License

Copyright (c) 2026 Mateusz Kłysz

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
struct M2c_arg0
{
  u8 pad_0[0x150];
  s32 unk150;
  u8 pad_154[0x58];
  s32 unk1AC;
  u8 pad_1B0[0x69C];
  s32 unk84C;
  s32 unk850;
  s32 unk854;
};

void func_0012CFE8(struct M2c_arg0 *arg0, s32 arg1)
{
  s32 temp_2_30;
  s32 temp_3_21;
  s32 temp_4_32;
  s32 var_4_8;
  s32 var_7_4;
  var_7_4 = 0;
  var_4_8 = 0;
  if ((arg0->unk150 != 3) && (arg1 != 0))
  {
    if (arg1 < 0)
    {
      var_7_4 = arg0->unk854 == 0;
    }
    arg0->unk854 = 0;
    var_4_8 = arg1;
  }
  temp_3_21 = arg0->unk84C + arg1;
  arg0->unk1AC = temp_3_21;
  if ((var_7_4 != 0) && (var_4_8 >= arg1))
  {
    arg0->unk1AC = (s32) (temp_3_21 + 0x400);
  }
  ;
  temp_4_32 = arg0->unk1AC;
  arg0->unk850 = (s32) ((arg0->unk850 < temp_4_32) ? (temp_4_32) : (arg0->unk850));
}

INCLUDE_ASM("asm/core", func_0012D060);
