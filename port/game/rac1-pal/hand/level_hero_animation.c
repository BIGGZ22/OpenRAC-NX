/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Native-only reconstruction of PAL 00232EF0..00233410, reviewed against
 * nonmatching/shared/func_L00_00232EF0.c and the complete retail body.
 * Pointer fields are explicit so hostgen keeps the EE's 32-bit layout.
 * This implementation is not a byte-matched PS2 decompilation.
 */
typedef struct HeroAnimClass {
    unsigned char pad00[0x48];
    unsigned char* sequences[256];
} HeroAnimClass;

typedef struct HeroAnimObject {
    unsigned char pad00[0x24];
    HeroAnimClass* model;
    unsigned char pad28[0x28];
    unsigned char key, next_key, sequence, next_sequence;
    float fraction;
    unsigned char pad58[0x10];
    float *key_data, *next_key_data;
    unsigned char pad70[12];
    unsigned char sound, voice, event_count, pad7F;
} HeroAnimObject;

extern unsigned char hero_anim_state[] __asm__("D_0013E633");
extern float hero_anim_curves[] __asm__("D_L00_0017BEF0");
extern float hero_anim_restart_rate __asm__("D_L00_0015F7E8");
extern int hero_anim_mode __asm__("D_L00_0015F6A8");
extern void hero_anim_tick(void) __asm__("func_L00_00211F68");
extern void hero_anim_restart(void) __asm__("func_L00_00232EA8");
extern void hero_anim_rebind(void) __asm__("func_L00_00232B78");
extern void hero_anim_update_a(void) __asm__("func_L00_00232980");
extern void hero_anim_update_b(void) __asm__("func_L00_00232B20");
extern int hero_anim_truncate(float) __asm__("func_001FA898");

/* The EE FPU's divide (openrac_fdiv in the runtime's guest.h, which this file is built without in
 * the tests): a zero divisor gives +-FLT_MAX with the operands' signs XORed, never an infinity. */
/* A float as the EE's FPU takes it: no infinities or NaNs, an exponent of 255 is a number beyond
 * FLT_MAX that its arithmetic saturates to +-FLT_MAX. The restart rate (D_L00_0015F7E8) is
 * 0x7F800000, "at once" to the EE and infinity to the host, where it made the fraction NaN and
 * froze Ratchet in the wrench throw's pose. */
static float hero_anim_ee(float a) {
    union { float f; unsigned int u; } x;
    x.f = a;
    if ((x.u & 0x7F800000u) == 0x7F800000u) {
        return (x.u & 0x80000000u) != 0 ? -3.40282347e38f : 3.40282347e38f;
    }
    return a;
}

static float hero_anim_divide(float a, float b) {
    union { float f; unsigned int u; } x, y;
    float q;
    x.f = a;
    y.f = b;
    if ((y.u & 0x7F800000u) == 0) {  /* zero or a denormal, which the EE reads as zero */
        return ((x.u ^ y.u) & 0x80000000u) != 0 ? -3.40282347e38f : 3.40282347e38f;
    }
    q = a / b;
    if (q > 3.40282347e38f) {
        return 3.40282347e38f;
    }
    if (q < -3.40282347e38f) {
        return -3.40282347e38f;
    }
    return q;
}
extern int hero_anim_play(int, int, void*) __asm__("func_0022ED80");
extern void hero_anim_stop(int) __asm__("func_L00_0028EBF0");
extern float hero_anim_position(void*) __asm__("func_0020D830");

static float* hero_anim_key(HeroAnimObject* r, unsigned char key) {
    unsigned char* sequence = r->model->sequences[r->next_sequence];
    return ((float**)(sequence + 0x1C))[key];
}

