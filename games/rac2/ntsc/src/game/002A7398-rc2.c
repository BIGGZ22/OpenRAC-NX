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

extern void func_0029F390(s8 *);

void func_002A7398(s8 *arg0, s32 arg1, s32 arg2) {
    s32 s1;
    s8 *a0;
    s8 *a2;
    s8 *p2;
    s32 v0;
    s32 v1;
    s32 a1b;
    u8 a1c;
    f32 f0;

    s1 = arg1 * 4;
    a0 = (*(s8 **)((s8 *)(arg0) + (0x24)));
    a0 = a0 + 0x48;
    a0 = a0 + s1;
    a1c = (*(u8 *)((s8 *)(*(s8 **)a0) + (0x10)));
    (*(s8 *)((s8 *)(arg0) + (0x42))) = arg1;
    v0 = (arg2 < (s32) a1c) ? arg2 : (a1c - 1);
    a1b = v0 + 1;
    (*(s8 *)((s8 *)(arg0) + (0x40))) = v0;
    (*(s8 *)((s8 *)(arg0) + (0x41))) = a1b;
    a2 = *(s8 **)a0;
    v1 = (*(u8 *)((s8 *)(a2) + (0x10))) - 1;
    if (v1 < (s32) (u8) (*(s8 *)((s8 *)(arg0) + (0x41)))) {
        (*(s8 *)((s8 *)(arg0) + (0x41))) = v1;
    }
    (*(s8 *)((s8 *)(arg0) + (0x43))) = arg1;
    p2 = *(s8 **)((*(s8 **)((s8 *)(arg0) + (0x24))) + s1 + 0x48);
    a1c = (*(u8 *)((s8 *)(p2) + (0x10)));
    if ((u8) (*(u8 *)((s8 *)(arg0) + (0x41))) >= a1c) {
        (*(s8 *)((s8 *)(arg0) + (0x41))) = 0;
    }
    func_0029F390(arg0);
    f0 = *(*(f32 **)((s8 *)(arg0) + (0x58)));
    (*(f32 *)((s8 *)(arg0) + (0x4C))) = f0;
    (*(u8 *)((s8 *)(arg0) + (0x60))) = (*(u8 *)((s8 *)(arg0) + (0x60))) & 0xFD;
    (*(s8 *)((s8 *)(arg0) + (0x6C))) = (*(u8 *)((s8 *)(*(s8 **)((*(s8 **)((s8 *)(arg0) + (0x24))) + s1 + 0x48)) + (0x11)));
}
