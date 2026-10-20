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

extern void func_0027C540(s32);
extern void func_0027C660(void);
extern void func_0027FAA8(s32, s32, u64, s32, s32);
extern s32 func_00288FB8(s32);
extern void func_00300968(s32, s32, f32, f32, f32, f32);
extern s32 func_00336D50(s32, s32);
extern s32 D_001A8CFC;
extern s32 D_001ABAAC;
extern s32 D_001ABAB4;
extern s32 D_001ABABC;
extern f32 D_001B23A8;
extern s32 g_001ABAB0 __attribute__((sda));
extern s32 g_001ABAA8 __attribute__((sda));
extern s32 g_001ABAB8 __attribute__((sda));
extern f32 g_001ABAC0 __attribute__((sda));

s32 func_002D50F0(void) {
    s32 s0;
    s32 s1;

    func_0027C540(0);
    if (D_001A8CFC != 0) {
        func_00300968(func_00336D50(D_001A8CFC + 0x8710, 0x8B), 0x60442D00, (f32)g_001ABAB0, (f32)D_001ABAB4, D_001B23A8, g_001ABAC0);
        func_00300968(func_00336D50(D_001A8CFC + 0x8710, 0x8C), 0x55F0C070, (f32)g_001ABAB0, (f32)D_001ABAB4, D_001B23A8, g_001ABAC0);
        func_00300968(func_00336D50(D_001A8CFC + 0x8710, 0x8D), 0x80FFDE8D, (f32)g_001ABAB0, (f32)D_001ABAB4, D_001B23A8, 0.0f);
        s1 = g_001ABAB0;
        s1 = s1 + g_001ABAA8;
        s0 = D_001ABAB4;
        s0 = s0 + D_001ABAAC;
        func_0027FAA8(s1, s0, (u64)0x80F0F0F0, func_00288FB8(0x2D61), -1);
        s1 = g_001ABAB0;
        s1 = s1 + g_001ABAB8;
        s0 = D_001ABAB4;
        s0 = s0 + D_001ABABC;
        func_0027FAA8(s1, s0, (u64)0x80F0F0F0, func_00288FB8(0x2D4F), -1);
    }
    func_0027C660();
    return 0;
}
