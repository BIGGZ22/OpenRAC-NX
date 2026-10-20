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

extern s32 D_001ABA68 __attribute__((sda));
extern s32 D_001ABA6C __attribute__((sda));
extern s32 D_001A8CFC;
extern s32 D_001A7340;
extern f32 D_001B23A8;
extern struct { s32 w0; s32 w1; s32 w2; } D_00138340;
void func_0027C540(s32);
void func_0027C660(void);
s32 func_0027F980(s32, s32, u64, s32, s32);
s32 func_0027FAA8(s32, s32, u64, s32, s32);
s32 func_00288FB8(s32);
s32 func_002A9588(s32, s32, s32, s32, s32);
void func_00300A00(s32, s32, f32, f32, f32, f32, f32);
s32 func_00336D50(s32, s32);

s32 func_002D2538(void) {
    s32 s0;
    s32 s1;
    f32 zf;

    func_0027C540(0);
    zf = 0.0f;
    func_00300A00(func_00336D50(D_001A8CFC + 0x8710, 0xE5), 0x60442D00, (f32) (D_001A7340 / 2), (f32) D_001ABA6C, 1.0f, D_001B23A8, zf);
    func_00300A00(func_00336D50(D_001A8CFC + 0x8710, 0xE4), 0x55F0C070, (f32) (D_001A7340 / 2), (f32) D_001ABA6C, 1.0f, D_001B23A8, zf);
    s1 = func_002A9588(0x60241700, 0x55F0C070, 0x19, 0, 0);
    if (D_001ABA68 > 0) {
        s0 = 0x80F0F0F0;
        if (!(D_00138340.w0 & 0x8000)) {
            s0 = s1;
        }
        func_00300A00(func_00336D50(D_001A8CFC + 0x8710, 0x4B), s0, 216.0f, 373.0f, 1.0f, D_001B23A8 * 0.8f, zf);
        func_0027FAA8(0x53, 0x171, 0x80F0F0F0, func_00288FB8(0x2F40), -1);
    }
    if (D_001ABA68 < 6) {
        s0 = 0x80F0F0F0;
        if (!(D_00138340.w0 & 0x2000)) {
            s0 = s1;
        }
        func_00300A00(func_00336D50(D_001A8CFC + 0x8710, 0x4A), s0, 295.0f, 373.0f, 1.0f, D_001B23A8 * 0.8f, zf);
        func_0027F980(0x1C1, 0x171, 0x80F0F0F0, func_00288FB8(0x2F41), -1);
    }
    func_0027FAA8(D_001A7340 / 2, 0x1D, 0x80F0F0F0, func_00288FB8(0x3294), -1);
    func_0027FAA8(D_001A7340 / 2, 0x173, 0x80F0F0F0, func_00288FB8(0x2D4F), -1);
    func_0027C660();
    return 0;
}
