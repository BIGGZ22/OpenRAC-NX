/* Carried from the PAL decompilation's nonmatching/functional/func_L01_00262FE8.c (games/rac1) through the version
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
/* FUN_L01_00262038: ripple / water grid wave propagation kernel.
 * Handwritten assembly on PS2: takes scratchpad buffer a0, grid index a1,
 * target buffer selector a2, and base grid array a3.
 * Propagates wave equation across 16x16 grid cells with damping and neighbor coupling.
 */
extern float D_L01_001CAD00[] __asm__("D_L01_001CAD00");

void FUN_L01_00262038(char *scratch, int idx, int buf_sel, char *base) {
    float k_damp = 0.015625f; // 0x3D800000 = 1.0f / 64.0f
    float k_wave = 0.25f;     // 0x3E800000
    float k_one  = 1.0f;      // 0x3F800000
    float threshold = *(float *)((char *)&D_L01_001CAD00 + 0x1C);
    
    char *curr = scratch + 0x70;
    char *next_buf = base + (idx * 0x1190) + (buf_sel * 0x5C0) + 0x50;
    
    for (int row = 0; row < 16; ++row) {
        for (int col = 0; col < 16; ++col) {
            float n0 = *(float *)(curr - 0x64);
            float n1 = *(float *)(curr - 0x60);
            float n2 = *(float *)(curr - 0x5C);
            float n3 = *(float *)(curr - 0x4);
            float n4 = *(float *)(curr + 0x4);
            float n5 = *(float *)(curr + 0x5C);
            float n6 = *(float *)(curr + 0x60);
            float n7 = *(float *)(curr + 0x64);
            float cur_val = *(float *)next_buf;
            
            float sum = (n0 + n1 + n2 + n3 + n4 + n5 + n6 + n7) * k_wave;
            float diff = sum - (cur_val * k_one);
            float next_val = diff - (diff * k_damp);
            
            int bits = *(int *)&next_val & 0x7FFFFFFF;
            int thresh_bits = *(int *)&threshold;
            
            if (bits - thresh_bits < 0) {
                *(float *)next_buf = next_val;
            }
            
            curr += 4;
            next_buf += 4;
        }
        curr += 0x20;
    }
    
    char *grid_entry = base + (idx * 0x1190);
    char *src = grid_entry + (buf_sel * 0x5C0) + 0x50;
    char *target_base = base + (buf_sel * 0x5C0) + 0x50;
    
    unsigned char b0 = (unsigned char)grid_entry[0xC];
    if (b0 != 0xFF) {
        char *dst = target_base + (b0 * 0x1190);
        for (int i = 0; i < 4; ++i) {
            memcpy(dst + 0x480 + i * 16, src + i * 16, 16);
            memcpy(dst + 0x500 + i * 16, src + 0x40 + i * 16, 16);
        }
    }
    
    unsigned char b1 = (unsigned char)grid_entry[0xD];
    if (b1 != 0xFF) {
        char *dst = target_base + (b1 * 0x1190);
        for (int i = 0; i < 16; ++i) {
            *(int *)(dst + 0x4C0 + i * 4) = *(int *)(src + i * 0x40);
            *(int *)(dst + 0x540 + i * 4) = *(int *)(src + 0x4 + i * 0x40);
        }
    }
    
    unsigned char b2 = (unsigned char)grid_entry[0xE];
    if (b2 != 0xFF) {
        char *dst = target_base + (b2 * 0x1190);
        for (int i = 0; i < 4; ++i) {
            memcpy(dst + 0x400 + i * 16, src + 0x3C0 + i * 16, 16);
        }
    }
    
    unsigned char b3 = (unsigned char)grid_entry[0xF];
    if (b3 != 0xFF) {
        char *dst = target_base + (b3 * 0x1190);
        for (int i = 0; i < 16; ++i) {
            *(int *)(dst + 0x440 + i * 4) = *(int *)(src + 0x3C + i * 0x40);
        }
    }
    
    unsigned char b4 = (unsigned char)grid_entry[0x10];
    if (b4 != 0xFF) {
        char *dst = target_base + (b4 * 0x1190);
        *(int *)(dst + 0x58C) = *(int *)src;
        *(int *)(dst + 0x590) = *(int *)(src + 0x4);
        *(int *)(dst + 0x594) = *(int *)(src + 0x40);
    }
    
    unsigned char b5 = (unsigned char)grid_entry[0x11];
    if (b5 != 0xFF) {
        char *dst = target_base + (b5 * 0x1190);
        *(int *)(dst + 0x588) = *(int *)(src + 0x3C);
    }
    
    unsigned char b6 = (unsigned char)grid_entry[0x12];
    if (b6 != 0xFF) {
        char *dst = target_base + (b6 * 0x1190);
        *(int *)(dst + 0x584) = *(int *)(src + 0x3C0);
    }
    
    unsigned char b7 = (unsigned char)grid_entry[0x13];
    if (b7 != 0xFF) {
        char *dst = target_base + (b7 * 0x1190);
        *(int *)(dst + 0x580) = *(int *)(src + 0x3FC);
    }
}
