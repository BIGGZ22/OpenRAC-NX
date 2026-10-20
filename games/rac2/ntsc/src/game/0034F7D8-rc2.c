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

extern void func_00282A88(void *a0, s32 a1);
extern s32 func_00133850(s32 a0, s32 a1, s32 a2, s32 a3, s32 t0, s32 t1);
extern s32 D_001B23D4;

struct S0 {
    s32 w00;
    s32 w04;
    s8 p08[0x28];
    s32 w30;
    s32 w34;
    s32 w38;
    s32 w3C;
    s32 w40;
    s32 w44;
    s32 w48;
    s32 w4C;
    s32 w50;
    s32 p54;
    s32 w58;
    s32 w5C;
    s32 w60;
};

s32 func_0034F7D8(struct S0 *a0, s32 a1, s32 a2, s32 a3) {
    s32 v;
    s32 r;

    func_00282A88(a0->p08, 0x20);
    a0->w34 = a1;
    a0->w40 = a2;
    a0->w04 = 3;
    a0->w00 = 0;
    a0->w30 = 0;
    a0->w38 = 0;
    a0->w3C = 0;
    a0->w44 = 0;
    a0->w50 = 0;
    a0->w58 = 0;
    a0->w5C = 0;
    a0->w60 = 0;
    D_001B23D4 = a3;
    a0->w4C = 0x400;
    v = func_00133850(0x400, 0x1000, 0x400, 0, 5, 3);
    a0->w48 = v;
    if (v < 0) {
        r = 1;
    } else {
        r = 0;
    }
    return r ^ 1;
}
