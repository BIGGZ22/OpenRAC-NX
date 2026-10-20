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

extern s8 *D_001B22A8;
extern s8 *D_001B15F4;
extern s8 *D_001B15F8;
extern void func_0029D0E0(s8 *p);
extern void func_002FC748(void);

void func_0027C660(void) {
    s8 *t;
    s8 *v;
    s8 *a;
    s32 s0;
    t = D_001B22A8;
    D_001B15F8 = t;
    s0 = 0x20000000;
    v = D_001B15F4;
    D_001B22A8 = t + 0x10;
    *(s32 *)v = s0;
    a = D_001B15F4;
    *(s32 *)(a + 4) = (s32)D_001B22A8;
    *(s32 *)(D_001B15F4 + 8) = 0;
    *(s32 *)(D_001B15F4 + 0xC) = 0;
    func_0029D0E0(a);
    func_002FC748();
    v = D_001B22A8;
    *(s32 *)v = s0;
    t = D_001B15F4;
    a = D_001B22A8;
    *(s32 *)(a + 4) = (s32)(t + 0x10);
    v = D_001B22A8;
    *(s32 *)(v + 8) = 0;
    a = D_001B22A8;
    *(s32 *)(a + 0xC) = 0;
    t = D_001B22A8;
    v = D_001B15F8;
    D_001B22A8 = t + 0x10;
    *(s32 *)v = s0;
    t = D_001B22A8;
    a = D_001B15F8;
    *(s32 *)(a + 4) = (s32)t;
    v = D_001B15F8;
    *(s32 *)(v + 8) = 0;
    t = D_001B15F8;
    *(s32 *)(t + 0xC) = 0;
}
