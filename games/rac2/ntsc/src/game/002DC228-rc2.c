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

extern s8 *D_001F2854;
extern s8 D_00262BE0[];
extern void func_0027C540(s32);
extern void func_0027C660(void);
extern void func_0027FAA8(s32, s32, u64, s32, s32);
extern s32 func_00288FB8(s32);

s32 func_002DC228(s8 *a0) {
    s8 *v1;
    s32 s4;

    v1 = (*(s8 **)((s8 *)(D_001F2854) + (0xE8)));
    s4 = (*(s32 *)((s8 *)((*(s8 **)((s8 *)(v1) + (0x34))) + (*(s32 *)((s8 *)(v1) + (0x40))) * 12) + (4)));
    func_0027C540(0);
    if (s4 == -1) {
        func_0027FAA8((*(s32 *)((s8 *)(a0) + (0x20))) / 2, (*(s32 *)((s8 *)(a0) + (0x24))) / 2 - 8, 0x80FFA888ULL, func_00288FB8(0x2E65), -1);
    } else {
        s32 t = s4 * 12;
        func_0027FAA8((*(s32 *)((s8 *)(a0) + (0x20))) / 2, (*(s32 *)((s8 *)(a0) + (0x24))) / 3 - 8, 0x80FFA888ULL, func_00288FB8((*(s32 *)((s8 *)(D_00262BE0 + t) + (0)))), -1);
        func_0027FAA8((*(s32 *)((s8 *)(a0) + (0x20))) / 2, (*(s32 *)((s8 *)(a0) + (0x24))) * 2 / 3 - 8, 0x80FFA888ULL, func_00288FB8((*(s32 *)((s8 *)(D_00262BE0 + t) + (4)))), -1);
    }
    func_0027C660();
    return 2;
}
