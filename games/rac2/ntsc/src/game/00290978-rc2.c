
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

extern void func_00288308();
extern s32 func_002908D0();
extern s32 func_00290A40(s32);
extern s32 func_002AD860();
extern volatile s32 D_001A7398;
extern volatile s32 D_001A73A0;
extern s32 D_001A79F0 __attribute__((sda));
extern volatile u8 D_001A7AFE;
extern volatile u8 D_001A7B16;
extern volatile u8 D_001A7B22;
extern volatile u8 D_001A7B27;
extern volatile s8 D_001A7B36;
extern volatile s8 D_001A7B68;
extern volatile s8 D_001A7B69;

void func_00290978(void) {
    if (D_001A7B16 == 0) {
        func_00288308(0x1E);
        func_002AD860(0x1E);
        D_001A7B68 = 0x5E;
        D_001A7398 = 0x1E;
    }
    if (D_001A7B22 == 0) {
        func_00288308(0x2A);
        func_002AD860(0x2A);
        D_001A7B69 = 0x6A;
    }
    if (D_001A7B27 == 0) {
        func_00288308(0x2F);
        D_001A73A0 = 0x2F;
    }
    if (D_001A7AFE == 0) {
        D_001A7B36 = 1;
        D_001A7AFE = 1;
    }
    func_002908D0();
    func_00290A40(D_001A79F0);
}
