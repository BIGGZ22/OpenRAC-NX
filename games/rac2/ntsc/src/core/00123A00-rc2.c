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

extern s32 func_00115F28(s32);
extern s32 *func_001236C8(void *, s32);
extern s32 func_00123930(s32);
extern void func_00123978(s32, void *, void *, void *);

void func_00123A00(void *arg0) {
    u32 loc[6];
    s32 *s0;
    s32 *temp1;
    s32 *temp2;
    s32 s4;
    s32 t;

    temp1 = (*(s32 **)((s8 *)(arg0) + (0xC)));
    if (temp1 != 0) {
        s0 = temp1;
        s4 = 0;
        if (*s0 != 0) {
            do {
                s4 += func_00123930(*s0);
                s0++;
            } while (*s0 != 0);
        }
    } else {
        s4 = func_00123930((*(s32 *)((s8 *)(arg0) + (8))));
    }
    t = s4 * 4;
    (*(s32 *)((s8 *)(arg0) + (0x10))) = s4;
    loc[0] = func_00115F28(t);
    loc[2] = func_00115F28(t);
    loc[4] = 0xFFFFFFFF;
    loc[1] = 0;
    loc[3] = 0;
    loc[5] = 0;
    temp2 = (*(s32 **)((s8 *)(arg0) + (0xC)));
    if (temp2 != 0) {
        s0 = temp2;
        if (*s0 != 0) {
            do {
                func_00123978(*s0, loc, &loc[4], &loc[5]);
                s0++;
            } while (*s0 != 0);
        }
    } else {
        func_00123978((*(s32 *)((s8 *)(arg0) + (8))), loc, &loc[4], &loc[5]);
    }
    (*(s32 **)((s8 *)(arg0) + (0xC))) = func_001236C8(loc, s4);
    (*(s32 *)((s8 *)(arg0) + (0))) = loc[4];
    (*(s32 *)((s8 *)(arg0) + (4))) = loc[5];
}
