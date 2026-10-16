/* Carried from the PAL decompilation's nonmatching/functional/func_L12_002677B4.c (games/rac1) through the version
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
/* functional: medium. Environment-map UVs: for n points (pos, 3 floats each) with normals (3 floats each), v = (cam - pos) * 0.45/|cam - pos| (cam = float xyz at D_L12_00167100+0x140), u = -v; if dot(u,n) > 0 uses v, else the reflection 2(n*dot) - u... (u + 2(n*dot - u)); writes (x + 0.5, y + 0.5) to out (8 bytes per point). VU0 rounding and the rsqrt (|x|) not checked against hardware. */
extern char D_L12_00167180_677B4[] __asm__("D_L12_00167100");


void FUN_L12_002667fc(int n, float *pos, float *nrm, float *out) {
    float *cam = (float *)(D_L12_00167180_677B4 + 0x140);
    float cx = cam[0], cy = cam[1], cz = cam[2];
    do {
        float dx = cx - pos[0];
        float dy = cy - pos[1];
        float dz = cz - pos[2];
        float len2 = dx * dx + dy * dy + 1.0f * (dz * dz);
        float q = 0.45f / __builtin_sqrtf(__builtin_fabsf(len2));
        float vx = dx * q, vy = dy * q, vz = dz * q;
        float ux = 0.0f - vx, uy = 0.0f - vy, uz = 0.0f - vz;
        float dot = ux * nrm[0] + uy * nrm[1] + 1.0f * (uz * nrm[2]);
        float rx, ry;
        pos += 3;
        if (dot > 0.0f) {
            rx = vx;
            ry = vy;
        } else {
            float tx = nrm[0] * dot - ux;
            float ty = nrm[1] * dot - uy;
            tx = tx + tx;
            ty = ty + ty;
            rx = ux + tx;
            ry = uy + ty;
        }
        nrm += 3;
        out[0] = rx + 0.5f;
        out[1] = ry + 0.5f;
        out += 2;
        n--;
    } while (n != 0);
}
