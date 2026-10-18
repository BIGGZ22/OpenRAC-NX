/* functional: high. Checked against the US assembly (0x217B88..0x217C10): D_001600AC pool, D_001600B4 high, D_001600D0 cursor, D_001600D4 end (gp-27476, -27468, -27440, -27436), D_001CDD80 callback table, 64-byte stride, active byte +1 signed, cursor and end reloaded after each call. */
/* Carried from port/game/rac1-pal/hand/game_particle_update.c through the version map (../README.md): names and
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
/* Native-only PAL handwritten 00218A80..00218B08, including the loop at
 * 00218AA8. gp=166D00: -6B54 pool, -6B4C high, -6B30 cursor, -6B2C end.
 * C keeps its return address on the host stack; particle state remains in
 * the retail globals. No PS2 matching claim. */
typedef struct NativeParticleUpdate {
    signed char type, active;
    unsigned char payload[62];
} NativeParticleUpdate;
extern NativeParticleUpdate* pu_pool __asm__("D_001600AC");
extern int pu_high __asm__("D_001600B4");
extern NativeParticleUpdate* pu_cursor __asm__("D_001600D0");
extern NativeParticleUpdate* pu_end __asm__("D_001600D4");
extern void (*pu_callbacks[])(NativeParticleUpdate*) __asm__("D_001CDD80");

void FUN_00217b88(void) {
    NativeParticleUpdate* next = pu_pool;
    NativeParticleUpdate* end = next + (pu_high + 1);
    pu_end = end;
    while (next != end) {
        NativeParticleUpdate* particle = next++;
        if (particle->active < 0) continue;
        pu_cursor = next;
        pu_callbacks[particle->type](particle);
        /* Retail reloads both after dispatch. The callbacks may remove,
         * add or redirect work through these globals. Do not hoist them. */
        next = pu_cursor;
        end = pu_end;
    }
}
