/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (c) 2026 the OpenRAC contributors */

/*
 * Ratchet & Clank: the collision queries, the game's hand-written EE + VU0 kernels, as the port's
 * own C on the game's memory (hostgen.json, "host_functions"):
 *
 * | kernel | PAL | here |
 * |---|---|---|
 * | CollLine_Fix, segment query | level func_L00_001EFFF0 (and the pieces after it), boot func_001EFE10 | coll_line |
 * | sphere query | func_L00_001F10E0 | coll_volume (no height) |
 * | vertical capsule (the hero's body) | func_L00_001F1D20 | coll_volume (height) |
 * | sphere against mobys only | func_L00_001F2BE8 | func_L00_001F2BE8 |
 * | sphere against the hero-only groups | func_L00_001F34F0 | func_L00_001F34F0 |
 *
 * Adapted from ReRAC (crates/rc-game/src/collision_query.rs and collision_query/mobys.rs, its
 * docs/plan/collision_queries.md; ISC License, Copyright (c) 2026 ReRAC contributors), which ported
 * the same kernels of the US release operation by operation. What differs here: the kernels read the
 * level's collision tree, the moby grid, the moby records and their class collision blobs straight from
 * the game's memory (PAL addresses below, read from the PAL kernels), write CollOutput, the moby query
 * stamp (+0x9C), the hit records and coll_sphere_mobys's list as the game does, and do their float work
 * on the host's floats rounded toward zero (the VU's rounding; no model of its guard bits).
 *
 * The joint primitives of posed classes (kinds 2 and 4: a sphere on a joint, a capsule between two;
 * most creatures are made of them) take their joints from the pose the port's own evaluator gives the
 * moby now (openrac_game_moby_chain), where the kernels call func_L00_00254218 (hand-written, not in
 * the port) through the 8-entry cache at CollOutput +4/+8/+0xC. So a moby whose matrix is not rebuilt
 * is tested in its pose of now, not the cached one. The scratchpad copies the kernels leave behind are
 * not made.
 */
#include "rac1.h"

#include <fenv.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

#pragma STDC FENV_ACCESS ON

/* ---- Where each program keeps what the kernels read (from the PAL kernels' code) ---- */

typedef struct {
    gaddr out;   /* CollOutput: +0 the level's collision tree, +0x10 query stamp, +0x14 hit log index */
    gaddr mobys; /* the word holding the moby array's address */
    gaddr grid;  /* the moby grid: 64 x 64 cells {u16 block, s8 count, u8 capacity}, lists at +0x4000 */
    gaddr hits;  /* the moby hit log, 64 records of 0x40 */
    gaddr list;  /* coll_sphere_mobys's list of mobys */
    gaddr hero;  /* the word holding the hero-collision section's address */
} Ctx;

/* Level 0's program (the level kernels name D_L00_00173F40, gp-0x6C68, D_L00_0019BB60, D_L00_00178200,
 * D_L00_00178000 and gp-0x7608; $gp is 0x166D00); in another level, that level's copies of them
 * (OPENRAC_LDATA, guest.h). */
static const Ctx LEVEL0 = {RAC1_LADDR(00, 0x00173F40u), RAC1_LADDR(00, 0x00160098u), RAC1_LADDR(00, 0x0019BB60u), RAC1_LADDR(00, 0x00178200u), RAC1_LADDR(00, 0x00178000u), RAC1_LADDR(00, 0x0015F6F8u)};
/* The executable's own copy of the line query (D_00194200, gp-0x6CE8, D_001B7A60, D_001984C0). */
static const Ctx BOOT = {RAC1_ADDR(0x00194200u), RAC1_ADDR(0x00160018u), RAC1_ADDR(0x001B7A60u), RAC1_ADDR(0x001984C0u), 0, 0};

#define U8(a) GREF(uint8_t, (a))
#define S8(a) GREF(int8_t, (a))
#define U16(a) GREF(uint16_t, (a))
#define S16(a) GREF(int16_t, (a))
#define U32(a) GREF(uint32_t, (a))
#define S32(a) GREF(int32_t, (a))
#define F32(a) GREF(float, (a))

/* ---- The VU's arithmetic on host floats ---- */

static inline uint32_t fb(float f) {
    uint32_t u;
    memcpy(&u, &f, 4);
    return u;
}
/* The sign bit, which the kernels test (bltz after qmfc2): -0 counts as negative. */
static inline int neg(float f) { return (fb(f) >> 31) != 0; }
/* The bits as a signed integer above zero (bgtz on the word): positive and not +0. */
static inline int gtz(float f) { return (int32_t)fb(f) > 0; }
/* vmini / vmax compare sign and magnitude. */
static inline int32_t fkey(float f) {
    const uint32_t u = fb(f);
    return (u >> 31) ? -(int32_t)(u & 0x7FFFFFFFu) : (int32_t)u;
}
static inline float fmin2(float a, float b) { return fkey(b) < fkey(a) ? b : a; }
static inline float fmax2(float a, float b) { return fkey(b) > fkey(a) ? b : a; }
/* vdiv: a zero divisor gives the largest float of the quotient's sign (no infinities on the VU). */
static inline float fdiv(float a, float b) {
    if (b == 0.0f) {
        return (neg(a) != neg(b)) ? -FLT_MAX : FLT_MAX;
    }
    const float q = a / b;
    return isinf(q) ? (q < 0 ? -FLT_MAX : FLT_MAX) : q;
}
/* vsqrt takes the magnitude. */
static inline float fsqrt(float a) { return sqrtf(fabsf(a)); }
/* vftoi0: truncation toward zero, saturating. */
static inline int32_t ftoi0(float f) {
    if (f != f) {
        return 0;
    }
    if (f >= 2147483648.0f) {
        return INT32_MAX;
    }
    if (f <= -2147483648.0f) {
        return INT32_MIN;
    }
    return (int32_t)f;
}

typedef struct {
    float v[3];
} V3;

static inline V3 v3(float x, float y, float z) {
    V3 r = {{x, y, z}};
    return r;
}
static inline V3 vld(gaddr a) { return v3(F32(a), F32(a + 4), F32(a + 8)); }
static inline V3 vadd(V3 a, V3 b) { return v3(a.v[0] + b.v[0], a.v[1] + b.v[1], a.v[2] + b.v[2]); }
static inline V3 vsub(V3 a, V3 b) { return v3(a.v[0] - b.v[0], a.v[1] - b.v[1], a.v[2] - b.v[2]); }
static inline V3 vmul(V3 a, V3 b) { return v3(a.v[0] * b.v[0], a.v[1] * b.v[1], a.v[2] * b.v[2]); }
static inline V3 vmuls(V3 a, float s) { return v3(a.v[0] * s, a.v[1] * s, a.v[2] * s); }
/* a + b * s per lane, the product rounded first. */
static inline V3 vmadds(V3 a, V3 b, float s) {
    return v3(a.v[0] + b.v[0] * s, a.v[1] + b.v[1] * s, a.v[2] + b.v[2] * s);
}
/* vopmula / vopmsub: a x b, both products rounded before the subtraction. */
static inline V3 cross(V3 a, V3 b) {
    return v3(a.v[1] * b.v[2] - b.v[1] * a.v[2], a.v[2] * b.v[0] - b.v[2] * a.v[0],
              a.v[0] * b.v[1] - b.v[0] * a.v[1]);
}
/* (x + y) + z of the products. */
static inline float dot(V3 a, V3 b) {
    const V3 p = vmul(a, b);
    return (p.v[0] + p.v[1]) + p.v[2];
}
static inline V3 vzero(void) { return v3(0.0f, 0.0f, 0.0f); }

#define K1024 1024.0f
#define INV1024 (1.0f / 1024.0f)
/* 0x3F7FDF3B: the sphere and capsule "best distance" shrink. */
static float best_shrink(void) {
    const uint32_t u = 0x3F7FDF3Bu;
    float f;
    memcpy(&f, &u, 4);
    return f;
}

/* ---- The level's collision tree ---- */

/* The cell lookup (func_001EFD70 and its level copy): the leaf word of cell (x, y, z), 0 for none. */
static uint32_t cell_word(gaddr mesh, int x, int y, int z) {
    int i = z - U16(mesh);
    if (i < 0 || (int)U16(mesh + 2) - i <= 0) {
        return 0;
    }
    gaddr slab = (gaddr)U16(mesh + 4 + 2 * i) * 4;
    if (slab == 0) {
        return 0;
    }
    slab += mesh;
    i = y - U16(slab);
    if (i < 0 || (int)U16(slab + 2) - i <= 0) {
        return 0;
    }
    gaddr row = U32(slab + 4 + 4 * i);
    if (row == 0) {
        return 0;
    }
    row += mesh;
    i = x - U16(row);
    if (i < 0 || (int)U16(row + 2) - i <= 0) {
        return 0;
    }
    return U32(row + 4 + 4 * i);
}

typedef struct {
    gaddr leaf;
    int nv;    /* vertices (byte +2) */
    int nf;    /* faces walked: the low byte of the count (lbu) */
    int nq;    /* quads (byte +3) */
    gaddr qv3; /* the quads' fourth indices */
} Cell;

