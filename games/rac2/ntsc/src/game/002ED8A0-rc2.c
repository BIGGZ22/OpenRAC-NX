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

extern s8 D_00262958[];

s32 func_002ED8A0(s32 arg0) {
    s32 b;
    s32 p;
    s8 *t1;
    s32 k ;
    s32 var_a0;
    s32 var_a2 ;

    var_a0 = arg0;
    var_a2 = -1;
    b = (s32)D_00262958;
    k = 0x28;
    t1 = (s8 *)(b - 0x14);
    for (;;) {
        if (var_a0 < 0) {
            goto done;
        }
        p = var_a0 * k;

        if (*(s32 *)(p + b) == 0) {
            return var_a2;
        }
        var_a2 = var_a0;
        var_a0 = *(s32 *)(p + (s32)t1);
    }
done:
    return var_a2;
}
