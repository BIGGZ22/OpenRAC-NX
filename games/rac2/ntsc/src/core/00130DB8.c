

typedef int s32;

typedef unsigned int u32;

/* Copied mechanically from Lombyte sdk/video/ipu/sce_ipu_sync.c; current source and actual
 * SDK-built full object/ref body pinned in donor-proof.json. Algorithm,
 * layout, signedness, expressions and source order preserved. Function
 * rename and duplicate EE scalar-type/include context removal only.
 * The donor SDK compiler b9aef69 differs; RAC2 acceptance requires its own qualification. */
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
u32 func_00130DB8(s32 arg0) {
    u32 result;

    result = 0;
    switch (arg0) {
    case 0:
        while (*(volatile s32 *)0x10002010 < 0) {
        }
        result = 0;
        break;
    case 1:
        result = *(volatile u32 *)0x10002010 >> 31;
        break;
    default:
        break;
    }
    return result;
}
