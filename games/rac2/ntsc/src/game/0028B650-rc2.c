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

extern s32 func_0028AB20(s32);
extern s8 *D_001B188C;

void func_0028B650(s8 *a0, s32 a1) {
    s32 v;
    u16 t;

    v = func_0028AB20(a1);
    t = *(u16 *)(D_001B188C + v * 8);
    (*(s16 *)((s8 *)(a0) + (0x40))) = v;
    (*(s32 *)((s8 *)(a0) + (0))) = t;
    (*(u8 *)((s8 *)(a0) + (0x42))) = *(u8 *)(D_001B188C + v * 8 + 6);
    (*(s32 *)((s8 *)(a0) + (0x44))) = *(u16 *)(D_001B188C + v * 8 + 4);
}
