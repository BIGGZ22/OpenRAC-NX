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

extern s8 D_0022AA10[];
extern void func_00316BC8(s32 a, s32 b);
extern void func_00281EB8(void *o, s32 a1, s32 a2, s32 a3, f32 f12, f32 f13, f32 f14);
extern f32 func_002A7878(f32 a, f32 b);

void func_003169D0(s32 arg0) {
    s32 i;
    s32 q;
    s8 *p;
    s8 *base;

    base = D_0022AA10;
    p = base + 0x30;
    q = (s32) base;
    for (i = 0; i < 3; i++) {
        if ((*(s32 *)((s8 *)(p) + (0x5C))) == arg0) {
            func_00316BC8(arg0, q);
            if ((*(s32 *)((s8 *)(p) + (0x64))) == 0) {
                func_00281EB8(p, 0x31, 0x3F007F00, 1, func_002A7878(0.125f, 0.25f) * 5.0f, 1.25f, 0.0f);
                func_00281EB8(p, 0x31, 0x5F7F7F7F, 1, func_002A7878(0.0625f, 0.125f) * 5.0f, 1.24f, 0.0f);
                func_00281EB8(p, 0xB, 0x10003F00, 1, func_002A7878(0.15f, 0.35f) * 5.0f, 1.23f, 0.0f);
            }
        }
        p += 0xA0;
        q += 0xA0;
    }
}
