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

extern void func_0012C9C8(s8 *);
extern s32 func_0012C878(s8 *, s32);
extern void func_00130300(s8 *);
extern void func_0012CBC0(s8 *);
extern void func_0012D060(s8 *);
extern void func_0012FA98(s32, void *);

s32 func_0012CAB0(s8 *arg0) {
    struct {
        s32 a;
        s32 pad;
        s64 b;
        s64 c;
    } blk;
    u32 v;

    for (;;) {
        func_0012C9C8(arg0);
        v = func_0012C878(arg0, 0x20);
        switch (v) {
        case 0x1B3:
            func_00130300(arg0);
            break;
        case 0x1B8:
            func_0012D060(arg0);
            break;
        case 0x100:
            func_0012CBC0(arg0);
            blk.a = 5;
            blk.b = -1;
            blk.c = -1;
            func_0012FA98(*(s32 *)(arg0 + 0x858), &blk);
            *(s64 *)(arg0 + 0x830) = blk.c;
            *(s64 *)(arg0 + 0x828) = blk.b;
            return *(s32 *)(arg0 + 0x150);
        case 0x1B7:
            return 0;
        }
    }
}
