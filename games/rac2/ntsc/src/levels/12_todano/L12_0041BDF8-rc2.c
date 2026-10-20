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
#ifndef NULL
#define NULL 0
#endif

extern void ref_L12_0041BDF8_00(void *);

void func_L12_0041BDF8(s8 *arg0) {
    s8 *s0;
    s8 **p;
    s32 a2;
    s8 *t;

    if (arg0 != NULL) {
        if (*(s16 *)(arg0 + 0xAA) == 0xFAE) {
            s0 = *(s8 **)(arg0 + 0x68);
            ref_L12_0041BDF8_00(arg0);
            p = (s8 **)(s0 + 8);
            a2 = 1;
            do {
                t = *p;
                a2 -= 1;
                if (t != NULL) {
                    *(s32 *)(t + 0x98) = *(s32 *)(*(s8 **)(t + 0x24) + 0x10);
                }
                p += 1;
            } while (a2 >= 0);
        }
    }
}
