

/*
MIT License

Copyright (c) 2026 Mateusz Kłysz

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
typedef signed int s32;

typedef unsigned int u32;

struct IpuContext {
    s32 unk0; s32 unk4; s32 unk8; s32 unkC;
    s32 unk10; s32 unk14; s32 unk18; s32 unk1C;
};

extern s32 SetD3Chcr();

extern s32 SetD4Chcr();

void sceIpuRestartDMA(struct IpuContext *ctx) {
    u32 bits;
    u32 magic;
    u32 count;
    s32 src;
    s32 dst;

    bits = (u32)ctx->unk1C;
    magic = bits & 0x7F;
    count = ((bits >> 16) & 3) + ((bits >> 8) & 0xF);
    dst = ctx->unk0 - count * 0x10;
    src = ctx->unk8 + count;
    if (ctx->unk10 != 0) {
        if (ctx->unk14 != 0) {
            *(volatile u32 *)0x1000B010 = ctx->unk10;
            *(volatile u32 *)0x1000B020 = ctx->unk14;
            SetD3Chcr(ctx->unk18 | 0x100);
        }
    }
    while (*(volatile s32 *)0x10002010 < 0) {
    }
    *(volatile u32 *)0x10002000 = magic;
    while (*(volatile s32 *)0x10002010 < 0) {
    }
    if (dst != 0) {
        if (src != 0) {
            *(volatile u32 *)0x1000B410 = dst;
            *(volatile u32 *)0x1000B430 = ctx->unk4;
            *(volatile u32 *)0x1000B420 = src;
            SetD4Chcr(ctx->unkC | 0x100);
        }
    }
}
