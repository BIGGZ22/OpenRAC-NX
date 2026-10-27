/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (c) 2026 the OpenRAC contributors */

/*
 * Ratchet & Clank (PAL): the game's small vector and number routines, in C.
 *
 * In the retail program these are hand-written assembly for the EE's vector
 * unit (VU0 macro instructions) and FPU, so the decompilation keeps them as
 * assembly: no C compiles to them. The port needs them in C all the same, and
 * they are the most called code in the game. Each function here was written
 * from the instructions of the routine it is named after and does what those
 * do to memory and to the result register, operation for operation and in the
 * same order, so that rounding agrees; the build's check runs both and
 * compares (runtime/port/README.md).
 *
 * What they do not reproduce is what the routines leave in the vector unit's
 * own registers. A routine whose result depends on what an earlier one left
 * there (the cross product stores a fourth field it never computed) is not
 * here.
 *
 * A vector is four floats, x first. The assembly loads all 128 bits of its
 * operands before it stores, so every function reads all it needs first: the
 * result may be one of the operands.
 */
#include "common.h"

/* The console's square root, and `a` over the square root of `b` as its one operation. */
float openrac_sqrt(float x);
float openrac_rsqrt(float a, float b);

/* How many game ticks one sixtieth of a second is (1.0 at 60 Hz, 1.2 at 50 Hz), read through $gp. */
extern float D_0015EE60;

/* The bits of a float, for a field that is copied and not computed. */
static __inline__ unsigned bits_of(float *v, int field) {
    return ((unsigned *)v)[field];
}

/* Stores three computed fields and the copied fourth. */
static __inline__ void put(float *out, float x, float y, float z, unsigned w) {
    out[0] = x;
    out[1] = y;
    out[2] = z;
    ((unsigned *)out)[3] = w;
}

/* out = a + b in x, y and z; w is a's. (vadd.xyz) */
void func_001F9BD8(float *out, float *a, float *b) {
    float x = a[0] + b[0];
    float y = a[1] + b[1];
    float z = a[2] + b[2];
    put(out, x, y, z, bits_of(a, 3));
}

/* out = a - b in x, y and z; w is a's. (vsub.xyz) */
void func_001F9BF0(float *out, float *a, float *b) {
    float x = a[0] - b[0];
    float y = a[1] - b[1];
    float z = a[2] - b[2];
    put(out, x, y, z, bits_of(a, 3));
}

/* out = a + (b - a) * t in x, y and z; w is a's. (vsub, vmulx, vadd) */
void func_001F9C08(float *out, float *a, float *b, float t) {
    float x = (b[0] - a[0]) * t;
    float y = (b[1] - a[1]) * t;
    float z = (b[2] - a[2]) * t;
    x = a[0] + x;
    y = a[1] + y;
    z = a[2] + z;
    put(out, x, y, z, bits_of(a, 3));
}

/* out = a * s in x, y and z; w is a's. (vmulx.xyz) */
void func_001F9C30(float *out, float *a, float s) {
    float x = a[0] * s;
    float y = a[1] * s;
    float z = a[2] * s;
    put(out, x, y, z, bits_of(a, 3));
}

/* The dot product of a and b over x, y and z: (x + y) + 1 * z, as the accumulator sums it. */
float func_001F9C78(float *a, float *b) {
    float x = a[0] * b[0];
    float y = a[1] * b[1];
    float z = a[2] * b[2];
    float sum = x + y;
    return sum + 1.0f * z;
}

/* The length of a over x, y and z. (vsqrt; the result is added to zero on its way out) */
float func_001F9CB8(float *a) {
    float x = a[0] * a[0];
    float y = a[1] * a[1];
    float z = a[2] * a[2];
    float sum = x + y;
    sum = sum + 1.0f * z;
    return 0.0f + openrac_sqrt(sum);
}

/* The length of a over x and y only (z is squared and not added). */
float func_001F9CE8(float *a) {
    float x = a[0] * a[0];
    float y = a[1] * a[1];
    return 0.0f + openrac_sqrt(x + y);
}

/* The distance from a to b over x, y and z. */
float func_001F9D10(float *a, float *b) {
    float x = a[0] - b[0];
    float y = a[1] - b[1];
    float z = a[2] - b[2];
    float sum;
    x = x * x;
    y = y * y;
    z = z * z;
    sum = x + y;
    sum = sum + 1.0f * z;
    return 0.0f + openrac_sqrt(sum);
}

/* The distance from a to b over x and y. */
float func_001F9D48(float *a, float *b) {
    float x = a[0] - b[0];
    float y = a[1] - b[1];
    x = x * x;
    y = y * y;
    return 0.0f + openrac_sqrt(x + y);
}