/* A cell to test: the leaf of a non-zero word with vertices. */
static int cell_open(gaddr mesh, uint32_t word, Cell* c) {
    if (word == 0) {
        return 0;
    }
    c->leaf = mesh + (word >> 8);
    c->nv = U8(c->leaf + 2);
    if (c->nv == 0) {
        return 0;
    }
    c->nf = U8(c->leaf);
    c->nq = U8(c->leaf + 3);
    c->qv3 = c->leaf + 4 + 4 * (gaddr)c->nv + 4 * (gaddr)U16(c->leaf);
    return 1;
}

typedef struct {
    V3 p;          /* offset from the cell centre, x1024 */
    uint32_t code; /* per axis: bit 0 below the query box, bit 1 above */
} Vert;

static Vert g_verts[2048];

/* The cell's vertices as halfwords (x << 6, y << 6, z << 4) and their outcodes against [lo, hi] with
 * 16-bit wrapping compares (psubh). */
static void decode_vertices(const Cell* c, const int16_t lo[3], const int16_t hi[3]) {
    for (int i = 0; i < c->nv; ++i) {
        const uint32_t w = U32(c->leaf + 4 + 4 * (gaddr)i);
        const int32_t f[3] = {((int32_t)(w << 22)) >> 22, ((int32_t)(w << 12)) >> 22, ((int32_t)w) >> 20};
        const int16_t h[3] = {(int16_t)(f[0] << 6), (int16_t)(f[1] << 6), (int16_t)(f[2] << 4)};
        uint32_t code = 0;
        for (int k = 0; k < 3; ++k) {
            const int below = (int16_t)(h[k] - lo[k]) < 0;
            const int above = (int16_t)(hi[k] - h[k]) < 0;
            code |= (uint32_t)(below | (above << 1)) << (8 * k);
        }
        g_verts[i].p = v3((float)h[0], (float)h[1], (float)h[2]);
        g_verts[i].code = code;
    }
}

/* Triangle j of the cell walk: the faces as (v0, v1, v2), then the first nq again as (v0, v2, v3). */
static int cell_triangle(const Cell* c, int j, int v[3], int* type) {
    const gaddr faces = c->leaf + 4 + 4 * (gaddr)c->nv;
    if (j < c->nf) {
        const gaddr f = faces + 4 * (gaddr)j;
        v[0] = U8(f);
        v[1] = U8(f + 1);
        v[2] = U8(f + 2);
        *type = U8(f + 3);
        return 1;
    }
    j -= c->nf;
    if (j < c->nq) {
        const gaddr f = faces + 4 * (gaddr)j;
        v[0] = U8(f);
        v[1] = U8(f + 2);
        v[2] = U8(c->qv3 + (gaddr)j);
        *type = U8(f + 3);
        return 1;
    }
    return 0;
}

/* A moby whose bounding sphere is not finite (its update produced a NaN, which the console's floats
 * cannot hold: they saturate) is in no query, rather than in every one (a NaN never compares as
 * outside), as the follow camera's line would otherwise be blocked by it everywhere. */
static int unbounded(gaddr m) {
    return !isfinite(F32(m)) || !isfinite(F32(m + 4)) || !isfinite(F32(m + 8)) || !isfinite(F32(m + 0xC));
}

/* Query flags 0x80 (type bit 7) and 0x20 (surface id == (flags >> 8) & 0x1F) leave a face out. */
static inline int face_excluded(uint32_t fl, int ty) {
    return (fl & 0x80 & (uint32_t)ty) != 0 || ((fl & 0x20) != 0 && (((fl >> 8) ^ (uint32_t)ty) & 0x1F) == 0);
}

/* ---- The running best hit ---- */

typedef struct {
    int have;
    int32_t kind; /* 0x1000 | face type, or -(primitive record address) */
    V3 point, normal, tri[3];
    gaddr moby;
} Best;

static void record(Best* b, int ty, V3 point, V3 normal, V3 v0, V3 e1, V3 e2, V3 centre, gaddr moby) {
    const V3 v0abs = vadd(v0, centre);
    b->have = 1;
    b->kind = 0x1000 + ty;
    b->point = vadd(point, v0abs);
    b->normal = normal;
    b->tri[0] = v0abs;
    b->tri[1] = vadd(e1, v0abs);
    b->tri[2] = vadd(e2, v0abs);
    b->moby = moby;
}

/* xyz of a vec4 in game memory (the kernels' sqc2 writes w from a lane the port does not keep). */
static void vst(gaddr a, V3 v) {
    F32(a) = v.v[0];
    F32(a + 4) = v.v[1];
    F32(a + 8) = v.v[2];
}

static void write_out(gaddr out, const Best* b, float k) {
    vst(out + 0x20, vmuls(b->point, k));
    vst(out + 0x40, b->normal);
    vst(out + 0x50, vmuls(b->tri[0], k));
    vst(out + 0x60, vmuls(b->tri[1], k));
    vst(out + 0x70, vmuls(b->tri[2], k));
    U32(out + 0x18) = b->moby;
    S32(out + 0x1C) = b->kind;
}

/* The stamp of a new query (CollOutput +0x10): -1 when it has run out (the kernel returns 0). */
static int64_t new_stamp(gaddr out) {
    const int32_t s = S32(out + 0x10);
    if (s < 0) {
        return -1;
    }
    S32(out + 0x10) = s + 1;
    return s + 1;
}

/* ---- Mobys ---- */

static inline gaddr moby_at(const Ctx* x, unsigned idx) { return U32(x->mobys) + idx * 0x100u; }

/* Cell (gx, gy) of the moby grid: its moby indices and how many. */
static gaddr grid_cell(const Ctx* x, unsigned gx, unsigned gy, int* count) {
    const gaddr cell = x->grid + gx * 4 + gy * 0x100;
    *count = S8(cell + 2);
    return x->grid + 0x4000 + (gaddr)U16(cell) * 32;
}

typedef struct {
    gaddr m, blob;
    V3 pos;     /* +0x10 x1024 */
    V3 rows[3]; /* +0xC0, +0xD0, +0xE0 */
    float s;    /* +0x2C */
    int nprims, nverts, nfaces;
    gaddr prims, verts, faces;
} MobyColl;

static void moby_coll(gaddr m, MobyColl* c) {
    c->m = m;
    c->blob = U32(m + 0x94);
    c->pos = vmuls(vld(m + 0x10), K1024);
    c->rows[0] = vld(m + 0xC0);
    c->rows[1] = vld(m + 0xD0);
    c->rows[2] = vld(m + 0xE0);
    c->s = F32(m + 0x2C);
    const int32_t pb = S32(c->blob + 4), fbytes = S32(c->blob + 8), vb = S32(c->blob + 0xC);
    c->nprims = pb / 0x20;
    c->nverts = vb / 8;
    c->nfaces = fbytes / 4;
    c->prims = c->blob + 0x10;
    c->verts = c->prims + (gaddr)pb;
    c->faces = c->verts + (gaddr)vb;
}

/* vmulax ACC, r0, v; vmadday ACC, r1, v; vmaddz d, r2, v. */
static inline V3 rot(const V3 r[3], V3 v) {
    V3 o;
    for (int l = 0; l < 3; ++l) {
        o.v[l] = (r[0].v[l] * v.v[0] + r[1].v[l] * v.v[1]) + r[2].v[l] * v.v[2];
    }
    return o;
}

/* The blob's mesh vertices relative to the moby's position, outcodes against [lo, hi] by sign. */
static int moby_mesh_verts(const MobyColl* c, V3 lo, V3 hi) {
    V3 rs[3];
    for (int r = 0; r < 3; ++r) {
        rs[r] = vmuls(c->rows[r], c->s);
    }
    const int n = c->nverts < 2048 ? c->nverts : 2048;
    for (int i = 0; i < n; ++i) {
        const gaddr v = c->verts + 8 * (gaddr)i;
        const V3 p = rot(rs, v3((float)S16(v), (float)S16(v + 2), (float)S16(v + 4)));
        uint32_t code = 0;
        for (int k = 0; k < 3; ++k) {
            const int below = neg(p.v[k] - lo.v[k]);
            const int above = neg(hi.v[k] - p.v[k]);
            code |= (uint32_t)(below | (above << 1)) << (8 * k);
        }
        g_verts[i].p = p;
        g_verts[i].code = code;
    }
    return n;
}

/* Face i of the blob, unless culled by its outcodes or the flags. */
static int moby_face(const MobyColl* c, int n, int i, uint32_t fl, Vert* v, int* ty) {
    const gaddr f = c->faces + 4 * (gaddr)i;
    const int a = U8(f), b = U8(f + 1), d = U8(f + 2);
    *ty = U8(f + 3);
    if (a >= n || b >= n || d >= n) {
        return 0;
    }
    v[0] = g_verts[a];
    v[1] = g_verts[b];
    v[2] = g_verts[d];
    return (v[0].code & v[1].code & v[2].code) == 0 && !face_excluded(fl, *ty);
}

/* Whether primitive i is one this query tests (moby +0x98 disables, mask bit 0 with flag 0x4, bit 1
 * without); *last is set on the record that ends the list (mask bit 15). */
