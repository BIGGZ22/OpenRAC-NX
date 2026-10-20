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

struct V4 { f32 x; f32 y; f32 z; s32 w; };
typedef int TI __attribute__((mode(TI)));
extern TI D_001BAFA0;
extern s32 func_002761F8(s8 *a0, s8 *a1, s32 a2, s32 a3, s32 a4);
extern s32 func_00282978(s8 *p);
extern void func_00282CC0(s8 *a0, s8 *a1, struct V4 *v);
extern void func_002B9180(s8 *p);

void func_002C1EF0(s8 *arg0) {
    struct V4 tmp;
    s8 *s1;
    s8 *s2;
    s8 *s3;

    s3 = arg0 + 0x10;
    s2 = arg0 + 0x30;
    s1 = arg0 + 0x20;
    tmp.x = *(f32 *)(arg0 + 0x30);
    tmp.y = *(f32 *)(s2 + 4);
    tmp.z = *(f32 *)(s2 + 8);
    tmp.w = 0;
    func_00282CC0(s3, s3, &tmp);
    func_00282CC0(s1, s1, &tmp);
    if (func_00282978(arg0 + 0xA) != 0) {
        func_002B9180(arg0);
        return;
    }
    if (func_002761F8(s1, s3, 0x10, *(s32 *)(s2 + 0xC), 0) != 0) {
        TI v = D_001BAFA0;
        *(s16 *)(arg0 + 0xA) = 0;
        *(volatile TI *)(arg0 + 0x10) = v;
    }
}