void func_L00_00232EF0(void) {
    unsigned char* g = hero_anim_state + 0xE1D;
    HeroAnimObject* r = *(HeroAnimObject**)(g + 0x2080);
    unsigned char previous_key;
    float old, delta, position, previous_position, restart_rate;
    int same, i, low, high, voice;
    unsigned char *sequence, *entry;
    int* events;

    hero_anim_tick();
    *(int*)(g + 0xA98) &= -4;
    previous_key = r->key;
    old = r->fraction;
    same = r->sequence == r->next_sequence;
    if (same) {
        r->fraction = hero_anim_ee(old + *(float*)(g + 0xA90) * *(float*)(g + 0xA94));
    } else if (*(int*)(g + 0xAA0) >= 0) {
        i = *(int*)(g + 0xAA4);
        r->fraction = hero_anim_curves[*(int*)(g + 0xAA0) * 25 + i];
        *(int*)(g + 0xAA4) = i + 1;
    } else {
        r->fraction = old + *(float*)(g + 0xA94);
    }
    if (r->fraction > 0.99f && r->fraction < 1.01f) {
        r->fraction = 1.0f;
    }
    if (r->fraction > -0.01f && r->fraction < 0.01f) {
        r->fraction = 0.0f;
    }
    delta = r->fraction - old;

    if (r->fraction >= 1.0f) {
        restart_rate = hero_anim_ee(hero_anim_restart_rate);
        while (r->fraction >= 1.0f) {
            if (r->sequence != r->next_sequence) {
                r->sequence = r->next_sequence;
                r->event_count = r->model->sequences[r->next_sequence][0x12];
            }
            *(int*)(g + 0xAA0) = -1;
            *(int*)(g + 0xA98) |= 1;
            r->key = r->next_key;
            r->key_data = r->next_key_data;
            ++r->next_key;
            if (*(int*)(g + 0xAB8) != 0) {
                *(int*)(g + 0xAB8) = 0;
                *(float*)(g + 0xA94) = restart_rate;
                r->fraction = 0.0f;
                r->next_key = g[0xAB4];
                r->next_key_data = hero_anim_key(r, r->next_key);
                hero_anim_restart();
                hero_anim_rebind();
            } else if (*(int*)(g + 0xAB0) != -1 && *(int*)(g + 0xAB4) < r->next_key) {
                *(float*)(g + 0xA94) = 0.33333334f;
                r->fraction = 0.0f;
                r->next_key = g[0xAB0];
                r->next_key_data = hero_anim_key(r, r->next_key);
                hero_anim_rebind();
            } else {
                sequence = r->model->sequences[r->next_sequence];
                if (r->next_key >= sequence[0x10]) {
                    r->next_key = 0;
                    *(int*)(g + 0xA98) |= 2;
                }
                r->fraction -= 1.0f;
                r->next_key_data = hero_anim_key(r, r->next_key);
                /* The EE's divide (a rate of 0 gives FLT_MAX, not infinity): hand C is compiled
                 * as it is, not lowered by hostgen, which does this for the game's own C. */
                r->fraction = hero_anim_divide(r->fraction, *(float*)(g + 0xA94));
                *(float*)(g + 0xA94) = *r->key_data;
                r->fraction *= *(float*)(g + 0xA94);
            }
        }
    }

    if (same && r->event_count != 0) {
        high = (r->key << 4) + hero_anim_truncate(r->fraction * 16.0f);
        low = (previous_key << 4) + hero_anim_truncate(old * 16.0f);
        sequence = r->model->sequences[r->sequence];
        events = (int*)((float**)(sequence + 0x1C) + sequence[0x10]);
        for (i = 0; i < sequence[0x12]; ++i) {
            int event = events[i];
            /* Retail emits only the first event in (old, new]. */
            if (low < (event >> 16) && high >= (event >> 16)) {
                hero_anim_play(event & 0xFFFF, 0, r);
                break;
            }
        }
    }

    voice = r->voice;
    if (voice != 255) {
        entry = hero_anim_state + 0x1D + voice * 0x70;
        if (*(HeroAnimObject**)(entry + 0x88) != r) {
            r->voice = 255;
        } else if (*(short*)(entry + 0x7E) != r->sound) {
            hero_anim_stop(voice);
            r->voice = 255;
        }
    } else if (r->sound != 255 && hero_anim_mode != 2 && hero_anim_mode != 6) {
        r->voice = (unsigned char)hero_anim_play(r->sound, 4, r);
    }
    hero_anim_update_a();
    hero_anim_update_b();
    previous_position = *(float*)(g + 0xAA8);
    position = hero_anim_position(r);
    *(float*)(g + 0xAA8) = position;
    *(float*)(g + 0xAAC) = previous_position <= position ? position - previous_position : delta;
    *(int*)(g + 0xA9C) = r->sequence != r->next_sequence;
}
