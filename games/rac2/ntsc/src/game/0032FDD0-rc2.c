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

extern void func_00282C88(f32 *v);
extern f32 func_00282DF0(f32 *v);
extern s8 D_00189E20[];
extern f32 D_001B284C;

void func_0032FDD0(s8 *a0) {
    s8 *base;
    s8 *obj;
    f32 vec[2];

    obj = *(s8 **)(a0 + 0x68);
    base = D_00189E20;
    (*(f32 *)((s8 *)(obj) + (0x54))) = -(*(f32 *)((s8 *)(base) + (0x1FB0)));
    (*(f32 *)((s8 *)(obj) + (0x50))) = -(*(f32 *)((s8 *)(base) + (0x1FB4)));
    if ((*(f32 *)((s8 *)(obj) + (0x54))) > 1.0f) {
        (*(f32 *)((s8 *)(obj) + (0x54))) = 1.0f;
    } else if ((*(f32 *)((s8 *)(obj) + (0x54))) < -1.0f) {
        (*(f32 *)((s8 *)(obj) + (0x54))) = -1.0f;
    }
    if ((*(f32 *)((s8 *)(obj) + (0x50))) > 1.0f) {
        (*(f32 *)((s8 *)(obj) + (0x50))) = 1.0f;
    } else if ((*(f32 *)((s8 *)(obj) + (0x50))) < -1.0f) {
        (*(f32 *)((s8 *)(obj) + (0x50))) = -1.0f;
    }
    func_00282C88(vec);
    vec[0] = (*(f32 *)((s8 *)(obj) + (0x50)));
    vec[1] = (*(f32 *)((s8 *)(obj) + (0x54)));
    D_001B284C = func_00282DF0(vec);
}
