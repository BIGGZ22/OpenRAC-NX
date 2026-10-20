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

extern void func_00282A60(void *p, s32 c, s32 n);
extern u8 *D_001C4FAC;
extern s32 D_001A9720[];

typedef struct { s32 w[16]; } Tab32;

void func_00297BB0(u8 *arg0)
{
    s32 buf[128];
    s32 tb[16];
    u8 *in;
    u8 *out;
    s32 s0;
    s32 k;
    s32 i;
    s32 q;

    out = arg0;
    in = D_001C4FAC;
    *(Tab32 *)tb = *(Tab32 *)D_001A9720;
    for (s0 = 0; s0 < 0x200; s0++) {
        if (s0 % 4 == 0) {
            func_00282A60(buf, 0, 0x200);
        }
        for (k = 0; k < 128; k += 2) {
            u8 b = *in++;
            buf[k] += tb[b & 0xF];
            buf[k + 1] += tb[b >> 4];
        }
        if (s0 % 4 == 3) {
            for (i = 0; i < 128; i++) {
                if (buf[i] < 8) {
                    buf[i] = 0;
                } else {
                    buf[i] = 1 << (i % 8);
                }
            }
            for (q = 0; q < 128; q += 8) {
                u8 *bp = (u8 *)&buf[q];
                s32 t1 = bp[0] | bp[4];
                s32 t2 = bp[8] | t1;
                s32 t3 = bp[12] | t2;
                s32 t4 = bp[16] | t3;
                s32 t5 = bp[20] | t4;
                s32 t6 = bp[24] | t5;
                s32 t7 = bp[28] | t6;
                *out++ = t7;
            }
        }
    }
    *arg0 &= 0xFE;
}
