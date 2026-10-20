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

extern f32 *func_00297F98(s32);
extern s32 D_001A79F0;

void func_00297FE8(f32 fparg0, f32 fparg1, f32 *arg0, f32 *arg1, s32 arg2) {
    f32 *temp_s1;
    f32 *temp_s1_2;
    s32 temp_s0;

    temp_s0 = (~arg2 != 0) ? arg2 : D_001A79F0;
    if (temp_s0 <= -1) {
        temp_s0 = 0;
    }
    if (temp_s0 >= 0x15) {
        temp_s0 = 0;
    }
    temp_s1 = func_00297F98(temp_s0);
    *arg0 = (temp_s1[0] + (func_00297F98(temp_s0)[1] * fparg0)) * 0.001953125f;
    temp_s1_2 = func_00297F98(temp_s0);
    *arg1 = (temp_s1_2[2] + (func_00297F98(temp_s0)[3] * fparg1)) * 0.001953125f;
}
