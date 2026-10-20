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

extern s16 D_001A63A8[];
extern void func_002B7578(void);
extern void func_00133230(void);
extern void func_00132028(void);
extern void func_00133220(void);
extern void func_00282A38(s32);

s16 func_002B7CE0(s32 arg0) {
    s16 *q;

    if (arg0 != 0) {
        while (1) {
            func_002B7578();
            func_00133230();
            func_00132028();
            func_00133220();
            if (D_001A63A8[2] == 0) {
                break;
            }
            func_00282A38(10000);
            if (D_001A63A8[2] == 0) {
                break;
            }
        }
    } else {
        func_002B7578();
        func_00133230();
        func_00132028();
        func_00133220();
    }
    q = D_001A63A8;
    return q[2];
}