static int prim_active(const MobyColl* c, int i, uint32_t fl, int* last) {
    const gaddr p = c->prims + 0x20 * (gaddr)i;
    const int16_t mask = S16(p + 2);
    *last = mask < 0;
    const uint32_t disable = U32(c->m + 0x98);
    const int off = i < 32 ? (int)((disable >> i) & 1) : 0;
    const int sel = (fl & 0x4) ? 1 : 2;
    return !off && (sel & mask) != 0;
}

static inline int joint_count(const MobyColl* c, uint32_t fl) { return U16(c->blob + ((fl & 0x4) ? 0 : 2)); }

/* The joints of the moby being tested, model units: the translation rows of its posed joint matrices
 * (pose_joints). */
#define JOINT_MAX 128
static V3 g_joints[JOINT_MAX];
static int g_joint_count;

/* Poses the first `count` joints of the moby (the blob's count for this kind of query): the port's
 * evaluator writes each marked joint's matrix to the scratchpad, 0x40 bytes a joint, which is put back
 * as it was. */
static void pose_joints(const MobyColl* c, uint32_t fl) {
    static gaddr marks;
    static uint8_t kept[JOINT_MAX * 0x40];
    int count = joint_count(c, fl);
    g_joint_count = 0;
    if (count <= 0) {
        return;
    }
    if (count > 0x7F) {
        count = 0x7F;
    }
    if (marks == 0) {
        marks = openrac_guest_static(0x80);
    }
    memset(G(marks), 0, 0x80);
    memset(G(marks), 1, (size_t)count);
    U8(marks + 0x7F) = (uint8_t)count;
    const gaddr spr = 0x70000000u;
    memcpy(kept, G(spr), (size_t)count * 0x40);
    memset(G(spr), 0, (size_t)count * 0x40);
    openrac_game_moby_chain(c->m, marks);
    for (int j = 0; j < count; ++j) {
        g_joints[j] = vld(spr + (gaddr)j * 0x40 + 0x30);
    }
    memcpy(G(spr), kept, (size_t)count * 0x40);
    g_joint_count = count;
}

static inline V3 joint_at(int j) { return j >= 0 && j < g_joint_count ? g_joints[j] : v3(0.0f, 0.0f, 0.0f); }

/* The sphere / capsule kernels' primitive point nearest the query point (a capsule's base) and its
 * radius: kind 1 a sphere, 2 a sphere on a joint, 3 a vertical cylinder, any other a capsule between
 * two joints. */
static int vol_prim(const MobyColl* c, gaddr p, V3 cq, int capsule, V3* cc, float* rad) {
    const int kind = S8(p);
    const float s = c->s;
    if (kind == 1) {
        const V3 q = vld(p + 0x10);
        *cc = vadd(rot(c->rows, vmuls(q, s)), c->pos);
        *rad = F32(p + 0x1C) * s;
        return 1;
    }
    if (kind == 2) {
        const V3 v = vadd(joint_at(S32(p + 4)), vld(p + 0x10));
        *cc = vadd(rot(c->rows, vmuls(v, s)), c->pos);
        *rad = F32(p + 0xC) * s;
        return 1;
    }
    if (kind == 3) {
        const V3 q = vld(p + 0x10);
        V3 o = vadd(rot(c->rows, vmuls(q, s)), c->pos);
        const float top = o.v[2] + F32(p + 4) * s;
        o.v[2] = fmin2(top, fmax2(o.v[2], cq.v[2]));
        *cc = o;
        *rad = F32(p + 0x1C) * s;
        return 1;
    }
    const V3 a = vadd(rot(c->rows, vmuls(joint_at(S16(p + 4)), s)), c->pos);
    const V3 b = vadd(rot(c->rows, vmuls(joint_at(S16(p + 6)), s)), c->pos);
    const V3 e = vsub(b, a);
    const V3 w = vsub(cq, a);
    *rad = F32(p + 0xC) * s;
    if (!capsule) {
        const float t = fmin2(fmax2(0.0f + fdiv(dot(w, e), dot(e, e)), 0.0f), 1.0f);
        *cc = vadd(a, vmuls(e, t));
        return 1;
    }
    /* A capsule query: the closest point in x/y, or the base's height clamped to the segment when
     * the segment is upright (|e.xy|^2 <= 1). */
    const float wd = w.v[0] * e.v[0] + w.v[1] * e.v[1];
    const float ee = e.v[0] * e.v[0] + e.v[1] * e.v[1];
    if (!neg(1.0f - fabsf(ee))) {
        V3 o = a;
        o.v[2] = fmin2(b.v[2], fmax2(a.v[2], cq.v[2]));
        *cc = o;
    } else {
        const float t = fmin2(fmax2(0.0f + fdiv(wd, ee), 0.0f), 1.0f);
        *cc = vmadds(a, e, t);
    }
    return 1;
}

/* The closest point of the primitive sphere (cc, rad) to c, its offset from cc and d^2, when r + rad
 * reaches c and d^2 <= best. */
static int vol_prim_test(V3 c, float r, V3 cc, float rad, float best, V3* pt, V3* off, float* d2out) {
    const float rr = r + rad;
    V3 e = vsub(cc, c);
    const float rr2 = rr * rr;
    const float d2 = dot(e, e);
    const float s = rr2 - d2;
    const float dist = 0.0f + fsqrt(d2);
    if (neg(s)) {
        return 0;
    }
    const float q = 0.0f + fdiv(1.0f, dist);
    const float k = fmin2(dist * 1.0f, rad);
    const V3 o = vmuls(vmuls(vsub(c, cc), q), k);
    const V3 p = vadd(cc, o);
    e = vsub(p, c);
    const float dd = dot(e, e);
    if (neg(best - dd)) {
        return 0;
    }
    *pt = p;
    *off = o;
    *d2out = dd;
    return 1;
}

static void prim_best(Best* b, gaddr prim, V3 point, V3 normal, gaddr moby) {
    b->have = 1;
    b->kind = -(int32_t)prim;
    b->point = point;
    b->normal = normal;
    b->moby = moby; /* the triangle (+0x50..+0x70) stays as the previous hit left it */
}

/* ---- Triangle tests ---- */

/* Line against one triangle: (t, P - v0, N, e1, e2) when the segment crosses it strictly before best. */
static int line_tri(uint32_t fl, V3 d, V3 a_rel, V3 b_rel, V3 v0, V3 v1, V3 v2, float best_t, float* tq,
                    V3* pp, V3* nn, V3* e1o, V3* e2o) {
    const V3 e1 = vsub(v1, v0);
    const V3 e2 = vsub(v2, v0);
    const V3 n = cross(e2, e1);
    const float dn = dot(d, n);
    const float s0 = dot(vsub(v0, a_rel), n);
    const float s1 = dot(vsub(v0, b_rel), n);
    const float q = fdiv(s0, dn);
    if ((fl & 0x10) == 0 && (!neg(s0) || !gtz(s1))) {
        return 0;
    }
    if (neg(s0) == neg(s1)) {
        return 0;
    }
    const V3 p = vmadds(vsub(a_rel, v0), d, q);
    const float c0 = dot(cross(p, e1), n);
    const float c1 = dot(cross(e2, p), n);
    const float c2 = dot(cross(vsub(p, e1), vsub(e2, e1)), n);
    const float margin = best_t - q;
    if (neg(c0) || neg(c1) || neg(c2) || !gtz(margin)) {
        return 0;
    }
    *tq = q;
    *pp = p;
    *nn = n;
    *e1o = e1;
    *e2o = e2;
    return 1;
}

/* Closest point on edge A -> B to p: t = ((p - A).(B - A)) / |B - A|^2 clamped to [0, 1]. */
static V3 edge_point(V3 p, V3 a, V3 b) {
    const V3 ba = vsub(b, a);
    float t = fdiv(dot(vsub(p, a), ba), dot(ba, ba));
    t = fmin2(fmax2(0.0f + t, 0.0f), 1.0f);
    return vmadds(a, ba, t);
}

/* The sphere / capsule triangle tail: p is the plane projection with squared plane distance d2; the
 * first failing edge test moves it to the closest point of that edge, measured again from `from`. */
static int closest_on_triangle(V3 p, float d2, V3 from, V3 e1, V3 e2, V3 n, float best, V3* out, float* d2o) {
    if (neg(best - d2)) {
        return 0;
    }
    const float c0 = dot(cross(p, e1), n);
    const float c1 = dot(cross(e2, p), n);
    const float c2 = dot(cross(vsub(p, e1), vsub(e2, e1)), n);
    V3 a, b;
    if (neg(c0)) {
        a = vzero();
        b = e1;
    } else if (neg(c1)) {
        a = e2;
        b = vzero();
    } else if (!neg(c2)) {
        *out = p;
        *d2o = d2;
        return 1;
    } else {
        a = e1;
        b = e2;
    }
    const V3 q = edge_point(p, a, b);
    const V3 diff = vsub(q, from);
    const float dd = dot(diff, diff);
    if (neg(best - dd)) {
        return 0;
    }
    *out = q;
    *d2o = dd;
    return 1;
}

