/* Carried from port/game/rac1-pal/hand/game_particle_allocate.c through the version map (../README.md): names and
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
/* Native-only C for handwritten PAL 00218928/00218930 through 00218A74.
 * Retail jumps over the reverse/reuse search and secondary bitmap writes;
 * a1 has no effect on either public entry. Exhaustion returns NULL.
 * The repeated stores to +0x10 really clear only the first 32 bytes of a
 * 64-byte record. Do not silently clear the remaining particle payload.
 * gp is 0x166C00: -6B54 pool, -6B50 next, -6B4C high, -6B48 count.
 */
extern unsigned char* particle_pool __asm__("D_001600AC");
extern int particle_next __asm__("D_001600B0");
extern int particle_high __asm__("D_001600B4");
extern int particle_count __asm__("D_001600B8");
extern unsigned char particle_bits[] __asm__("D_001CD780");

void* func_00217A38(int type, int unused) {
    int slot = particle_next, next, i;
    unsigned char* result;
    (void)unused;
    if (slot >= 2048) {
        return 0;
    }
    particle_bits[(unsigned int)slot >> 3] |= (unsigned char)(1u << (slot & 7));
    next = slot + 1;
    if (slot <= particle_high) {
        /* The retail scan starts at bit zero of the containing byte. */
        next = (int)((unsigned int)next & ~7u);
        while (next < 2048 && (particle_bits[(unsigned int)next >> 3] & (1u << (next & 7)))) {
            ++next;
        }
    }
    particle_next = next;
    if (slot > particle_high) {
        particle_high = slot;
    }
    ++particle_count;
    result = particle_pool + slot * 64;
    for (i = 0; i < 32; ++i) {
        result[i] = 0;
    }
    result[0] = (unsigned char)type;
    return result;
}

void* FUN_00217a30(int type) {
    return func_00217A38(type, 0);
}
