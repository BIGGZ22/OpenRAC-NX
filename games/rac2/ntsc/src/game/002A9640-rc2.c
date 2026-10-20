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

extern s8 D_001BF200;

void *func_002A9640(void *arg0, u32 arg1, s32 arg2) {
    u8 idx;
    s8 *e;
    void *owner;

    idx = *(u8 *)((s8 *)arg0 + 0xA8);
    if (idx == 0xFF) {
        return 0;
    }
    e = (s8 *)&D_001BF200 + (idx << 6);
    owner = *(void **)(e + 0x38);
    if (owner != arg0) {
        return 0;
    }
    if (*(u32 *)(e + 0x24) & arg1) {
        return e;
    }
    if (arg2 == 0) {
        *(u8 *)((s8 *)owner + 0xA8) = 0xFF;
    }
    return 0;
}
