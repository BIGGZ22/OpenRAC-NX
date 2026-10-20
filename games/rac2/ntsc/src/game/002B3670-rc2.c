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

extern f32 func_00282E80(void *, s32);
extern s32 func_002A8AF0(s32, void *, s32);
extern struct { s32 w0; s32 w1; s32 w2; } D_0018C0B0;
extern volatile s32 D_001A8BB0;
extern s32 *D_001F1700[];

typedef int TI __attribute__((mode(TI)));

s32 func_002B3670(void *arg0, s32 *arg1, s32 arg2, s32 arg3, void *arg4, f32 fparg0, f32 fparg1) {
    TI buf;
    f32 temp_f20;
    s32 temp_a1;
    s32 *temp_a1_2;
    s32 *temp_a1_3;

    if (arg4 != 0) {
        buf = *(TI *)arg4;
    } else {
        buf = *(TI *)((s8 *)arg0 + 0x10);
    }
    temp_a1 = D_0018C0B0.w0;
    *arg1 = temp_a1;
    if (D_001A8BB0 != 0) {
        return 0;
    }
    temp_f20 = func_00282E80(&buf, temp_a1 + 0x10);
    if (arg2 != -1) {
        temp_a1_2 = D_001F1700[arg2];
        if (func_002A8AF0(*arg1 + 0x10, (s8 *)temp_a1_2 + 0x10, *temp_a1_2) != 0) {
            return 2;
        }
    } else {
        if (temp_f20 < fparg0) {
            return 2;
        }
    }
    if (arg3 != -1) {
        temp_a1_3 = D_001F1700[arg3];
        if (func_002A8AF0(*arg1 + 0x10, (s8 *)temp_a1_3 + 0x10, *temp_a1_3) != 0) {
            return 1;
        }
    } else {
        if (temp_f20 < fparg1) {
            return 1;
        }
    }
    return 0;
}
