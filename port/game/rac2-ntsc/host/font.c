/* SPDX-License-Identifier: GPL-3.0-or-later
 * Going Commando's font prints, passing the font's TEX0 whole.
 *
 * 0x0027C878 (rac1's GetEffectTex, func_001F4868) returns a texture's whole
 * TEX0: TBP0, width and format in the low word, CBP and the palette's load
 * control in the high one (hostgen.json "returns_64"). The decompilation's
 * matching C keeps it in an int before handing it to the glyph builders,
 * which is right on the console (the register holds all 64 bits) but cuts
 * it to 32 here: the labels came out with no palette. These are the same
 * wrappers with the TEX0 as an unsigned long long, as rac1's port does for
 * FontPrintWindow (rac1/host/hand.c, func_001F7560 and func_001F75D0). The
 * builders take it so too (asm.signatures).
 */
#include <stdint.h>
#include <string.h>

#include "openrac/guest.h"

unsigned long long func_0027C878(int index);
int func_0027F178(gaddr text, int length);
int func_0027F198(gaddr text, int length);
int func_0027F1B8(gaddr text, int length);
void func_0027F2A8(int x, int y, int style, int text, int length, unsigned long long tex0, int table);
void func_0027FC70(int x, int y, int text, int length, unsigned long long tex0, int table);
void func_0027F640(float x, float y, float scale, int a, int b, int c, unsigned long long tex0, int d);
int func_00336A40(gaddr object);

/* The fonts' glyph tables: large, small and the third. */
#define FONT_LARGE 0x00263B50u
#define FONT_SMALL 0x00263EF0u
#define FONT_THIRD 0x00264290u

/* Print in the large font (texture 1) and the small one (texture 2). */
void func_0027F538(int x, int y, int style, gaddr text, int length) {
    func_0027F2A8(x, y, style, (int)text, length, func_0027C878(1), (int)FONT_LARGE);
}

void func_0027F5B8(int x, int y, int style, gaddr text, int length) {
    func_0027F2A8(x, y, style, (int)text, length, func_0027C878(2), (int)FONT_SMALL);
}

/* Right-aligned (x is the right edge). */
void func_0027F980(int x, int y, int style, gaddr text, int length) {
    x -= func_0027F178(text, length);
    func_0027F2A8(x, y, style, (int)text, length, func_0027C878(1), (int)FONT_LARGE);
}

void func_0027FA10(int x, int y, int style, gaddr text, int length) {
    x -= func_0027F198(text, length);
    func_0027F2A8(x, y, style, (int)text, length, func_0027C878(2), (int)FONT_SMALL);
}

/* Centred on x; the left edge is returned. */
int func_0027FAA8(int x, int y, int style, gaddr text, int length) {
    x -= func_0027F178(text, length) >> 1;
    func_0027F2A8(x, y, style, (int)text, length, func_0027C878(1), (int)FONT_LARGE);
    return x;
}

int func_0027FB40(int x, int y, int style, gaddr text, int length) {
    x -= func_0027F198(text, length) >> 1;
    func_0027F2A8(x, y, style, (int)text, length, func_0027C878(2), (int)FONT_SMALL);
    return x;
}

int func_0027FBD8(int x, int y, int style, gaddr text, int length) {
    x -= func_0027F1B8(text, length) >> 1;
    func_0027F2A8(x, y, style, (int)text, length, func_0027C878(3), (int)FONT_THIRD);
    return x;
}

/* The same through 0x0027FC70, which takes no style. */
void func_00280190(int x, int y, gaddr text, int length) {
    func_0027FC70(x, y, (int)text, length, func_0027C878(1), (int)FONT_LARGE);
}

void func_00280200(int x, int y, gaddr text, int length) {
    func_0027FC70(x, y, (int)text, length, func_0027C878(2), (int)FONT_SMALL);
}

void func_00280270(int x, int y, gaddr text, int length) {
    func_0027FC70(x, y, (int)text, length, func_0027C878(3), (int)FONT_THIRD);
}

static float f32_at(gaddr a) {
    float v;
    memcpy(&v, G(a), sizeof v);
    return v;
}

static int32_t s32_at(gaddr a) {
    int32_t v;
    memcpy(&v, G(a), sizeof v);
    return v;
}

/* The GUI's text element draw (0x00336A78, the decompilation's C with the
 * texture whole): nothing when its scale (through +16) or its text (+64) is
 * zero; the x position (through +0) moved left by half or all of the text's
 * width for the alignment at +68; then the string drawer with the y (through
 * +0, +4), the scale (through +4), the colour (through +12), the text, the
 * width, the texture of +56 and the word at +52. */
void func_00336A78(gaddr o) {
    const gaddr scale_at = (gaddr)s32_at(o + 16);
    if (f32_at(scale_at) == 0.0f || s32_at(o + 64) == 0) {
        return;
    }
    const int width = func_00336A40(o);
    const gaddr p0 = (gaddr)s32_at(o);
    const gaddr p1 = (gaddr)s32_at(o + 4);
    float x = f32_at(p0);
    switch (s32_at(o + 68)) {
        case 1:
            x = x - (float)(width >> 1);
            break;
        case 2:
            x = x - (float)width;
            break;
        default:
            break;
    }
    const int colour = s32_at((gaddr)s32_at(o + 12));
    const unsigned long long tex0 = func_0027C878(s32_at(o + 56));
    func_0027F640(x, f32_at(p0 + 4), f32_at(p1), colour, s32_at(o + 64), width, tex0, s32_at(o + 52));
}
