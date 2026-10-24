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

/* The quadword an address lies in: lq, sq, lqc2 and sqc2 ignore the address's low four bits. */
static __inline__ void *quad(void *p) {
    return (void *)((unsigned)p & ~0xFu);
}

/* out = (0, 0, 1, 0): the unit vector $vf0 rotated one field to the left. (vmr32) */
void func_001F9BC8(float *out) {
    put(out, 0.0f, 0.0f, 1.0f, 0);
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

/*
 * One product of the vector unit's multiplier: a factor with exponent 0 is zero and makes the
 * product zero (with the sign of the two) whatever the other factor is, for the console's floats
 * have no infinity or NaN (an exponent of 255 is an ordinary number); the host's 0 * NaN is a NaN.
 * Such bit patterns sit in lanes no routine computes: the capsule query stores its push-out's w as
 * a mesh vertex word it loaded (0xFFFFFF01, a -255), the hero's position takes that quadword whole,
 * and func_L00_0024FF10 (HeroSyncMoby) multiplies (position - moby position) by a rotation whose
 * fourth row is zero: 0 there on the console, here a NaN in the hero's collision primitive, which
 * the camera's sphere query then hit (Oltanis: the camera, the stick's heading and the hero NaN).
 */
static __inline__ float vu_times(float a, float b) {
    union {
        float f;
        unsigned u;
    } p;
    unsigned ua = bits_of(&a, 0), ub = bits_of(&b, 0);
    if ((ua & 0x7F800000u) == 0 || (ub & 0x7F800000u) == 0) {
        p.u = (ua ^ ub) & 0x80000000u;
        return p.f;
    }
    return a * b;
}

/* out = m * v for a matrix of four columns and all four fields of v. (vmulax, vmadday, vmaddaz, vmaddw) */
void func_001F9EE8(float *out, float *v, float *m) {
    float x = v[0], y = v[1], z = v[2], w = v[3];
    float r[4];
    int i;
    for (i = 0; i < 4; i++) {
        float sum = vu_times(m[i], x);
        sum = sum + vu_times(m[4 + i], y);
        sum = sum + vu_times(m[8 + i], z);
        r[i] = sum + vu_times(m[12 + i], w);
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
        float sum = vu_times(m[i], x);
        sum = sum + vu_times(m[4 + i], y);
        sum = sum + vu_times(m[8 + i], z);
        r[i] = sum + vu_times(last[i], w);
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

/* Clears the 16 bytes of the quadword p lies in. (sq $0: the address's low four bits are ignored) */
void func_001F9BC0(unsigned *p_) {
    unsigned *p = quad(p_);
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

/* x cut to an integer. (cvt.w.s) The EE saturates what is out of range, 2^31 or more to 0x7FFFFFFF and
 * -2^31 or less to 0x80000000 (its floats have no infinity or NaN); C leaves that undefined, and x86
 * gives 0x80000000 for both. A host overflow, FLT_MAX for a divide by zero (openrac_fdiv) times a scale,
 * came out as the most negative number there: is_value_within_interpolated_window (func_00215B18) rounds
 * an animation step's length through here, which for the rate of a blend over 0 ticks is that. */
int func_001FA898(float x) {
    if (x >= 2147483648.0f) {
        return 0x7FFFFFFF;
    }
    if (x < -2147483648.0f) {
        return (int)0x80000000u;
    }
    if (x != x) {
        return 0x7FFFFFFF;  /* a host NaN, which no EE value is: taken as the largest */
    }
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
 * The vector unit's operations one by one (vsubx.w for 1 - t, vitof0, vmulaw into the
 * accumulator, vmaddx, vftoi0): the sum is cut to an integer, so a last place takes a whole unit
 * off or on, and the port computes this function in the console's float arithmetic (hostgen.json
 * "ps2_float"). In the host's, even rounding toward zero, 1 - t keeps bits of t the console's
 * adder drops: for a small t (a tween's first ticks) every channel came out one below the
 * starting colour (0x72ABEFF5 for 0x73ACF0F6), and about one call in a hundred with t = k/n.
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
void func_001FA588(void *out_, void *a_, void *b_) {
    float *out = out_, *a = a_, *b = b_;
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

/* --- Memory, timers and small numbers (written from the retail routines). --- */

/* The port's report of a trap the game's code raises on purpose. */
void openrac_game_trap(void);

/* log2dim: the index of the highest bit that differs from the sign bit (the floor of log2 for a
 * positive value), from the EE's count of leading sign bits. */
int func_001F9968(int x) {
    unsigned int u = x < 0 ? ~(unsigned int)x : (unsigned int)x;
    int same = 31;  /* leading bits equal to the sign bit, less one */
    while (u != 0) {
        u >>= 1;
        same--;
    }
    return 30 - same;
}

/* A trap the game raises on purpose (teq $0, $0): an assertion that failed. */
void func_001F9978(void) {
    openrac_game_trap();
}

/* A short busy wait, two counts a pass: nothing to wait for on the PC. */
void func_001F9988(int count) {
    (void)count;
}

/* FastMemSet: stores the word at least once, then until the byte count is used up. */
void func_001F99B0(int *dst, int value, int bytes) {
    do {
        *dst++ = value;
        bytes -= 4;
    } while (bytes > 0);
}

/* FastMemZero16: clears 16 bytes at least once, then until the byte count is used up. Each sq
 * stores the quadword the address lies in, so a destination off a 16-byte boundary is cleared from
 * the boundary below it (callers pass 8-aligned buffers too). */
void func_001F99D8(void *dst_, int bytes) {
    int *dst = quad(dst_);
    do {
        dst[0] = 0;
        dst[1] = 0;
        dst[2] = 0;
        dst[3] = 0;
        dst += 4;
        bytes -= 16;
    } while (bytes > 0);
}

/* FastMemCopy: copies 16 bytes at least once, then until the byte count is used up (lq and sq, so
 * both addresses are taken down to their quadword's). */
void func_001F9A98(void *to, void *from, int bytes) {
    int *dst = quad(to);
    int *src = quad(from);
    do {
        int a = src[0], b = src[1], c = src[2], d = src[3];
        dst[0] = a;
        dst[1] = b;
        dst[2] = c;
        dst[3] = d;
        src += 4;
        dst += 4;
        bytes -= 16;
    } while (bytes > 0);
}

/* FastMemOr16: dst = a | b, 16 bytes at a time, at least once (on quadwords, as FastMemCopy). */
void func_001F9AC0(int *dst_, int *a_, int *b_, int bytes) {
    int *dst = quad(dst_), *a = quad(a_), *b = quad(b_);
    do {
        int x = a[0] | b[0], y = a[1] | b[1], z = a[2] | b[2], w = a[3] | b[3];
        dst[0] = x;
        dst[1] = y;
        dst[2] = z;
        dst[3] = w;
        a += 4;
        b += 4;
        dst += 4;
        bytes -= 16;
    } while (bytes > 0);
}

/* FastDecTimer: 1 if the timer was already zero (nothing stored); otherwise it counts down from at
 * least 1 and the result is 0 while it stays above zero, 2 when it reaches it. */
int func_001F9908(int *timer) {
    int v = *timer;
    if (v == 0) {
        return 1;
    }
    if (v < 1) {
        v = 1;
    }
    v = v - 1;
    *timer = v;
    return v > 0 ? 0 : 2;
}

/* FastDecTimer for a 16-bit timer. */
int func_001F9938(short *timer) {
    int v = *timer;
    if (v == 0) {
        return 1;
    }
    if (v < 1) {
        v = 1;
    }
    v = v - 1;
    *timer = (short)v;
    return v > 0 ? 0 : 2;
}

/* Whether two spheres (centre in x, y, z, radius in w) overlap: 1 when the squared distance of
 * the centres is below the square of the summed radii. The sums are taken in the vector unit's
 * order: (dy^2 + dx^2) + dz^2, then minus (ra + rb)^2. */
int func_001F9D78(float *a, float *b) {
    float dx = a[0] - b[0];
    float dy = a[1] - b[1];
    float dz = a[2] - b[2];
    float r = a[3] + b[3];
    float d = dy * dy + dx * dx;
    d = d + dz * dz;
    d = d - r * r;
    return d < 0.0f ? 1 : 0;
}

/* The absolute value of an integer (negated when below zero; the most negative value stays). */
int func_001F9B70(int x) {
    if (x < 0) {
        x = -x;
    }
    return x;
}

/* The view the sphere tests below use (the camera's, written each frame): +0x00..+0x20 the
 * rotation's rows, +0x30 the camera position (w: a scale), +0x40..+0x70 the frustum's slopes and
 * planes. */
extern float D_0018D080[32];

/* A float's sign bit, as the routines test it (-0 counts as negative). */
static __inline__ int negative(float f) {
    return (*(unsigned *)&f >> 31) != 0;
}

/*
 * Where a sphere (centre p, radius r, scaled by the view's w) is against the view: -1 outside,
 * 0 across an edge, 1 inside. far is what the depth test leaves over (the radius past the
 * sphere's depth), which func_001FA9E8 turns into a fade.
 */
static int sphere_in_view(float *p, float r, float *far) {
    float *m = D_0018D080;
    float s = m[0x30 / 4 + 3];
    float x = p[0] * s - m[12];
    float y = p[1] * s - m[13];
    float z = p[2] * s - m[14];
    float w = p[3] * s;
    float vx = m[0] * x + m[4] * y + m[8] * z;
    float vy = m[1] * x + m[5] * y + m[9] * z;
    float vz = m[2] * x + m[6] * y + m[10] * z;
    float nearx = w + vz;
    float neary = -w + vz;
    float t3x = 0.0f - nearx;
    float t3y = r * s - neary;
    float e8x = m[0x60 / 4] * w;
    float e8y = m[0x60 / 4 + 1] * w;
    float e5x = m[0x40 / 4] * vz;
    float e5y = m[0x40 / 4 + 1] * vz;
    float ax = vx < 0.0f ? -vx : vx;
    float ay = vy < 0.0f ? -vy : vy;
    float e7x = e8x * m[0x60 / 4 + 2];
    float e7y = e8y * m[0x60 / 4 + 2];
    float e9x = e5x * m[0x50 / 4];
    float e9y = e5y * m[0x50 / 4 + 1];
    float e6x = ax - e8x;
    float e6y = ay - e8y;
    float o8x = ax + e7x;
    float o8y = ay + e7y;
    float e4x = nearx - m[0x70 / 4];
    float e4y = neary - m[0x70 / 4 + 1];
    float o7x = e5x - e6x;
    float o7y = e5y - e6y;
    float f8x = e9x - o8x;
    float f8y = e9y - o8y;

    *far = t3y;
    if (negative(t3y) || !negative(t3x)) {
        return -1;
    }
    if (negative(o7y) || negative(o7x)) {
        return -1;
    }
    if (negative(f8y) || negative(e4y) || negative(f8x) || !negative(e4x)) {
        return 0;
    }
    return 1;
}

/* Is the sphere at p (all four fields scaled by the view's w; w is its depth extent) with radius r
 * in view: -1 no, 0 partly, 1 wholly. */
int func_001FA8F0(float *p, float r) {
    float far;
    return sphere_in_view(p, r, &far);
}

/*
 * func_001FA8F0's test, and an alpha from it in *alpha: what is left of the radius past the
 * sphere's depth, as an integer, over 128 and at most 128 (0 when the sphere is out of view).
 */
int func_001FA9E8(float *p, int *alpha, float r) {
    float far;
    int in = sphere_in_view(p, r, &far);
    int v;
    unsigned a;

    if (in < 0) {
        *alpha = 0;
        return in;
    }
    v = far >= 2147483647.0f ? 0x7FFFFFFF : far <= -2147483648.0f ? (int)0x80000000 : (int)far;
    a = (unsigned)v >> 7;
    *alpha = a > 0x80 ? 0x80 : (int)a;
    return in;
}

/* out = v with x and y scaled to length len; (0, 0) when they are both zero. z and w are v's. */
void func_001F9E10(float *out, float *v, float len) {
    float x = v[0];
    float y = v[1];
    unsigned z = bits_of(v, 2);
    unsigned w = bits_of(v, 3);
    float sq = x * x + y * y;
    float q;

    if (*(unsigned *)&sq == 0) {
        out[0] = 0.0f;
        out[1] = 0.0f;
    } else {
        q = openrac_rsqrt(len, sq);
        out[0] = x * q;
        out[1] = y * q;
    }
    ((unsigned *)out)[2] = z;
    ((unsigned *)out)[3] = w;
}

/* The rotation of the quaternion q (x, y, z, w) as three matrix rows, the rows' w 0. (Two
 * products of q with 2q, then the usual sums, in the vector unit's order.) */
static void quat_rows(float *q, float *out) {
    const float x = q[0], y = q[1], z = q[2], w = q[3];
    const float x2 = x + x, y2 = y + y, z2 = z + z;
    const float wx = x2 * w, wy = y2 * w, wz = z2 * w;
    const float xx = x2 * x, xy = y2 * x, xz = z2 * x;
    const float yy = y2 * y, yz = z2 * y, zz = z2 * z;
    out[0] = 1.0f - yy - zz;
    out[1] = xy - wz;
    out[2] = xz + wy;
    out[3] = 0.0f;
    out[4] = wz + xy;
    out[5] = 1.0f - xx - zz;
    out[6] = yz - wx;
    out[7] = 0.0f;
    out[8] = xz - wy;
    out[9] = wx + yz;
    out[10] = 1.0f - xx - yy;
    out[11] = 0.0f;
}

/* The quaternion's rotation as three matrix rows; out's fourth row is left as it was. */
void func_001FA648(void *q_, void *out_) {
    quat_rows(q_, out_);
}

/*
 * The quaternion t of the way from a to b, the short way round (b negated when a and b point
 * apart), normalised: out = (a (1 - t) +- b t) / |a (1 - t) +- b t|. (VU0 macro mode in retail:
 * the sums go through the accumulator, y + x first; the way round is the sign bit of the dot
 * product, so -0 counts as apart; vrsqrt makes the scale in one step.)
 */
void func_001FA5C8(void *out_, void *a_, void *b_, float t) {
    float *out = out_, *a = a_, *b = b_;
    const float s = 1.0f - t;
    float p[4], q[4], v[4], d[4], sq[4];
    float dot, len, scale;
    int i;
    for (i = 0; i < 4; i++) {
        p[i] = a[i] * s;
        q[i] = b[i] * t;
    }
    for (i = 0; i < 4; i++) {
        v[i] = p[i] + q[i];
        d[i] = p[i] * q[i];
        sq[i] = v[i] * v[i];
    }
    dot = d[1] + d[0];
    dot = dot + 1.0f * d[2];
    dot = dot + 1.0f * d[3];
    if (negative(dot)) {
        for (i = 0; i < 4; i++) {
            v[i] = p[i] - q[i];
            sq[i] = v[i] * v[i];
        }
    }
    len = sq[0] + sq[1];
    len = len + 1.0f * sq[2];
    len = len + 1.0f * sq[3];
    scale = openrac_rsqrt(1.0f, len);
    for (i = 0; i < 4; i++) {
        out[i] = v[i] * scale;
    }
}

/* func_001FA648's rows, and (0, 0, 0, 1) as the fourth: its own copy of the routine, not a call. */
void func_001FA6C0(float *q, float *out) {
    quat_rows(q, out);
    out[12] = 0.0f;
    out[13] = 0.0f;
    out[14] = 0.0f;
    out[15] = 1.0f;
}