/* Sphere against one triangle: closest point - v0, d^2, N, e1, e2 when d^2 <= best. */
static int sphere_tri(uint32_t fl, V3 c_rel, V3 v0, V3 v1, V3 v2, float best, V3* pt, float* d2o, V3* no,
                      V3* e1o, V3* e2o) {
    const V3 e1 = vsub(v1, v0);
    const V3 e2 = vsub(v2, v0);
    const V3 w = vsub(c_rel, v0);
    const V3 n = cross(e2, e1);
    const float s = dot(w, n);
    const float nn = dot(n, n);
    const float q = fdiv(s, nn);
    if ((fl & 0x10) == 0 && neg(s)) {
        return 0;
    }
    const V3 nq = vmuls(n, q);
    const V3 p = vmadds(w, vsub(vzero(), n), q);
    const V3 sq = vmul(nq, nq);
    const float d2 = (sq.v[0] + sq.v[1]) + sq.v[2];
    if (!closest_on_triangle(p, d2, w, e1, e2, n, best, pt, d2o)) {
        return 0;
    }
    *no = n;
    *e1o = e1;
    *e2o = e2;
    return 1;
}

/* Capsule (base c_rel, height h, x1024) against one triangle. */
static int capsule_tri(uint32_t fl, V3 c_rel, float h, V3 v0, V3 v1, V3 v2, float best, V3* pt, float* d2o,
                       V3* no, V3* e1o, V3* e2o) {
    const V3 e1 = vsub(v1, v0);
    const V3 e2 = vsub(v2, v0);
    const V3 bot = vsub(c_rel, v0);
    V3 top = bot;
    top.v[2] = top.v[2] + h;
    const V3 n = cross(e2, e1);
    const float sb = dot(bot, n);
    const float st = dot(top, n);
    if ((fl & 0x10) == 0 && neg(sb) && neg(st)) {
        return 0;
    }
    const float nn = dot(n, n);
    V3 axis, nq;
    float pz = 0.0f;
    int from_pz = 0;
    if ((int32_t)((fb(n.v[2]) & 0x7FFFFFFFu) - 0x3F800000u) < 0) {
        pz = 0.0f; /* a vertical face: the height of v0 */
        from_pz = 1;
    } else {
        const float q = fdiv(sb, h * n.v[2]);
        const float b1z = bot.v[2] - e1.v[2];
        const float b2z = bot.v[2] - e2.v[2];
        const float t1z = top.v[2] - e1.v[2];
        const float t2z = top.v[2] - e2.v[2];
        if (!neg(bot.v[2]) && !neg(b1z) && !neg(b2z)) {
            axis = bot;
            nq = vmuls(n, fdiv(sb, nn));
        } else if (neg(top.v[2]) && neg(t1z) && neg(t2z)) {
            axis = top;
            nq = vmuls(n, fdiv(st, nn));
        } else {
            /* The axis meets the plane at X = (base.xy, base.z - h Q). */
            const V3 x = v3(bot.v[0], bot.v[1], bot.v[2] - h * q);
            const float c0 = dot(cross(x, e1), n);
            const float c1 = dot(cross(e2, x), n);
            const float c2 = dot(cross(vsub(x, e1), vsub(e2, e1)), n);
            V3 p;
            if (neg(c0)) {
                p = edge_point(x, vzero(), e1);
            } else if (neg(c1)) {
                p = edge_point(x, e2, vzero());
            } else if (neg(c2)) {
                p = edge_point(x, e1, e2);
            } else {
                p = x;
            }
            pz = p.v[2];
            from_pz = 1;
        }
    }
    if (from_pz) {
        const float k = fmax2(fmin2(pz - bot.v[2], h), 0.0f);
        axis = v3(bot.v[0], bot.v[1], k + bot.v[2]);
        nq = vmuls(n, fdiv(dot(axis, n), nn));
    }
    const V3 sq = vmul(nq, nq);
    const float d2 = (sq.v[0] + sq.v[1]) + sq.v[2];
    const V3 p = vsub(axis, nq);
    if (!closest_on_triangle(p, d2, axis, e1, e2, n, best, pt, d2o)) {
        return 0;
    }
    *no = n;
    *e1o = e1;
    *e2o = e2;
    return 1;
}

/* ---- CollLine_Fix ---- */

typedef struct {
    float t;     /* entry parameter */
    int c[3];    /* cell */
} LineCell;

#define DDA_MAX 256
static LineCell g_list[3 * DDA_MAX + 2];

/* `vmini`/`vmax` of the endpoints and the [0, 1024)^3 test from the MAC sign flags. */
static int in_world(V3 a, V3 b) {
    for (int k = 0; k < 3; ++k) {
        if (neg(fmin2(a.v[k], b.v[k]) - 0.0f) || !neg(fmax2(a.v[k], b.v[k]) - K1024)) {
            return 0;
        }
    }
    return 1;
}

static inline float fbits(uint32_t u) {
    float f;
    memcpy(&f, &u, 4);
    return f;
}

/* The DDA cell list: entries (t, cell) in walk order, without the 1.0 terminator; -1 for a zero-length
 * segment (the query then returns no hit and skips the moby pass). */
static int line_cell_list(V3 a1024, V3 b1024) {
    int ia[3], ib[3], s[3], e[3];
    for (int k = 0; k < 3; ++k) {
        ia[k] = ftoi0(a1024.v[k]);
        ib[k] = ftoi0(b1024.v[k]);
        s[k] = (int)((uint32_t)ia[k] >> 12);
        e[k] = (int)((uint32_t)ib[k] >> 12);
    }
    g_list[0].t = 0.0f;
    memcpy(g_list[0].c, s, sizeof s);
    if (s[0] == e[0] && s[1] == e[1] && s[2] == e[2]) {
        if (ia[0] == ib[0] && ia[1] == ib[1] && ia[2] == ib[2]) {
            return -1;
        }
        return 1;
    }
    static int32_t lists[3][DDA_MAX + 1];
    int len[3] = {0, 0, 0}, dir[3];
    const V3 d = vsub(b1024, a1024);
    for (int k = 0; k < 3; ++k) {
        const float q = fdiv(1.0f, d.v[k]);
        int at, end, step;
        if (e[k] - s[k] >= 0) {
            at = s[k];
            end = e[k];
            step = 1;
        } else {
            at = s[k] + 1;
            end = e[k] + 1;
            step = -1;
        }
        dir[k] = step;
        while (at != end && len[k] < DDA_MAX) {
            at += step;
            const float t = ((float)(at << 12) - a1024.v[k]) * q;
            int32_t bits = (int32_t)fb(t);
            bits = bits < 0x3F800000 ? bits : 0x3F800000;
            bits = bits > 0 ? bits : 0;
            lists[k][len[k]++] = bits;
        }
    }
    const int total = len[0] + len[1] + len[2];
    int n = 1, idx[3] = {0, 0, 0};
    int cell[3] = {s[0], s[1], s[2]};
    for (int i = 0; i < total; ++i) {
        int32_t v[3];
        for (int k = 0; k < 3; ++k) {
            v[k] = idx[k] < len[k] ? lists[k][idx[k]] : 0x50000000;
        }
        const int k = (v[1] - v[0] < 0) ? ((v[2] - v[1] < 0) ? 2 : 1) : ((v[2] - v[0] < 0) ? 2 : 0);
        cell[k] += dir[k];
        g_list[n].t = fbits((uint32_t)v[k]);
        memcpy(g_list[n].c, cell, sizeof cell);
        ++n;
        idx[k] += 1;
    }
    return n;
}

static void line_world(gaddr mesh, uint32_t fl, V3 a1024, V3 b1024, V3 d, int n, float* best_t, Best* best) {
    for (int i = 0; i < n; ++i) {
        const float t_enter = g_list[i].t;
        const float t_next = i + 1 < n ? g_list[i + 1].t : 1.0f;
        if (fb(t_enter) == 0x3F800000u) {
            break;
        }
        if (best->have && (int32_t)fb(*best_t) - (int32_t)fb(t_enter) <= 0) {
            break;
        }
        const int* c = g_list[i].c;
        Cell cell;
        if (!cell_open(mesh, cell_word(mesh, c[0], c[1], c[2]), &cell)) {
            continue;
        }
        int ci[3];
        int16_t lo[3], hi[3];
        const V3 pe = vmadds(a1024, d, t_enter);
        const V3 pn = vmadds(a1024, d, t_next);
        for (int k = 0; k < 3; ++k) {
            ci[k] = (c[k] << 12) + 2048;
            const int32_t ee = (int32_t)((uint32_t)ftoi0(pe.v[k]) - (uint32_t)ci[k]);
            const int32_t nx = (int32_t)((uint32_t)ftoi0(pn.v[k]) - (uint32_t)ci[k]);
            lo[k] = (int16_t)((ee < nx ? ee : nx) - 1);
            hi[k] = (int16_t)((ee > nx ? ee : nx) + 1);
        }
        const V3 centre = v3((float)ci[0], (float)ci[1], (float)ci[2]);
        const V3 a_rel = vsub(a1024, centre);
        const V3 b_rel = vsub(b1024, centre);
        decode_vertices(&cell, lo, hi);
        int vi[3], ty;
        for (int j = 0; cell_triangle(&cell, j, vi, &ty); ++j) {
            const Vert* v0 = &g_verts[vi[0]];
            const Vert* v1 = &g_verts[vi[1]];
            const Vert* v2 = &g_verts[vi[2]];
            if ((v0->code & v1->code & v2->code) != 0 || face_excluded(fl, ty)) {
                continue;
            }
            float q;
            V3 p, nrm, e1, e2;
            if (!line_tri(fl, d, a_rel, b_rel, v0->p, v1->p, v2->p, *best_t, &q, &p, &nrm, &e1, &e2)) {
                continue;
            }
            *best_t = q;
            record(best, ty, p, nrm, v0->p, e1, e2, centre, 0);
        }
    }
}

