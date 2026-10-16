/* Carried from the PAL decompilation's nonmatching/functional/func_L01_00262DA8.c (games/rac1) through the version
 * map (../../README.md): functional C, not matched C. Names and addresses are this version's. */
#include "types.h"
#include "qcopy.h"
#include "qzero.h"
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
/* What the PAL file this was written in declares before it (its unit, and its own file). */
/* functional: medium. Lights a 17x17 height grid held in scratchpad (heights at 0x70000070, 0x60-byte rows): per point a normal from the four neighbour height differences, a grey shade byte from the normal and the light dir (replicated to 0xBBBBBB into 0x70001088) and an environment-map UV from the view vector reflected about the normal (pairs of floats into 0x70000780). Written from the VU0 assembly by hand; not testable (VU0, not called in the capture run). */
extern float sqrtf(float);
extern float D_L01_001CAF80_62DA8[] __asm__("D_L01_001CAD00");
extern float D_L01_00167180_62DA8[] __asm__("D_L01_00167100");



void FUN_L01_00261df8(float *arg0) {
    float *c16 = D_L01_001CAF80_62DA8;
    float *c15 = c16 + 4;
    float *c21 = c16 + 12;
    float *eye = D_L01_00167180_62DA8 + 0x140 / 4;
    float half = 0.5f;
    float k65 = 65536.5f;
    float cc = 1.0f;
    union { unsigned int u; float f; } bits;
    float k3 = 3.25f;
    float *hrow = (float *)0x70000070;
    float *uvp = (float *)0x70000780;
    unsigned int *colp = (unsigned int *)0x70001088;
    float X = c15[0], Y = c15[1], S = c15[2];
    float py, px, pz;
    int row, col;
    float a_x, a_y, b_x, b_y, c_x, c_y, d_x, d_y;

    bits.u = 0x3F12B021;
    cc = bits.f;

    /* corner offsets: vf10 (0,-Y), vf11 (-X,Y), vf12 (0,Y), vf13 (X,-Y) */
    a_x = 0.0f; a_y = 0.0f - Y;       /* vf10 */
    b_x = 0.0f - X; b_y = 0.0f + Y;   /* vf11 */
    c_x = 0.0f; c_y = 0.0f + Y;       /* vf12 */
    d_x = 0.0f + X; d_y = 0.0f - Y;   /* vf13 */

    py = arg0[1] + c16[3];
    for (row = 0; row < 17; row++) {
        px = arg0[0] + c16[2];
        for (col = 0; col < 17; col++) {
            float hc = hrow[col];
            float hl = hrow[col - 1];
            float hr = hrow[col + 1];
            float hu = hrow[col - 0x18];
            float hd = hrow[col + 0x18];
            float z10, z11, z12, z13;
            float n1x, n1y, n1z, n2x, n2y, n2z, nx, ny, nz;
            float s, q, vx, vy, vz, dx, dy, dz, d, t;
            float lx, ly, lz, rx, ry, rz;
            float ex, ey, ez, sq2, q2;

            pz = 0.0f + hc;
            z10 = pz - hd;
            z11 = hd - hr;
            z12 = pz - hu;
            z13 = hu - hl;
            /* n1 = vf11 x vf10, n2 = vf13 x vf12 */
            n1x = b_y * z10 - z11 * a_y;
            n1y = z11 * a_x - z10 * b_x;
            n1z = b_x * a_y - a_x * b_y;
            n2x = d_y * z12 - z13 * c_y;
            n2y = z13 * c_x - z12 * d_x;
            n2z = d_x * c_y - c_x * d_y;
            nx = n1x + n2x;
            ny = n1y + n2y;
            nz = n1z + n2z;
            s = (nx * nx + ny * ny) + 1.0f * (nz * nz);
            q = 1.0f / sqrtf(s);
            pz = pz + arg0[2];
            nx = nx * q;
            ny = ny * q;
            nz = nz * q;
            /* vf1 = -n */
            lx = 0.0f - nx;
            ly = 0.0f - ny;
            lz = 0.0f - nz;
            ex = eye[0] - px;
            ey = eye[1] - py;
            ez = eye[2] - pz;
            sq2 = (ex * ex + ey * ey) + 1.0f * (ez * ez);
            q2 = 1.0f / sqrtf(sq2);
            /* shade = clamp((dot(light, -n) - cc) * 3.25 + cc, 0, 1) */
            t = (c21[0] * lx + c21[1] * ly) + 1.0f * (c21[2] * lz);
            t = t - cc;
            t = t * k3;
            t = t + cc;
            if (t < 0.0f) t = 0.0f;
            if (t > 1.0f) t = 1.0f;
            t = t + k65;
            bits.f = t;
            {
                unsigned int b = bits.u & 0xFF;
                *colp = (b | (b << 8) | (b << 16)) & 0xFFFFFF;
            }
            colp++;
            vx = ex * q2;
            vy = ey * q2;
            vz = ez * q2;
            /* d = v . n */
            d = ((0.0f - vx) * lx + (0.0f - vy) * ly) + 1.0f * ((0.0f - vz) * lz);
            bits.f = d;
            rx = vx;
            ry = vy;
            rz = vz;
            if ((bits.u & 0x80000000u) == 0) {
                float tx = lx * d - (0.0f - vx);
                float ty = ly * d - (0.0f - vy);
                float tz = lz * d - (0.0f - vz);
                tx = tx + tx;
                ty = ty + ty;
                tz = tz + tz;
                rx = (0.0f - vx) + tx;
                ry = (0.0f - vy) + ty;
                rz = (0.0f - vz) + tz;
            }
            uvp[0] = (rx * S) * half + half;
            uvp[1] = (ry * S) * half + half;
            uvp += 2;
            px = px + X;
        }
        py = py + Y;
        hrow += 0x18;
    }
}