/*
 * out = a scaled to the length `length` in x, y and z; w is a's. A vector whose squared length
 * is exactly zero gives zeros. (vrsqrt: length over the root of the squared length, in one step)
 */
void func_001F9DC0(float *out, float *a, float length) {
    float x = a[0] * a[0];
    float y = a[1] * a[1];
    float z = a[2] * a[2];
    float sum = x + y;
    float scale;
    sum = sum + 1.0f * z;
    if (bits_of(&sum, 0) == 0) {
        put(out, 0.0f + 0.0f, 0.0f + 0.0f, 0.0f + 0.0f, bits_of(a, 3));
        return;
    }
    scale = openrac_rsqrt(length, sum);
    x = a[0] * scale;
    y = a[1] * scale;
    z = a[2] * scale;
    put(out, x, y, z, bits_of(a, 3));
}

/* The level programs' own copy of the routine above. */
void func_L00_001FF4B0(float *out, float *a, float length) {
    func_001F9DC0(out, a, length);
}

/* out = m * v for a matrix of four columns and all four fields of v. (vmulax, vmadday, vmaddaz, vmaddw) */
void func_001F9EE8(float *out, float *v, float *m) {
    float x = v[0], y = v[1], z = v[2], w = v[3];
    float r[4];
    int i;
    for (i = 0; i < 4; i++) {
        float sum = m[i] * x;
        sum = sum + m[4 + i] * y;
        sum = sum + m[8 + i] * z;
        r[i] = sum + m[12 + i] * w;
    }
    out[0] = r[0];
    out[1] = r[1];
    out[2] = r[2];
    out[3] = r[3];
}