/* The segment's entry into a sphere (or, in x/y, a cylinder), clamped to [0, 1]. */
static float entry_root(float wd, float ww, float dd, float rr) {
    const float k = wd + wd;
    const float wr = ww - rr;
    const float disc = fmax2(k * k - (4.0f * dd) * wr, 0.0f);
    const float root = fsqrt(disc);
    const float q = fdiv((0.0f - k) + root, dd + dd);
    return fmin2(fmax2(0.0f - q, 0.0f), 1.0f);
}

/* One primitive against the segment: (t, hit, hit - C). */
static int line_prim(const MobyColl* c, gaddr p, V3 a, V3 b, float best_t, float* to, V3* hit, V3* nrm) {
    const int kind = S8(p);
    const float s = c->s;
    V3 cc;
    float rad;
    if (kind == 1) {
        cc = vadd(rot(c->rows, vmuls(vld(p + 0x10), s)), c->pos);
        rad = F32(p + 0x1C) * s;
    } else if (kind == 3) {
        cc = vadd(rot(c->rows, vmuls(vld(p + 0x10), s)), c->pos);
        rad = F32(p + 0x1C) * s;
        const V3 d2 = vsub(b, a);
        const V3 w = vsub(cc, a);
        const float dd = fmax2(d2.v[0] * d2.v[0] + d2.v[1] * d2.v[1], fbits(0x3C800000u));
        const float wd = w.v[0] * d2.v[0] + w.v[1] * d2.v[1];
        const float ww = w.v[0] * w.v[0] + w.v[1] * w.v[1];
        const float q = fdiv(wd, dd);
        const float rr = rad * rad;
        const float t = fmin2(fmax2(0.0f + q, 0.0f), 1.0f);
        const float ex = (a.v[0] + d2.v[0] * t) - cc.v[0];
        const float ey = (a.v[1] + d2.v[1] * t) - cc.v[1];
        const float v = (ex * ex - rr) + ey * ey;
        if (gtz(v)) {
            return 0;
        }
        const float te = entry_root(wd, ww, dd, rr);
        const V3 p6 = vmadds(a, d2, te);
        const float top = cc.v[2] + F32(p + 4) * s;
        cc.v[2] = fmin2(fmax2(cc.v[2], p6.v[2]), top);
    } else if (kind == 2) {
        const V3 v = vadd(joint_at(S32(p + 4)), vld(p + 0x10));
        cc = vadd(rot(c->rows, vmuls(v, s)), c->pos);
        rad = F32(p + 0xC) * s;
    } else {
        /* A capsule between two joints: where the segment enters the capsule's infinite axis line (the
         * segment's start when they are parallel), brought back onto the axis between the joints. */
        const V3 a0 = rot(c->rows, vmuls(joint_at(S16(p + 4)), s));
        const V3 b0 = rot(c->rows, vmuls(joint_at(S16(p + 6)), s));
        rad = F32(p + 0xC) * s;
        const float rr = rad * rad;
        const V3 d = vsub(b, a);
        const V3 e = vsub(b0, a0);
        const V3 pa = vadd(a0, c->pos);
        const V3 n3 = cross(e, d);
        const V3 n4 = cross(d, e);
        const V3 n3sq = vmul(n3, n3);
        const V3 n7 = cross(d, n3);
        const V3 n8 = cross(e, n4);
        const int parallel = neg(n3sq.v[0] - 1.0f) && neg(n3sq.v[1] - 1.0f) && neg(n3sq.v[2] - 1.0f);
        V3 x = a;
        if (!parallel) {
            const float nn = (n3sq.v[0] + n3sq.v[1]) + n3sq.v[2];
            const float s7 = dot(n7, vsub(a, pa));
            const float s8 = dot(n8, vsub(pa, a));
            const V3 c3 = vmadds(pa, e, fdiv(s7, nn)); /* the axis line's point nearest the segment's line */
            const V3 c4 = vmadds(a, d, fdiv(s8, nn));  /* the segment's line's point nearest the axis */
            const V3 g = vsub(c4, c3);
            const V3 gs = vmul(g, g);
            const float v = (((0.0f - rr) + gs.v[0]) + gs.v[1]) + gs.v[2];
            if (!neg(v)) {
                return 0;
            }
            const V3 eh = vmuls(e, fdiv(1.0f, fsqrt(dot(e, e))));
            const V3 dh = vmuls(d, fdiv(1.0f, fsqrt(dot(d, d))));
            const float cs = dot(eh, dh);
            const float k = fmax2(0.0f + fdiv(0.0f - v, 1.0f - cs * cs), 0.0f);
            const V3 entry = vsub(c4, vmuls(dh, fsqrt(k)));
            if (!neg(dot(vsub(entry, a), d))) {
                x = entry;
            }
        }
        const float t = fmin2(fmax2(0.0f + fdiv(dot(vsub(x, pa), e), dot(e, e)), 0.0f), 1.0f);
        cc = vadd(pa, vmuls(e, t));
    }
    const V3 d2 = vsub(b, a);
    const V3 w = vsub(cc, a);
    const float dd = dot(d2, d2);
    const float wd = dot(w, d2);
    const float ww = dot(w, w);
    const float rr = rad * rad;
    const float t0 = fmin2(fmax2(0.0f + fdiv(wd, dd), 0.0f), 1.0f);
    const V3 e = vsub(vmadds(a, d2, t0), cc);
    const V3 sq = vmul(e, e);
    const float v = ((sq.v[0] - rr) + sq.v[1]) + sq.v[2];
    if (gtz(v)) {
        return 0;
    }
    const float t = entry_root(wd, ww, dd, rr);
    const V3 h = vmadds(a, d2, t);
    if (!gtz(best_t - t)) {
        return 0;
    }
    *to = t;
    *hit = h;
    *nrm = vsub(h, cc);
    return 1;
}

/* The moby pass of CollLine_Fix: the grid cells under the DDA list, (x >> 2, y >> 2) without repeats. */
static void line_mobys(const Ctx* x, uint32_t stamp, uint32_t fl, V3 a, V3 b, V3 d, int n, gaddr ignore,
                       float* best_t, Best* best) {
    static unsigned cells[3 * DDA_MAX + 2][2];
    int nc = 0;
    for (int i = 0; i < n; ++i) {
        if (i > 0 && fb(g_list[i].t) == 0x3F800000u) {
            break;
        }
        const unsigned gx = (unsigned)(g_list[i].c[0] & 0xFF) >> 2, gy = (unsigned)(g_list[i].c[1] & 0xFF) >> 2;
        if (nc == 0 || cells[nc - 1][0] != gx || cells[nc - 1][1] != gy) {
            cells[nc][0] = gx;
            cells[nc][1] = gy;
            ++nc;
        }
    }
    const V3 dn = vmuls(d, fdiv(1.0f, dot(d, d)));
    for (int ci = 0; ci < nc; ++ci) {
        int count;
        const gaddr list = grid_cell(x, cells[ci][0], cells[ci][1], &count);
        for (int li = 0; li < count; ++li) {
            const gaddr m = moby_at(x, U16(list + 2 * (gaddr)li));
            if (m == ignore) {
                continue;
            }
            if ((fl & 0x9) != 0 && (U16(m + 0x34) & 0x4000) == 0) {
                continue;
            }
            const V3 bc = vld(m);
            const float br = F32(m + 0xC);
            const float t = fmin2(fmax2(dot(vsub(bc, a), dn), 0.0f), *best_t);
            const V3 e = vsub(vmadds(a, d, t), bc);
            const V3 sq = vmul(e, e);
            const float v = ((sq.v[0] - br * br) + sq.v[1]) + sq.v[2];
            if (U32(m + 0x94) == 0 || gtz(v) || U32(m + 0x9C) == stamp || unbounded(m)) {
                continue;
            }
            U32(m + 0x9C) = stamp;
            MobyColl c;
            moby_coll(m, &c);
            if (joint_count(&c, fl) != 0 && (fl & 0x2) != 0 && c.nverts == 0) {
                continue;
            }
            pose_joints(&c, fl);
            if (c.nverts != 0) {
                const V3 a_rel = vsub(a, c.pos);
                const V3 b_rel = vsub(b, c.pos);
                V3 lo, hi;
                for (int k = 0; k < 3; ++k) {
                    lo.v[k] = fmin2(a_rel.v[k], b_rel.v[k]);
                    hi.v[k] = fmax2(a_rel.v[k], b_rel.v[k]);
                }
                const int nv = moby_mesh_verts(&c, lo, hi);
                for (int fi = 0; fi < c.nfaces; ++fi) {
                    Vert v[3];
                    int ty;
                    if (!moby_face(&c, nv, fi, fl, v, &ty)) {
                        continue;
                    }
                    float q;
                    V3 p, nrm, e1, e2;
                    if (!line_tri(fl, d, a_rel, b_rel, v[0].p, v[1].p, v[2].p, *best_t, &q, &p, &nrm, &e1, &e2)) {
                        continue;
                    }
                    *best_t = q;
                    record(best, ty, p, nrm, v[0].p, e1, e2, c.pos, m);
                }
            }
            if (fl & 0x2) {
                continue;
            }
            for (int i = 0; i < c.nprims; ++i) {
                int last;
                const int on = prim_active(&c, i, fl, &last);
                if (on) {
                    const gaddr p = c.prims + 0x20 * (gaddr)i;
                    float t2;
                    V3 hit, nrm;
                    if (line_prim(&c, p, a, b, *best_t, &t2, &hit, &nrm)) {
                        *best_t = 0.0f + t2;
                        prim_best(best, p, hit, nrm, m);
                    }
                }
                if (last) {
                    break;
                }
            }
        }
    }
}

