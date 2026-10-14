/* Carried from port/game/rac1-pal/hand/level_moby_lighting.c through the version map (../README.md): names and
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
/* Native-only PAL handwritten lighting: 00257E18..00258248. The query
 * bodies include their 00257EBC and 00257FB4/0025804C branch tails.
 * Reconstructed from retail instructions; no PS2 matching claim. */
typedef struct MobyLightRegion {
    float matrix[16];
    unsigned int colors[2], lights[2], flags;
    unsigned char pad54[0x2C];
} MobyLightRegion;
typedef struct MobyPointLight {
    float position[3], radius_squared;
    unsigned int color;
    unsigned char pad14[12];
} MobyPointLight;
typedef struct MobyLightObject {
    unsigned char pad00[16];
    float position[4];
    unsigned char pad20[0x18];
    unsigned int lights, color;
    unsigned char pad40[0x40];
    unsigned int ambient;
} MobyLightObject;
extern int ml_last __asm__("D_L00_0015FBC4");
extern float ml_bounds[][4] __asm__("D_L00_0017ECC0");
extern MobyLightRegion ml_regions[] __asm__("D_L00_0017EEC0");
extern unsigned int* ml_grid __asm__("D_L00_0015FBC0");
extern MobyPointLight* ml_points __asm__("D_L00_0015FB80");

/* Use the same saturated conversion convention as native collision. */
static int ml_integer(float f) {
    if (f != f) return 0;
    if (f >= 2147483648.0f) return 0x7FFFFFFF;
    if (f <= -2147483648.0f) return (-2147483647 - 1);
    return (int)f;
}

int func_L00_00256DE0(const float* position, float* blend, int* index) {
    int i, j;
    for (i = 0; i <= ml_last; ++i) {
        float x = position[0] - ml_bounds[i][0];
        float y = position[1] - ml_bounds[i][1];
        union { float f; unsigned int bits; } distance;
        float local[3];
        const float* m;
        distance.f = (x * x + y * y) - ml_bounds[i][3];
        if (!(distance.bits & 0x80000000u)) continue;
        m = ml_regions[i].matrix;
        for (j = 0; j < 3; ++j) {
            float v = m[j] * position[0];
            v = v + m[4 + j] * position[1];
            v = v + m[8 + j] * position[2];
            local[j] = v + m[12 + j];
        }
        /* Retail returns after the first broad hit, even when clipping fails.
         * It ignores the input w and uses 1 for the translation row. */
        for (j = 0; j < 3; ++j) if (local[j] < -1 || local[j] > 1) return 0;
        *index = i;
        *blend = (local[0] + 1.0f) * 0.5f;
        return 1;
    }
    return 0;
}

MobyPointLight* func_L00_00256F14(const float* position, float* distance_squared) {
    unsigned int x = (unsigned int)ml_integer(position[0]) >> 4;
    unsigned int y = (unsigned int)ml_integer(position[1]) >> 4;
    unsigned int offset = ml_grid[x + (y << 6)];
    unsigned int* list;
    unsigned int remaining;
    if (!offset) return 0;
    list = (unsigned int*)((unsigned char*)ml_grid + offset);
    remaining = *list++;
    /* Retail's list walk executes once before checking the remaining count. */
    do {
        MobyPointLight* light = &ml_points[*list++];
        float dx = position[0] - light->position[0];
        float dy = position[1] - light->position[1];
        float distance = dx * dx + dy * dy;
        union { float f; unsigned int bits; } test;
        test.f = distance - light->radius_squared;
        if (test.bits & 0x80000000u) {
            *distance_squared = distance;
            return light;
        }
    } while (--remaining);
    return 0;
}

void FUN_L00_00257024(MobyLightObject* object) {
    float blend, distance;
    int index, channel;
    MobyPointLight* point;
    if (func_L00_00256DE0(object->position, &blend, &index) && (ml_regions[index].flags & 1)) {
        MobyLightRegion* region = &ml_regions[index];
        unsigned int weight = (unsigned int)ml_integer(blend * 255.0f);
        unsigned int ambient = 0;
        for (channel = 0; channel < 4; ++channel) {
            unsigned int a = (region->colors[0] >> (channel * 8)) & 255;
            unsigned int b = (region->colors[1] >> (channel * 8)) & 255;
            /* PMULTH expands bytes into words. PADDUB then saturates each
             * byte separately, without carrying the discarded low bytes.
             * Sum the high bytes, not the products before shifting. */
            unsigned int value = ((a * (255 - weight)) >> 8) + ((b * weight) >> 8);
            if (value > 255) value = 255;
            ambient |= value << (channel * 8);
        }
        object->ambient = ambient;
        object->lights = region->lights[0] | (region->lights[1] << 8) | (weight << 16);
    }
    point = func_L00_00256F14(object->position, &distance);
    if (!point) {
        object->color = object->ambient;
        return;
    }
    {
        unsigned int weight = 255u - (unsigned int)ml_integer((distance / point->radius_squared) * 255.0f);
        unsigned int color = 0;
        for (channel = 0; channel < 4; ++channel) {
            unsigned int add = (((point->color >> (channel * 8)) & 255) * weight) >> 8;
            unsigned int value = ((object->ambient >> (channel * 8)) & 255) + add;
            if (value > 255) value = 255;
            color |= value << (channel * 8);
        }
        object->color = color;
    }
}