/* out = the first three columns of m times v, plus (0, 0, 0, 1) times v's w. */
void func_001F9EC0(float *out, float *v, float *m) {
    static const float last[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    float x = v[0], y = v[1], z = v[2], w = v[3];
    float r[4];
    int i;
    for (i = 0; i < 4; i++) {
        float sum = m[i] * x;
        sum = sum + m[4 + i] * y;
        sum = sum + m[8 + i] * z;
        r[i] = sum + last[i] * w;
    }
    out[0] = r[0];
    out[1] = r[1];
    out[2] = r[2];
    out[3] = r[3];
}

/* The square root of x. */
float func_001F9B50(float x) {
    return 0.0f + openrac_sqrt(x);
}

/* Clears 16 bytes. (sq $0) */
void func_001F9BC0(unsigned *p) {
    p[0] = 0;
    p[1] = 0;
    p[2] = 0;
    p[3] = 0;
}

/* out = the four bytes of a colour as floats, the lowest byte first. (pextlb, pextlh, vitof0) */
void func_001F9F18(float *out, unsigned colour) {
    float x = (float)(int)(colour & 0xFF);
    float y = (float)(int)((colour >> 8) & 0xFF);
    float z = (float)(int)((colour >> 16) & 0xFF);
    float w = (float)(int)(colour >> 24);
    out[0] = x;
    out[1] = y;
    out[2] = z;
    out[3] = w;
}

/* Ticks to a count: n * the tick scale + 0.5, cut to an integer. (adda.s, madd.s, cvt.w.s) */
int func_001F9850(int n) {
    float half = 0.25f + 0.25f;
    return (int)(half + (float)n * D_0015EE60);
}

/* x cut to an integer. (cvt.w.s) */
int func_001FA898(float x) {
    return (int)x;
}

/* a + b brought back into (-pi, pi] by one turn. The assembly adds and subtracts pi twice. */
float func_001FA748(float a, float b) {
    float pi = 3.14159274f;
    float r = a + b;
    if (r < pi) {
        if (r < -pi) {
            r = r + pi;
            r = r + pi;
        }
    } else {
        r = r - pi;
        r = r - pi;
    }
    return r;
}

/* a - b brought back the same way. */
float func_001FA790(float a, float b) {
    float pi = 3.14159274f;
    float r = a - b;
    if (r < pi) {
        if (r < -pi) {
            r = r + pi;
            r = r + pi;
        }
    } else {
        r = r - pi;
        r = r - pi;
    }
    return r;
}

/*
 * Two colours mixed, byte by byte: each byte is from * (1 - t) + to * t, cut to an integer and
 * its low eight bits kept. The result fills the register's low 32 bits and leaves the rest zero.
 */
unsigned long func_001FA8A8(unsigned from, unsigned to, float t) {
    float keep = 1.0f - t;
    unsigned long mixed = 0;
    int i;
    for (i = 0; i < 4; i++) {
        float a = (float)(int)((from >> (8 * i)) & 0xFF);
        float b = (float)(int)((to >> (8 * i)) & 0xFF);
        float sum = a * keep;
        sum = sum + b * t;
        mixed |= (unsigned long)((unsigned)(int)sum & 0xFF) << (8 * i);
    }
    return mixed;
}

/* out = a * s in all four fields. (vmulx.xyzw) */
void func_001F9C48(float *out, float *a, float s) {
    float x = a[0] * s;
    float y = a[1] * s;
    float z = a[2] * s;
    float w = a[3] * s;
    out[0] = x;
    out[1] = y;
    out[2] = z;
    out[3] = w;
}

/* out = the four floats of v cut to integers, their low bytes packed into a colour. (vftoi0, ppach, ppacb) */
unsigned long func_001F9F30(float *v) {
    unsigned long colour = 0;
    int i;
    for (i = 0; i < 4; i++) {
        colour |= (unsigned long)((unsigned)(int)v[i] & 0xFF) << (8 * i);
    }
    return colour;
}

/* out = the three rows of m copied, and (0, 0, 0, 1) as the fourth. */
void func_001FA460(unsigned *out, unsigned *m) {
    unsigned rows[12];
    int i;
    for (i = 0; i < 12; i++) {
        rows[i] = m[i];
    }
    for (i = 0; i < 12; i++) {
        out[i] = rows[i];
    }
    out[12] = 0;
    out[13] = 0;
    out[14] = 0;
    ((float *)out)[15] = 1.0f;
}

/*
 * out = the upper three by three of m turned over its diagonal, each field added to zero on its
 * way, the fourth field of each row zero (one less one), and (0, 0, 0, 1) as the fourth row.
 */
void func_001FA4A0(float *out, float *m) {
    float r[12];
    int row, column;
    for (row = 0; row < 3; row++) {
        for (column = 0; column < 3; column++) {
            r[row * 4 + column] = 0.0f + m[column * 4 + row];
        }
        r[row * 4 + 3] = 1.0f - 1.0f;
    }
    out[12] = 0.0f;
    out[13] = 0.0f;
    out[14] = 0.0f;
    out[15] = 1.0f;
    for (row = 0; row < 12; row++) {
        out[row] = r[row];
    }
}

/*
 * out = the product of two rotations held as (x, y, z, w): the vector parts scaled by the other's
 * w and added, plus their cross product; w is the product of the w's less the dot product.
 */
void func_001FA588(float *out, float *a, float *b) {
    float ax = a[0], ay = a[1], az = a[2], aw = a[3];
    float bx = b[0], by = b[1], bz = b[2], bw = b[3];
    float ww = bw * aw;
    float dx = bx * ax, dy = by * ay, dz = bz * az;
    float sx = bx * aw, sy = by * aw, sz = bz * aw;
    float tx = ax * bw, ty = ay * bw, tz = az * bw;
    float cx = ay * bz, cy = az * bx, cz = ax * by;
    float dot;
    cx = cx - by * az;
    cy = cy - bz * ax;
    cz = cz - bx * ay;
    dot = dx + dy;
    dot = dot + 1.0f * dz;
    sx = sx + tx;
    sy = sy + ty;
    sz = sz + tz;
    out[0] = sx + cx;
    out[1] = sy + cy;
    out[2] = sz + cz;
    out[3] = ww - dot;
}

/* a less the whole multiples of b in it: the quotient's fraction times b. (vdiv, vftoi0, vitof0) */
float func_L00_00200210(float a, float b) {
    float quotient = 0.0f + a / b;
    float whole = (float)(int)quotient;
    return (quotient - whole) * b;
}

/*
 * out = a scaled to the length `length` over x and y; z and w are a's. A vector whose squared
 * length over x and y is exactly zero gets zeros there.
 */
void func_L00_001FF500(float *out, float *a, float length) {
    float x = a[0] * a[0];
    float y = a[1] * a[1];
    float sum = x + y;
    unsigned z = bits_of(a, 2);
    unsigned w = bits_of(a, 3);
    float scale;
    if (bits_of(&sum, 0) == 0) {
        x = 0.0f + 0.0f;
        y = 0.0f + 0.0f;
    } else {
        scale = openrac_rsqrt(length, sum);
        x = a[0] * scale;
        y = a[1] * scale;
    }
    out[0] = x;
    out[1] = y;
    ((unsigned *)out)[2] = z;
    ((unsigned *)out)[3] = w;
}
