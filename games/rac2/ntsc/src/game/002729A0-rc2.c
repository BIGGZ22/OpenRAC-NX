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

extern s32 func_0027C878(s32);
extern void func_0027E4A8(f32, f32, s32, s32, s32, s32, s32, f32, f32, s32, s32, f32, f32, f32);
extern f32 func_00283180(f32);
extern f32 func_00283198(f32);
extern f32 func_00283B98(f32, f32);

void func_002729A0(s8 *arg0, f32 fparg0, f32 fparg1)
{
    f32 v[2][4];
    s32 s2;
    s32 type;
    s32 i;
    f32 x;
    f32 k;

    k = 40.0f;
    s2 = func_0027C878((*(s32 *)((s8 *)(arg0) + (0x18))));
    type = (*(s32 *)((s8 *)(arg0) + (0x2C)));
    x = (*(f32 *)((s8 *)(arg0) + (0x1C)));
    switch (type) {
    case 0:
        if ((*(s16 *)((s8 *)(arg0) + (0x26))) > 0) {
            i = 0;
            do {
                func_0027E4A8(fparg0, fparg1, 63, 63, s2, 0xFFFFF3, (*(s32 *)((s8 *)(arg0) + (0x14))),
                              k * (*(f32 *)((s8 *)(arg0) + (0x10))), k * (*(f32 *)((s8 *)(arg0) + (0x10))), 0, 0,
                              x, 0.0f, 0.0f);
                i++;
                x = func_00283B98(x, (*(f32 *)((s8 *)(arg0) + (0x28))));
            } while (i < (*(s16 *)((s8 *)(arg0) + (0x26))));
        }
        break;
    case 1:
        v[0][0] = func_00283198(x) * k * (*(f32 *)((s8 *)(arg0) + (0x10)));
        v[0][1] = func_00283180(x) * k * (*(f32 *)((s8 *)(arg0) + (0x10)));
        v[1][0] = func_00283180(x) * k * (*(f32 *)((s8 *)(arg0) + (0x10)));
        v[1][1] = func_00283198(x) * -k * (*(f32 *)((s8 *)(arg0) + (0x10)));
        ((void (*)(f32, f32, f32, f32, f32, s32, s32, s32, s32, s32, s32, s32, f32, f32))func_0027E4A8)(
            fparg0, fparg1, (*(f32 *)((s8 *)(arg0) + (0x10))) * k, (*(f32 *)((s8 *)(arg0) + (0x10))) * k, x,
            63, 63, s2, 0xFFFFF3, (*(s32 *)((s8 *)(arg0) + (0x14))), 0, 0, 0.0f, 0.0f);
        ((void (*)(f32, f32, f32, f32, f32, s32, s32, s32, s32, s32, s32, s32, f32, f32))func_0027E4A8)(
            fparg0 + v[1][0], fparg1 + v[1][1], (*(f32 *)((s8 *)(arg0) + (0x10))) * k, (*(f32 *)((s8 *)(arg0) + (0x10))) * k, x,
            63, 63, s2, 0xFFFFF3, (*(s32 *)((s8 *)(arg0) + (0x14))), 1, 0, 0.0f, 0.0f);
        ((void (*)(f32, f32, f32, f32, f32, s32, s32, s32, s32, s32, s32, s32, f32, f32))func_0027E4A8)(
            fparg0 - v[0][0], fparg1 - v[0][1], (*(f32 *)((s8 *)(arg0) + (0x10))) * k, (*(f32 *)((s8 *)(arg0) + (0x10))) * k, x,
            63, 63, s2, 0xFFFFF3, (*(s32 *)((s8 *)(arg0) + (0x14))), 0, 1, 0.0f, 0.0f);
        ((void (*)(f32, f32, f32, f32, f32, s32, s32, s32, s32, s32, s32, s32, f32, f32))func_0027E4A8)(
            (fparg0 + v[1][0]) - v[0][0], (fparg1 + v[1][1]) - v[0][1], (*(f32 *)((s8 *)(arg0) + (0x10))) * k,
            (*(f32 *)((s8 *)(arg0) + (0x10))) * k, x, 63, 63, s2, 0xFFFFF3, (*(s32 *)((s8 *)(arg0) + (0x14))), 1, 1, 0.0f, 0.0f);
        break;
    case 2:
        ((void (*)(f32, f32, f32, f32, f32, s32, s32, s32, s32, s32, s32, s32, f32, f32))func_0027E4A8)(
            fparg0, fparg1, (*(f32 *)((s8 *)(arg0) + (0x10))) * k, (*(f32 *)((s8 *)(arg0) + (0x10))) * k, x,
            63, 63, s2, 0xFFFFF3, (*(s32 *)((s8 *)(arg0) + (0x14))), 0, 0, 0.5f, 0.5f);
        break;
    }
}
