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

extern s8 D_001A63A8[];

s32 func_L12_0041A6D0(s8 *arg0, u32 arg1) {
    s8 *d;
    s8 *b;
    s32 x;

    d = D_001A63A8;
    if ((*(s32 *)(d + 0x68) == 0) && (*(s32 *)(d + 0x24) == -1)) {
        b = *(s8 **)(arg0 + 0x68);
        if (arg1 >= 10) {
            return 0;
        }
        x = *(s32 *)(b + (arg1 << 2) + 0x230);
        *(s8 **)(d + 0x28) = arg0;
        *(s32 *)(d + 0x24) = x + 0x7D0;
        return 1;
    }
    return 0;
}