/* A hit record for the moby a line with a template hit (moby +0xA4 indexes the record), unless the
 * record it has for it already carries more damage. */
static void hit_record(const Ctx* x, gaddr moby, gaddr tmpl, V3 point, int32_t index, float w) {
    const int8_t old = S8(moby + 0xA4);
    if (old >= 0) {
        const gaddr r = x->hits + (gaddr)old * 64;
        if (U32(r + 0x34) == moby && (int32_t)(U32(r + 0x2C) - U32(tmpl + 0x1C)) > 0) {
            return;
        }
    }
    const uint32_t i = U32(x->out + 0x14);
    U32(x->out + 0x14) = (i + 1) & 0x3F;
    S8(moby + 0xA4) = (int8_t)i;
    const gaddr r = x->hits + (i & 0x3F) * 64; /* the kernel does not mask the index it stores at */
    vst(r, point);
    F32(r + 0xC) = w;
    memmove(G(r + 0x10), G(tmpl), 0x24);
    U32(r + 0x34) = moby;
    S32(r + 0x38) = index;
}

static int coll_line(const Ctx* x, gaddr pa, gaddr pb, uint32_t fl, gaddr ignore, gaddr tmpl) {
    const int64_t stamp = new_stamp(x->out);
    if (stamp < 0) {
        return 0;
    }
    const V3 a = vld(pa), b = vld(pb);
    if (!in_world(a, b)) {
        return 0;
    }
    const V3 a1024 = vmuls(a, K1024);
    const V3 b1024 = vmuls(b, K1024);
    const int n = line_cell_list(a1024, b1024);
    if (n < 0) {
        return 0;
    }
    const V3 d = vsub(b1024, a1024);
    float best_t = 1.0f;
    Best best;
    memset(&best, 0, sizeof best);
    /* A primitive hit keeps the triangle registers as they were: the previous hit's, or here what
     * CollOutput holds. */
    for (int k = 0; k < 3; ++k) {
        best.tri[k] = vmuls(vld(x->out + 0x50 + 0x10 * (gaddr)k), K1024);
    }
    if ((fl & 0x1) == 0) {
        line_world(U32(x->out), fl, a1024, b1024, d, n, &best_t, &best);
    }
    line_mobys(x, (uint32_t)stamp, fl, a1024, b1024, d, n, ignore, &best_t, &best);
    if (!best.have) {
        return 0;
    }
    write_out(x->out, &best, INV1024);
    if (best.moby != 0 && tmpl != 0 && (U16(best.moby + 0x34) & 0x4000) != 0) {
        const int32_t index =
            best.kind < 0 ? (int32_t)(((uint32_t)(-best.kind) - U32(best.moby + 0x94)) >> 5) : -1;
        hit_record(x, best.moby, tmpl, vmuls(best.point, INV1024), index, 0.0f);
    }
    return 1;
}

/* ---- Sphere and capsule ---- */

typedef struct {
    V3 c1024, lo, hi, lo1024, hi1024;
    float r1024, h1024, rr;
    int capsule;
} Volume;

static int volume_new(Volume* v, gaddr centre, float radius, float height, int capsule) {
    const V3 c = vld(centre);
    v->capsule = capsule;
    for (int k = 0; k < 3; ++k) {
        v->lo.v[k] = c.v[k] - radius;
        v->hi.v[k] = c.v[k] + radius;
    }
    if (capsule) {
        v->hi.v[2] = v->hi.v[2] + height;
    }
    v->r1024 = radius * K1024;
    for (int k = 0; k < 3; ++k) {
        if (neg(v->lo.v[k] - 0.0f)) {
            return 0;
        }
    }
    if (!gtz(radius)) {
        return 0;
    }
    for (int k = 0; k < 3; ++k) {
        if (!neg(v->hi.v[k] - K1024)) {
            return 0;
        }
    }
    if (capsule && !gtz(height)) {
        return 0;
    }
    v->c1024 = vmuls(c, K1024);
    v->lo1024 = vmuls(v->lo, K1024);
    v->hi1024 = vmuls(v->hi, K1024);
    v->h1024 = capsule ? height * K1024 : 0.0f;
    v->rr = v->r1024 * v->r1024;
    return 1;
}

/* The push-out: Q = r / sqrt(best); the centre (capsule: base) moved to touch the hit. */
static V3 volume_pushed(const Volume* v, const Best* b, float best) {
    const float q = fdiv(v->r1024, fsqrt(best));
    V3 w = vsub(v->c1024, b->point);
    float k = 0.0f;
    if (v->capsule) {
        const float up = b->point.v[2] - v->c1024.v[2];
        k = fmax2(fmin2(up, v->h1024), 0.0f);
        w.v[2] = w.v[2] + k;
    }
    w = vmuls(w, q);
    if (v->capsule) {
        w.v[2] = w.v[2] - k;
    }
    return vadd(vmuls(w, INV1024), vmuls(b->point, INV1024));
}

/* The moby pass of the sphere and capsule kernels. */
static void volume_mobys(const Ctx* x, uint32_t stamp, uint32_t fl, const Volume* vol, gaddr ignore,
                         float* best_d, Best* best) {
    const V3 c = vol->c1024;
    const float r = vol->r1024;
    const float h = vol->h1024;
    const float shrink = best_shrink();
    unsigned lo[2], hi[2];
    for (int k = 0; k < 2; ++k) {
        lo[k] = (uint32_t)ftoi0(vol->lo1024.v[k]) >> 14;
        hi[k] = (uint32_t)ftoi0(vol->hi1024.v[k]) >> 14;
    }
    for (unsigned gy = lo[1]; gy <= hi[1]; ++gy) {
        for (unsigned gx = lo[0]; gx <= hi[0]; ++gx) {
            int count;
            const gaddr list = grid_cell(x, gx, gy, &count);
            for (int li = 0; li < count; ++li) {
                const gaddr m = moby_at(x, U16(list + 2 * (gaddr)li));
                if (m == ignore || U32(m + 0x9C) == stamp || U32(m + 0x94) == 0 || unbounded(m)) {
                    continue;
                }
                const V3 bs = vld(m);
                const float rr = r + F32(m + 0xC);
                if (!vol->capsule) {
                    const V3 e = vsub(bs, c);
                    const V3 sq = vmul(e, e);
                    const float v = ((sq.v[0] - rr * rr) + sq.v[1]) + sq.v[2];
                    if (gtz(v)) {
                        continue;
                    }
                } else {
                    const float ex = bs.v[0] - c.v[0], ey = bs.v[1] - c.v[1];
                    const float z4 = ((bs.v[2] - rr) - c.v[2]) - h;
                    const float z5 = (bs.v[2] + rr) - c.v[2];
                    const float v = (ex * ex - rr * rr) + ey * ey;
                    if (gtz(v) || !gtz(z5) || !neg(z4)) {
                        continue;
                    }
                }
                U32(m + 0x9C) = stamp;
                MobyColl mc;
                moby_coll(m, &mc);
                if (joint_count(&mc, fl) != 0 && (fl & 0x2) != 0 && mc.nverts == 0) {
                    continue;
                }
                pose_joints(&mc, fl);
                if (mc.nverts != 0) {
                    const V3 c_rel = vsub(c, mc.pos);
                    V3 vlo, vhi;
                    for (int k = 0; k < 3; ++k) {
                        vlo.v[k] = c_rel.v[k] - r;
                        vhi.v[k] = c_rel.v[k] + r;
                    }
                    if (vol->capsule) {
                        vhi.v[2] = vhi.v[2] + h;
                    }
                    const int nv = moby_mesh_verts(&mc, vlo, vhi);
                    for (int fi = 0; fi < mc.nfaces; ++fi) {
                        Vert v[3];
                        int ty;
                        if (!moby_face(&mc, nv, fi, fl, v, &ty)) {
                            continue;
                        }
                        V3 pt, nrm, e1, e2;
                        float d2;
                        const int hit = vol->capsule
                                            ? capsule_tri(fl, c_rel, h, v[0].p, v[1].p, v[2].p, *best_d, &pt, &d2,
                                                          &nrm, &e1, &e2)
                                            : sphere_tri(fl, c_rel, v[0].p, v[1].p, v[2].p, *best_d, &pt, &d2, &nrm,
                                                         &e1, &e2);
                        if (!hit) {
                            continue;
                        }
                        *best_d = d2 * shrink;
                        record(best, ty, pt, nrm, v[0].p, e1, e2, mc.pos, m);
                    }
                }
                if (fl & 0x2) {
                    continue;
                }
                for (int i = 0; i < mc.nprims; ++i) {
                    int last;
                    const int on = prim_active(&mc, i, fl, &last);
                    if (on) {
                        const gaddr p = mc.prims + 0x20 * (gaddr)i;
                        V3 cc, pt, off;
                        float rad, d2;
                        if (vol_prim(&mc, p, c, vol->capsule, &cc, &rad)) {
                            V3 from = c;
                            if (vol->capsule) {
                                from.v[2] = c.v[2] + fmax2(fmin2(cc.v[2] - c.v[2], h), 0.0f);
                            }
                            if (vol_prim_test(from, r, cc, rad, *best_d, &pt, &off, &d2)) {
                                *best_d = d2 * 1.0f;
                                prim_best(best, p, pt, off, m);
                            }
                        }
                    }
                    if (last) {
                        break;
                    }
                }
            }
        }
    }
}

