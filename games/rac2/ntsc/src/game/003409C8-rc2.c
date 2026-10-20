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

extern s32 func_002E5E58(s32 a, s32 b, s32 c);
extern void func_003407F8(void *o, void *p, s32 n);
extern s16 D_00259F58[][5];

struct Obj {
    s8 pad[0x814];
    s32 f814;
    s32 f818;
    s32 f81C;
    s32 f820;
};

void func_003409C8(struct Obj *o, s32 flags) {
    if (flags & 0x1000) {
        s32 m1;
        s32 r;
        s32 t;
        func_002E5E58(3, 0, 0);
        m1 = -1;
        t = o->f818 - 1;
        r = (m1 < t) ? t : 5;
        o->f818 = r;
    } else if (flags & 0x4000) {
        s32 t;
        func_002E5E58(3, 0, 0);
        t = o->f818 + 1;
        t = (5 < t) ? 0 : t;
        o->f818 = t;
    } else if (flags & 0x8000) {
        s32 t;
        s32 a;
        s32 b;
        func_002E5E58(3, 0, 0);
        t = o->f814 - 1;
        o->f814 = t;
        if (t < 0) {
            a = o->f818;
            o->f814 = 3;
            b = a - 4;
            if ((u32)b < 2) {
                o->f814 = 2;
                o->f818 = b;
                o->f81C = 1;
            }
        }
    } else if (flags & 0x2000) {
        s32 t;
        s32 k;
        func_002E5E58(3, 0, 0);
        t = o->f814 + 1;
        o->f814 = t;
        if (t >= 4) {
            o->f814 = 0;
            k = o->f818;
            if (k == 4) {
                o->f818 = 0;
                o->f81C = 1;
            } else if (k == 5) {
                o->f814 = 1;
                o->f818 = 1;
                o->f81C = 1;
            }
        }
    } else if (flags & 0x40) {
        func_003407F8(o, D_00259F58, 4);
    }
    o->f820 = D_00259F58[o->f814 + o->f818 * 4][3];
}
