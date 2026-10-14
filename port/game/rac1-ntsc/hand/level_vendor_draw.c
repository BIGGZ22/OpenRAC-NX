/* Carried from port/game/rac1-pal/hand/level_vendor_draw.c through the version map (../README.md): names and
 * addresses are this version's, raw addresses in comments are PAL's. Not yet checked in play. */
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Native-only reconstruction of PAL 002D2E60..002D3328, from retail
 * instructions. Builds the reflective vendor mesh and submits 74 quads.
 * This is not a matching PS2 decompilation. */
typedef struct VendorQuad {
    float corner[4][4];
    unsigned int color[4];
    float st[4][2];
    unsigned long long clamp, texture, filter, alpha;
} VendorQuad;

/* Level 0 gp is 00166D00; these are gp-5354 through gp-533C. */
extern int vendor_blend[] __asm__("D_L00_001618DC");
extern float vendor_camera[] __asm__("D_L00_00166C80");
extern float vendor_vertices[][4] __asm__("D_L00_001DE7B0");
extern short vendor_indices[][4][3] __asm__("D_L00_001DEE10");
extern float vendor_normals[][4] __asm__("D_L00_001DF9A0");
extern float vendor_world[][4] __asm__("D_L00_001E04A0");
extern float vendor_uv[][2] __asm__("D_L00_001E0B00");
extern float vendor_saved_uv[][2] __asm__("D_L00_001E0E30");
extern unsigned long long vendor_texture(int) __asm__("func_001F44B8");
extern float vendor_abs(float) __asm__("func_001F99C0");
extern float vendor_sqrt(float) __asm__("func_001F9988");
extern int vendor_countdown(int*) __asm__("func_001F9740");
extern int vendor_ticks(int) __asm__("func_001F9688");
extern float vendor_float(int) __asm__("func_001FA6C0");
extern void vendor_transform(float*, float*, float*) __asm__("func_001F9D20");
extern void vendor_sub(float*, float*, float*) __asm__("func_001F9A28");
extern void vendor_normalize(float*, float*, float) __asm__("func_L00_001FF500");
extern float vendor_dot(float*, float*) __asm__("func_001F9AB0");
extern void vendor_scale(float*, float*, float) __asm__("func_001F9A68");
extern void vendor_draw(VendorQuad*, void*, int) __asm__("func_L00_001FD228");

void FUN_L00_002d19b0(unsigned char* moby) {
    float* data = *(float**)(moby + 0x78);
    VendorQuad quad;
    float view[4], normal[4], reflected[4], ratio, twice_dot, denominator;
    int i, j, lane, nearby;
    quad.texture = vendor_texture(vendor_blend[5]);
    quad.clamp = 0;
    quad.filter = 0xFF9000000260ULL;
    /* lw sign-extends to 64 bits before the shifts/ORs on the EE. */
    quad.alpha = (unsigned long long)(long long)vendor_blend[0]
        | ((unsigned long long)(long long)vendor_blend[1] << 2)
        | ((unsigned long long)(long long)vendor_blend[2] << 4)
        | ((unsigned long long)(long long)vendor_blend[3] << 6)
        | ((unsigned long long)(long long)vendor_blend[4] << 32);
    for (i = 0; i < 4; ++i) quad.color[i] = (unsigned int)vendor_blend[6];
    nearby = vendor_abs(vendor_camera[0x50] - *(float*)(moby + 0x10)) < 16.0f
        && vendor_abs(vendor_camera[0x51] - *(float*)(moby + 0x14)) < 16.0f;
    if (nearby || moby[0x20] == 1) {
        moby[0xBC] = 1;
        vendor_countdown((int*)(data + 16));
        ratio = vendor_float(*(int*)(data + 16));
        ratio = ratio / vendor_float(vendor_ticks(60));
        for (i = 0; i < 102; ++i) {
            vendor_transform(vendor_world[i], vendor_vertices[i], data);
            vendor_sub(view, vendor_world[i], vendor_camera + 0x50);
            vendor_normalize(view, view, 1.0f);
            vendor_transform(normal, vendor_normals[i], data);
            vendor_normalize(normal, normal, 0.1f);
            twice_dot = vendor_dot(normal, view);
            twice_dot = twice_dot + twice_dot;
            vendor_scale(reflected, normal, twice_dot);
            vendor_sub(reflected, view, reflected);
            vendor_normalize(reflected, reflected, 1.0f);
            reflected[2] = reflected[2] + 1.0f;
            denominator = vendor_sqrt(reflected[2] + reflected[2]);
            denominator = denominator + denominator;
            for (lane = 0; lane < 2; ++lane) {
                float uv = reflected[lane] / denominator + 0.5f;
                if (moby[0x20] != 1 && *(int*)(data + 16) != 0)
                    uv = uv + (vendor_saved_uv[i][lane] - uv) * ratio;
                vendor_uv[i][lane] = uv;
            }
        }
        if (moby[0x20] == 1) moby[0x20] = 2;
    } else {
        int save_uv = moby[0xBC] == 1;
        if (save_uv) moby[0xBC] = 0;
        for (i = 0; i < 102; ++i) {
            if (save_uv) {
                vendor_saved_uv[i][0] = vendor_uv[i][0];
                vendor_saved_uv[i][1] = vendor_uv[i][1];
            }
            vendor_transform(vendor_world[i], vendor_vertices[i], data);
        }
        *(int*)(data + 16) = vendor_ticks(60);
    }
    for (i = 0; i < 74; ++i) {
        for (j = 0; j < 4; ++j) {
            int index = vendor_indices[i][j][0];
            for (lane = 0; lane < 4; ++lane)
                quad.corner[j][lane] = vendor_world[index][lane];
            quad.st[j][0] = vendor_uv[index][0];
            quad.st[j][1] = vendor_uv[index][1];
        }
        vendor_draw(&quad, 0, 0);
    }
}