static int coll_volume(const Ctx* x, float radius, float height, gaddr centre, uint32_t fl, gaddr ignore,
                       int capsule) {
    const int64_t stamp = new_stamp(x->out);
    if (stamp < 0) {
        return 0;
    }
    Volume vol;
    if (!volume_new(&vol, centre, radius, height, capsule)) {
        return 0;
    }
    const float shrink = best_shrink();
    float best_d = vol.rr * shrink;
    Best best;
    memset(&best, 0, sizeof best);
    for (int k = 0; k < 3; ++k) {
        best.tri[k] = vmuls(vld(x->out + 0x50 + 0x10 * (gaddr)k), K1024);
    }
    const gaddr mesh = U32(x->out);
    if ((fl & 0x1) == 0) {
        int lo[3], hi[3];
        for (int k = 0; k < 3; ++k) {
            lo[k] = (int)((uint32_t)ftoi0(vol.lo.v[k]) >> 2);
            hi[k] = (int)((uint32_t)ftoi0(vol.hi.v[k]) >> 2);
        }
        const float nrr = 0.0f - vol.rr;
        for (int z = lo[2]; z <= hi[2]; ++z) {
            for (int y = lo[1]; y <= hi[1]; ++y) {
                for (int xx = lo[0]; xx <= hi[0]; ++xx) {
                    const int cxyz[3] = {xx, y, z};
                    float sq[3];
                    for (int k = 0; k < 3; ++k) {
                        const float cmin = (float)(cxyz[k] << 12);
                        const float cmax = cmin + 4096.0f;
                        const float p = fmin2(fmax2(cmin, vol.c1024.v[k]), cmax);
                        const float e = p - vol.c1024.v[k];
                        sq[k] = e * e;
                    }
                    if (!neg(((nrr + sq[0]) + sq[1]) + sq[2])) {
                        continue;
                    }
                    Cell cell;
                    if (!cell_open(mesh, cell_word(mesh, xx, y, z), &cell)) {
                        continue;
                    }
                    const V3 cen = v3((float)(xx << 12) + 2048.0f, (float)(y << 12) + 2048.0f,
                                      (float)(z << 12) + 2048.0f);
                    int16_t blo[3], bhi[3];
                    for (int k = 0; k < 3; ++k) {
                        blo[k] = (int16_t)ftoi0(vol.lo1024.v[k] - cen.v[k]);
                        bhi[k] = (int16_t)ftoi0(vol.hi1024.v[k] - cen.v[k]);
                    }
                    const V3 c_rel = vsub(vol.c1024, cen);
                    decode_vertices(&cell, blo, bhi);
                    int vi[3], ty;
                    for (int j = 0; cell_triangle(&cell, j, vi, &ty); ++j) {
                        const Vert* v0 = &g_verts[vi[0]];
                        const Vert* v1 = &g_verts[vi[1]];
                        const Vert* v2 = &g_verts[vi[2]];
                        if ((v0->code & v1->code & v2->code) != 0 || face_excluded(fl, ty)) {
                            continue;
                        }
                        V3 pt, nrm, e1, e2;
                        float d2;
                        const int hit = capsule ? capsule_tri(fl, c_rel, vol.h1024, v0->p, v1->p, v2->p, best_d, &pt,
                                                              &d2, &nrm, &e1, &e2)
                                                : sphere_tri(fl, c_rel, v0->p, v1->p, v2->p, best_d, &pt, &d2, &nrm,
                                                             &e1, &e2);
                        if (!hit) {
                            continue;
                        }
                        best_d = d2 * shrink;
                        record(&best, ty, pt, nrm, v0->p, e1, e2, cen, 0);
                    }
                }
            }
        }
    }
    volume_mobys(x, (uint32_t)stamp, fl, &vol, ignore, &best_d, &best);
    if (!best.have) {
        return 0;
    }
    write_out(x->out, &best, INV1024);
    vst(x->out + 0x30, volume_pushed(&vol, &best, best_d));
    return 1;
}

/* ---- A count of queries and hits (OPENRAC_COLL_TRACE: logged every 2000 queries) ---- */

static void trace(int kind, int hit) {
    static int on = -1;
    static unsigned long calls[6], hits[6], total;
    static const char* names[6] = {"line", "sphere", "capsule", "sphere-mobys", "hero-groups", "boot line"};
    if (on < 0) {
        on = getenv("OPENRAC_COLL_TRACE") != NULL;
    }
    if (!on) {
        return;
    }
    calls[kind]++;
    hits[kind] += hit != 0;
    if (++total % 2000 == 0) {
        fprintf(stderr, "[debug] collision:");
        for (int k = 0; k < 6; ++k) {
            fprintf(stderr, " %s %lu/%lu", names[k], hits[k], calls[k]);
        }
        fprintf(stderr, "\n");
    }
}

/* ---- The entry points ---- */

/* The globals of the level program loaded: level 0's addresses moved to the loaded level's. */
static const Ctx* level_ctx(void) {
    static Ctx x;
    x.out = OPENRAC_LDATA(0, LEVEL0.out);
    x.mobys = OPENRAC_LDATA(0, LEVEL0.mobys);
    x.grid = OPENRAC_LDATA(0, LEVEL0.grid);
    x.hits = OPENRAC_LDATA(0, LEVEL0.hits);
    x.list = OPENRAC_LDATA(0, LEVEL0.list);
    x.hero = OPENRAC_LDATA(0, LEVEL0.hero);
    return &x;
}

/* The executable's line query: its own globals, or with a level loaded (where retail runs the level's
 * copy of this kernel) the level's. */
int func_001EFE10(gaddr a0, gaddr a1, int a2, int a3, int a4) {
    const int fr = fegetround();
    fesetround(FE_TOWARDZERO);
    const Ctx* x = openrac_game_loaded_overlay() >= 0 ? level_ctx() : &BOOT;
    const int r = coll_line(x, a0, a1, (uint32_t)a2, (gaddr)a3, (gaddr)a4);
    trace(5, r);
    fesetround(fr);
    return r;
}

/* CollLine_Fix(a, b, flags, ignore, hit template): the hit nearest a on the segment a -> b, world mesh
 * then mobys; 1 with CollOutput filled, else 0. */
int func_L00_001EFFF0(gaddr a0, gaddr a1, int a2, int a3, int a4) {
    const int fr = fegetround();
    fesetround(FE_TOWARDZERO);
    const int r = coll_line(level_ctx(), a0, a1, (uint32_t)a2, (gaddr)a3, (gaddr)a4);
    trace(0, r);
    fesetround(fr);
    return r;
}

/* The sphere query (radius, centre, flags, ignore): the closest face within the radius, the centre
 * pushed out to touch it (+0x30). */
int func_L00_001F10E0(float a0, gaddr a1, int a2, gaddr a3) {
    const int fr = fegetround();
    fesetround(FE_TOWARDZERO);
    const int r = coll_volume(level_ctx(), a0, 0.0f, a1, (uint32_t)a2, a3, 0);
    trace(1, r);
    fesetround(fr);
    return r;
}

/* The vertical capsule query (radius, height, base, flags, ignore): the hero's body. */
int func_L00_001F1D20(float a0, float a1, gaddr a2, int a3, int a4) {
    const int fr = fegetround();
    fesetround(FE_TOWARDZERO);
    const int r = coll_volume(level_ctx(), a0, a1, a2, (uint32_t)a3, (gaddr)a4, 1);
    trace(2, r);
    fesetround(fr);
    return r;
}

/* coll_sphere_mobys(radius, centre, flags, ignore, hit template): every moby the sphere touches, in
 * grid order, listed (0-terminated) at the list address; returns how many. CollOutput gets +0x18 = the
 * first, +0x1C = 0x3F, +0x20 = (0, 0, 0, 1); with a template each listed moby with mode bit 0x4000 gets
 * a hit record. */
