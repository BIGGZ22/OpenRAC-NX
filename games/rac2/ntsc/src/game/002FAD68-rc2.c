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

extern s8 D_0021F240[];

void func_002FAD68(s32 arg0) {
    s32 i;
    s32 n;
    s32 one;
    s8 *base;
    s8 *q;
    s8 *p;

    base = D_0021F240;
    if ((*(s32 *)((s8 *)(base) + (0x740))) > 0) {
        i = 0;
        q = base;
        one = 1;
        p = q + 0x150;
        do {
            if ((*(s32 *)((s8 *)(p) + (-0x10))) == arg0) {
                (*(s32 *)((s8 *)(p) + (0))) = one;
            }
            p += 0x18;
            n = (*(s32 *)((s8 *)(q) + (0x740)));
            i++;
        } while (i < n);
    }
}
