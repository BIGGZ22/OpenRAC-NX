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

extern s8 D_00189EA0[];
extern s8 ref_L04_00441100_00[];
extern s8 D_001395C8;
extern s32 ref_L04_00441100_01(s8 *, s32);
extern void ref_L04_00441100_02(s8 *);
extern void ref_L04_00441100_03(s8 *, s32, s32);

void func_L04_00441100(s8 *arg0) {
    s32 *p;
    s32 a1;

    if (*(u8 *)(arg0 + 0x20) == 0) {
        p = *(s32 **)(arg0 + 0x68);
        a1 = *p;
        if (a1 != -1 && ref_L04_00441100_01(D_00189EA0, a1) != 0) {
            ref_L04_00441100_02(ref_L04_00441100_00);
            D_001395C8 = 1;
            ref_L04_00441100_03(arg0, 1, -1);
        }
    }
}
