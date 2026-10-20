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

extern void func_00282CF0(f32 *a, s32 b, s8 *c);
extern void func_00282D30(f32 *a, f32 *b, f32 c);
extern void func_002830C0(f32 *a, f32 *b, s8 *c);
extern s8 D_001B5340[];
extern s8 D_001B9250[];
extern s32 D_001A7340;
extern s32 D_001A7344;

void func_0027C768(s32 arg0, f32 *arg1, f32 *arg2) {
    f32 sp[4];
    s8 *base;
    f32 den;
    f32 temp_f1;
    f32 temp_f2;

    base = D_001B5340;
    func_00282CF0(sp, arg0, base);
    func_00282D30(sp, sp, 1024.0f);
    sp[3] = 1.0f;
    func_002830C0(sp, sp, base - 0x100);
    den = sp[3];
    func_00282D30(sp, sp, *(f32 *)D_001B9250 / den);
    temp_f1 = sp[0] + (f32)(D_001A7340 >> 1);
    *arg1 = temp_f1;
    *arg1 = temp_f1 / (f32)D_001A7340;
    temp_f2 = sp[1] + (f32)(D_001A7344 >> 1);
    *arg2 = temp_f2;
    *arg2 = temp_f2 / (f32)D_001A7344;
}
