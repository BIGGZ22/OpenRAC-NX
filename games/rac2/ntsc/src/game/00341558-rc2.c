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

extern s8 D_00265290[];
s32 func_00341558(s32 arg0) {
    s32 temp_a1;
    s32 var_a2;
    s32 tmp0;

    temp_a1 = (*(s32 *)((s8 *)(arg0) + (0x2FC)));
    tmp0 = arg0 + (temp_a1 * 4);
    var_a2 = *(s32 *)(((s32)D_00265290) + (s32) (((*(s32 *)((s8 *)(tmp0) + (0x1B8))) * 4) + (temp_a1 << 5)));
    if ((temp_a1 == 2) && (*(u8 *)0x1A7A0A == 0)) {
        var_a2 = ((*(s32 *)((s8 *)(arg0) + (0x1C0))) >= 3) ? 0 : var_a2;
    }
    return var_a2;
}
