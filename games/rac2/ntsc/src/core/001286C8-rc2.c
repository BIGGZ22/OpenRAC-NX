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

extern s32 func_0011D620(s8 *a0, s32 a1, s32 a2, s8 *a3, s32 t0, s32 *t1, s32 t2, s32 t3, s32 s9);
extern s32 func_0011D810(s8 *a0, s32 a1);
extern void func_00128898(s8 *a0);
extern s8 D_00143130[];
extern s8 D_00143180[];
extern s8 D_0013B990[];

s32 func_001286C8(s32 arg0, s32 arg1, s32 *arg2, u8 *arg3) {
    s32 var_a1;
    s32 r;
    s8 *base;

    base = D_00143180;
    *(s32 *)D_00143180 = arg0;
    *(s32 *)(base + 4) = arg1;
    *(s32 *)(base + 8) = *arg2;
    var_a1 = 0;
    if (*arg2 > 0) {
        base += 0xC;
        do {
            base[var_a1] = arg3[var_a1];
            var_a1 += 1;
        } while (var_a1 < *arg2);
    }
    if (func_0011D810(D_00143130, var_a1) == 1) {
        return 0;
    }
    r = func_0011D620(D_00143130, 0x8000091B, 1, D_00143180, 0x400, D_00143180, 0x400, 0, 0);
    if (r < 0) {
        func_00128898(D_0013B990);
        return 0;
    }
    return 1;
}