int func_L00_001F2BE8(float a0, gaddr a1, int a2, gaddr a3, gaddr a4) {
    const int fr = fegetround();
    fesetround(FE_TOWARDZERO);
    const Ctx* x = level_ctx();
    const uint32_t fl = (uint32_t)a2;
    int result = 0;
    const int64_t stamp = new_stamp(x->out);
    Volume vol;
    if (stamp >= 0 && volume_new(&vol, a1, a0, 0.0f, 0)) {
        const V3 c = vol.c1024;
        const float r = vol.r1024;
        const float best = vol.rr;
        gaddr cursor = x->list;
        unsigned lo[2], hi[2];
        for (int k = 0; k < 2; ++k) {
            lo[k] = (uint32_t)ftoi0(vol.lo.v[k]) >> 4;
            hi[k] = (uint32_t)ftoi0(vol.hi.v[k]) >> 4;
        }
        for (unsigned gy = lo[1]; gy <= hi[1]; ++gy) {
            for (unsigned gx = lo[0]; gx <= hi[0]; ++gx) {
                int count;
                const gaddr list = grid_cell(x, gx, gy, &count);
                for (int li = 0; li < count; ++li) {
                    const gaddr m = moby_at(x, U16(list + 2 * (gaddr)li));
                    if (m == a3 || U32(m + 0x9C) == (uint32_t)stamp || U32(m + 0x94) == 0 || unbounded(m)) {
                        continue;
                    }
                    const V3 bs = vld(m);
                    const float rr = r + F32(m + 0xC);
                    const V3 e = vsub(bs, c);
                    const V3 sq = vmul(e, e);
                    if (gtz(((sq.v[0] - rr * rr) + sq.v[1]) + sq.v[2])) {
                        continue;
                    }
                    U32(m + 0x9C) = (uint32_t)stamp;
                    MobyColl mc;
                    moby_coll(m, &mc);
                    pose_joints(&mc, fl);
                    int hit = 0;
                    if (mc.nverts != 0) {
                        const V3 c_rel = vsub(c, mc.pos);
                        V3 vlo, vhi;
                        for (int k = 0; k < 3; ++k) {
                            vlo.v[k] = c_rel.v[k] - r;
                            vhi.v[k] = c_rel.v[k] + r;
                        }
                        const int nv = moby_mesh_verts(&mc, vlo, vhi);
                        for (int fi = 0; fi < mc.nfaces && !hit; ++fi) {
                            Vert v[3];
                            int ty;
                            V3 pt, nrm, e1, e2;
                            float d2;
                            if (moby_face(&mc, nv, fi, fl, v, &ty) &&
                                sphere_tri(fl, c_rel, v[0].p, v[1].p, v[2].p, best, &pt, &d2, &nrm, &e1, &e2)) {
                                hit = 1;
                            }
                        }
                    }
                    for (int i = 0; i < mc.nprims && !hit; ++i) {
                        int last;
                        const int on = prim_active(&mc, i, fl, &last);
                        if (on) {
                            V3 cc, pt, off;
                            float rad, d2;
                            const gaddr p = mc.prims + 0x20 * (gaddr)i;
                            if (vol_prim(&mc, p, c, 0, &cc, &rad) && vol_prim_test(c, r, cc, rad, best, &pt, &off, &d2)) {
                                hit = 1;
                            }
                        }
                        if (last) {
                            break;
                        }
                    }
                    if (hit) {
                        U32(cursor) = m;
                        cursor += 4;
                    }
                }
            }
        }
        U32(cursor) = 0;
        const gaddr out = x->out;
        F32(out + 0x20) = 0.0f;
        F32(out + 0x24) = 0.0f;
        F32(out + 0x28) = 0.0f;
        F32(out + 0x2C) = 1.0f;
        S32(out + 0x1C) = 0x3F;
        U32(out + 0x18) = U32(x->list);
        if (a4 != 0) {
            /* OPENRAC_HIT_LOG: each sphere with a template and the mobys it listed, on stderr. */
            static int hit_log = -1;
            if (hit_log < 0) {
                hit_log = getenv("OPENRAC_HIT_LOG") != NULL;
            }
            if (hit_log) {
                fprintf(stderr, "[debug] hit sphere r %.2f at %.2f %.2f %.2f flags %#x damage %.2f by class %d: %d listed",
                        (double)a0, (double)F32(a1), (double)F32(a1 + 4), (double)F32(a1 + 8), U32(a4 + 0x14),
                        (double)F32(a4 + 0x1C), a3 != 0 ? S16(a3 + 0xA6) : -1, (int)((cursor - x->list) >> 2));
                for (gaddr at = x->list; at != cursor; at += 4) {
                    fprintf(stderr, " %d%s", S16(U32(at) + 0xA6), (U16(U32(at) + 0x34) & 0x4000) ? "" : "(no hits)");
                }
                fprintf(stderr, "\n");
            }
            for (gaddr at = x->list; at != cursor; at += 4) {
                const gaddr m = U32(at);
                if ((U16(m + 0x34) & 0x4000) == 0) {
                    continue;
                }
                const int8_t old = S8(m + 0xA4);
                if (old >= 0) {
                    const gaddr rec = x->hits + (gaddr)old * 64;
                    if (U32(rec + 0x34) == m && (int32_t)(U32(rec + 0x2C) - U32(a4 + 0x1C)) > 0) {
                        continue;
                    }
                }
                const uint32_t i = U32(out + 0x14);
                const gaddr rec = x->hits + i * 64;
                F32(rec) = 0.0f;
                F32(rec + 4) = 0.0f;
                F32(rec + 8) = 0.0f;
                F32(rec + 0xC) = 1.0f;
                S32(rec + 0x38) = 0;
                memmove(G(rec + 0x10), G(a4), 0x24);
                U32(rec + 0x34) = m;
                S8(m + 0xA4) = (int8_t)i;
                U32(out + 0x14) = (i + 1) & 0x3F;
            }
        }
        result = (int)((cursor - x->list) >> 2);
    }
    trace(3, result);
    fesetround(fr);
    return result;
}

/* Sphere against the hero-only collision groups (radius, centre), after each capsule pass of the hero:
 * everything at x64, groups culled by their bounding sphere, vertices unsigned and absolute. */
int func_L00_001F34F0(float a0, gaddr a1) {
    const int fr = fegetround();
    fesetround(FE_TOWARDZERO);
    const Ctx* x = level_ctx();
    const gaddr hero = U32(x->hero);
    int result = 0;
    if (hero != 0) {
        const float shrink = best_shrink();
        const float r64 = a0 * 64.0f;
        const V3 c64 = vmuls(vld(a1), 64.0f);
        float best_d = r64 * r64;
        int16_t lo16[3], hi16[3];
        for (int k = 0; k < 3; ++k) {
            lo16[k] = (int16_t)ftoi0(c64.v[k] - r64);
            hi16[k] = (int16_t)ftoi0(c64.v[k] + r64);
        }
        Best best;
        memset(&best, 0, sizeof best);
        const int groups = S32(hero);
        for (int g = 0; g < groups; ++g) {
            const gaddr h = hero + 0x10 + 0x10 * (gaddr)g;
            const V3 s = v3((float)U16(h), (float)U16(h + 2), (float)U16(h + 4));
            const float rr = r64 + (float)U16(h + 6);
            const V3 dd = vsub(s, c64);
            const V3 sq = vmul(dd, dd);
            const float v = ((sq.v[1] - rr * rr) + sq.v[0]) + sq.v[2];
            if (!neg(v) && !(fb(v) == 0 && fb(sq.v[0]) == 0)) {
                continue;
            }
            const int nv = U16(h + 0xA);
            const int nt = U8(h + 8);
            const gaddr data = U32(h + 0xC);
            for (int i = 0; i < nv && i < 2048; ++i) {
                const gaddr vp = data + 8 * (gaddr)i;
                const int16_t hv[3] = {(int16_t)U16(vp), (int16_t)U16(vp + 2), (int16_t)U16(vp + 4)};
                uint32_t code = 0;
                for (int k = 0; k < 3; ++k) {
                    const int below = (int16_t)(hv[k] - lo16[k]) < 0;
                    const int above = (int16_t)(hi16[k] - hv[k]) < 0;
                    code |= (uint32_t)(below | (above << 1)) << (8 * k);
                }
                g_verts[i].p = v3((float)U16(vp), (float)U16(vp + 2), (float)U16(vp + 4));
                g_verts[i].code = code;
            }
            const gaddr tris = data + 8 * (gaddr)nv;
            for (int t = 0; t < nt; ++t) {
                const gaddr tp = tris + 4 * (gaddr)t;
                const int i0 = U8(tp), i1 = U8(tp + 1), i2 = U8(tp + 2);
                if (i0 >= nv || i1 >= nv || i2 >= nv) {
                    continue;
                }
                const Vert* v0 = &g_verts[i0];
                const Vert* v1 = &g_verts[i1];
                const Vert* v2 = &g_verts[i2];
                if ((v0->code & v1->code & v2->code) != 0) {
                    continue;
                }
                V3 pt, nrm, e1, e2;
                float d2;
                if (!sphere_tri(0, c64, v0->p, v1->p, v2->p, best_d, &pt, &d2, &nrm, &e1, &e2)) {
                    continue;
                }
                best_d = d2 * shrink;
                record(&best, U8(tp + 3), pt, nrm, v0->p, e1, e2, vzero(), 0);
            }
        }
        if (best.have) {
            const float q = fdiv(r64, fsqrt(best_d));
            const float k = 1.0f / 64.0f;
            const V3 w = vmuls(vmuls(vsub(c64, best.point), q), k);
            write_out(x->out, &best, k);
            vst(x->out + 0x30, vadd(w, vmuls(best.point, k)));
            result = 1;
        }
    }
    trace(4, result);
    fesetround(fr);
    return result;
}
