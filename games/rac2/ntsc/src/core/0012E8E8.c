

/*
MIT License

Copyright (c) 2026 Mateusz KÅ‚ysz

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
/* Isolated licensed source for a future independent SDK unit proof.
 * Existing SDK foundation scope does not qualify this 64-bit function.
 * Algorithm, original symbol, parameter types and layout are unchanged. */
typedef unsigned char u8;

typedef signed int s32;

typedef unsigned int u32;

typedef signed long long s64;

typedef struct SysbitStream {
    s64 bits;
    s32 field8;
    u8 *ptr;
    u32 cnt;
    u32 pad14;
    s64 total;
    u8 *base;
    u8 *end;
} SysbitStream;

s64 _sysbitFlush(SysbitStream *ctx, s32 amount) {
    s64 new_var;
    SysbitStream *new_var2;
    s64 bits = ctx->bits;
    s32 cnt = ctx->cnt - amount;

    new_var = bits << amount;
    ctx->bits = new_var;
    ctx->cnt = cnt;
    if ((u32)cnt < 0x39U) {
        do {
            ctx->bits |= (s64)(*(ctx->ptr++)) << (0x38 - ctx->cnt);
            new_var2 = ctx;
            if ((u32)new_var2->ptr >= (u32)new_var2->end) {
                if (ctx->cnt) {
                    ctx->ptr = ctx->base;
                } else {
                    ctx->ptr = ctx->base;
                }
            }
            ctx->cnt += 8;
        } while (ctx->cnt < 0x39U);
    }
    return ctx->total = ctx->total + amount;
}
