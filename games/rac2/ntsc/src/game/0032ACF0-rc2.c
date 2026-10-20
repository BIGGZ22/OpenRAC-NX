/* SPDX-License-Identifier: MIT
 * MIT License
 *
 * Copyright (c) 2026 RC2 decomp contributions
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "common.h"

extern s8 D_00232760[];
extern s8 D_00189E20[];
extern s32 func_00282948(s8 *);
extern void func_00282CC0(s8 *, s8 *, s8 *);
extern void func_00282F20(s8 *, s8 *, f32);
extern f32 func_00283B98(f32, f32);
extern f32 func_00283CE0(s32);
extern s32 func_00283CF0(f32);
extern void func_002ACEC0(s8 *, s8 *, s8 *, f32);
extern void func_002FE8D8(s32, s8 *);
extern void func_0032AE30(s32, s8 *, s32);

void func_0032ACF0(s32 arg0) {
    s8 sp[16];
    s8 sp10[16];
    f32 temp_f20;
    f32 temp_f20_2;
    s8 *var_s3;
    s8 *var_s0;
    s8 *end;
    s8 *base;
    s8 *p5;

    base = D_00232760;
    var_s3 = base + 0x18;
    var_s0 = base;
    end = var_s0 + 0x240;
    p5 = D_00189E20;
    do {
        if ((*(s32 *)((s8 *)(var_s0) + (0x1C))) == arg0) {
            func_002FE8D8((*(s32 *)((s8 *)(p5) + (0x2290))) + 0x10, sp);
            temp_f20 = func_00283CE0((*(s32 *)((s8 *)(var_s0) + (0x18))));
            temp_f20_2 = temp_f20 / func_00283CE0(0x25);
            func_002ACEC0(sp10, var_s0, sp, (*(f32 *)((s8 *)(var_s0) + (0x10))));
            func_0032AE30(arg0, sp10, func_00283CF0(temp_f20_2 * 128.0f));
            (*(f32 *)((s8 *)(var_s0) + (0x10))) = func_00283B98((*(f32 *)((s8 *)(var_s0) + (0x10))), (*(f32 *)((s8 *)(var_s0) + (0x14))));
            func_00282F20(sp, sp, 0.04f);
            func_00282CC0(var_s0, var_s0, sp);
            if (func_00282948(var_s3) != 0) {
                (*(s32 *)((s8 *)(var_s0) + (0x1C))) = 0;
            }
        }
        var_s0 += 0x30;
        var_s3 += 0x30;
    } while ((s32)var_s0 < (s32)end);
}
