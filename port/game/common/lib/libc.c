/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * The C library's memory and string functions the game calls whose code is
 * still assembly in the decompilation (libraries.tsv): the host's own, on
 * game memory. */
#include "openrac/game_host.h"
#include "openrac/game_lib.h"

/* memcmp */
int openrac_lib_memcmp(gaddr a, gaddr b, unsigned int n) {
    return memcmp(G(a), G(b), n);
}

/* memcpy */
gaddr openrac_lib_memcpy(gaddr dst, gaddr src, unsigned int n) {
    memmove(G(dst), G(src), n);
    return dst;
}

/* memset */
gaddr openrac_lib_memset(gaddr dst, int c, unsigned int n) {
    memset(G(dst), c, n);
    return dst;
}

/* strchr */
gaddr openrac_lib_strchr(gaddr s, int c) {
    const char* host = (const char*)G(s);
    const char* at = strchr(host, c);
    return at == NULL ? 0 : s + (gaddr)(at - host);
}

/* strcmp */
int openrac_lib_strcmp(gaddr a, gaddr b) {
    return strcmp((const char*)G(a), (const char*)G(b));
}

/* strcpy */
gaddr openrac_lib_strcpy(gaddr dst, gaddr src) {
    memmove(G(dst), G(src), strlen((const char*)G(src)) + 1);
    return dst;
}

/* strlen */
int openrac_lib_strlen(gaddr s) {
    return (int)strlen((const char*)G(s));
}

/* strncmp */
int openrac_lib_strncmp(gaddr a, gaddr b, int n) {
    return strncmp((const char*)G(a), (const char*)G(b), (size_t)n);
}

/* strncpy */
gaddr openrac_lib_strncpy(gaddr dst, gaddr src, int n) {
    strncpy((char*)G(dst), (const char*)G(src), (size_t)n);
    return dst;
}

/* __extendsfdf2: float to double, which the console did in software. */
double openrac_lib___extendsfdf2(float f) {
    return (double)f;
}

/* __muldi3: the low 64 bits of a 64-bit product, which the console's 32-bit
 * multiplier built from halves (libgcc). Unsigned, so it wraps as there. */
long long openrac_lib___muldi3(long long a, long long b) {
    return (long long)((unsigned long long)a * (unsigned long long)b);
}

/* __main: what the console's start-up ran before main (global constructors;
 * the game has none). */
void openrac_lib___main(void) {}
