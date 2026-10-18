/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * The game's hand-written assembly routines that only drive the console's
 * hardware, answered by the port (hostgen.json, "host_functions"). Each is
 * written from what the retail routine does, never from its bytes. */
#include "rac1.h"
#include "openrac/game_lib.h"
#include "openrac/sound_engine.h"

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* InitDma: sets the DMA controller's priority bit, enables it and clears the
 * tag address of the VIF0, VIF1, GIF, IPU and SPR channels. The port has no
 * DMA controller: the game's display lists are read at sceDmaSend. */
void func_0020C268(void) {}

/* The square root and the reciprocal square root the hand-written vector
 * routines (hand/) use, as the FPU's sqrt.s and rsqrt.s and VU0's vsqrt;
 * translated code calls a named function of its own under the game_ prefix.
 * All three take the root of the magnitude, so a negative argument gives a
 * number where the host's sqrtf gives NaN (a toad's turn spring passes one
 * the first time it turns, and the NaN spread to its position). rsqrt.s by
 * zero gives +-FLT_MAX, like the EE's divide. */
float game_openrac_sqrt(float x) { return sqrtf(fabsf(x)); }
float game_openrac_rsqrt(float a, float b) {
    const float root = sqrtf(fabsf(b));
    const int negative = signbit(a) != signbit(b);
    if (root < 1.17549435e-38f) {
        return negative ? -3.40282347e38f : 3.40282347e38f;
    }
    const float q = a / root;
    if (isinf(q)) {
        return q < 0.0f ? -3.40282347e38f : 3.40282347e38f;
    }
    return q;
}

/* A trap the game raises on purpose (teq $0, $0 in a hand-written routine): an assertion of the
 * game that failed. The console would stop in its exception handler; the port says so and stops. */
void game_openrac_game_trap(void) {
    /* The crash report's backtrace names the functions that led here. */
    openrac_game_crash("the game raised a trap (a failed assertion of its own)");
}

/* VU0_loadMicroProgram (func_002347F0, matched C in the decompilation): waits for VIF0, points it
 * at a microprogram and starts the transfer, then waits again. There is no vector unit 0 in the
 * port, and VIF0's status register is plain memory there, so the game's wait would never end: the
 * routines that ran these programs are the port's own C (hand/). */
void func_002347F0(gaddr program) {
    (void)program;
}

/* The asset readers' WAD decompressor (game/common/lib/wad.cpp). */
uint32_t openrac_lib_wad_decompress(const uint8_t *src, uint8_t *dst, uint32_t capacity);

/* WadDecompress (func_0020C468, hand-written): decompresses the WAD stream at src into dst. The
 * retail routine streams through the scratchpad with its DMA channel and returns the size it wrote,
 * which the game's declarations of it do not read; the port decompresses straight from game
 * memory. Its output may run to the end of main memory. */
int func_0020C468(int src, int dst) {
    const uint32_t end_of_ram = 0x02000000u;
    const gaddr to = (gaddr)dst;
    const uint32_t made = openrac_lib_wad_decompress((const uint8_t *)G(src), (uint8_t *)G(to),
                                                     to < end_of_ram ? end_of_ram - to : 0);
    if (getenv("OPENRAC_TRACE_WAD")) {
        const uint8_t *in = (const uint8_t *)G(src);
        fprintf(stderr, "[debug] WAD %08x -> %08x: %02x%02x%02x%02x, %u bytes\n", (unsigned)src,
                (unsigned)to, in[0], in[1], in[2], in[3], (unsigned)made);
    }
    return (int)made;
}

/*
 * FastCos and FastSin. The retail routines hand the angle to a VU0 microprogram (the patch
 * InitOnce uploads from D_0010E4C0 to VU0 address 0xC80): the entry at 0xC80 adds pi/2 and goes on
 * into the sine at 0xC90, which folds the angle into [-pi/2, pi/2] (max(min(x, pi - x), -pi - x))
 * and sums x + c0 x^3 + c1 x^5 + c2 x^7 + c3 x^9 in that order. (ReRAC's moby_light.rs describes
 * the same microprogram, as a reference.) The constants are the microprogram's own: they are read
 * from its I-register immediates in the game's memory the first time, so nothing of the game is
 * copied here; the Taylor terms stand in only if they cannot be found.
 */


/* A word of the VU0 patch microprogram in the game's memory (D_0010E4C0). */
static unsigned word(int i) {
    unsigned w;
    memcpy(&w, G(RAC1_ADDR(0x0010E4C0u) + (gaddr)i * 4u), 4);
    return w;
}

static float as_float(unsigned w) {
    float f;
    memcpy(&f, &w, 4);
    return f;
}

static float sincos_half_pi = 1.57079625f;
static float sincos_pi = 3.1415925f;
static float sincos_c[4] = {-1.0f / 6.0f, 1.0f / 120.0f, -1.0f / 5040.0f, 1.0f / 362880.0f};
static int sincos_ready;

/* True when `v` lies within a tenth of a percent of `want`. */
static int sincos_near(float v, float want) {
    float d = v - want;
    float tolerance = want * 0.001f;
    if (tolerance < 0.0f) {
        tolerance = -tolerance;
    }
    return d <= tolerance && d >= -tolerance;
}

static void sincos_init(void) {
    int i;
    int found = 0;

    sincos_ready = 1;
    /* The program follows its MPG code (0x4A, 0x5E instructions, at 0x190); the immediates are the
       lower words of the pairs whose upper word has the I bit (bit 31) set. */
    for (i = 0; i < 64; i++) {
        if (word(i) == 0x4A5E0190u) {
            break;
        }
    }
    if (i == 64) {
        return;
    }
    i++;
    if (i & 1) {
        i++;
    }
    for (; i + 1 < 64 + 0x5E * 2; i += 2) {
        unsigned upper = word(i + 1);
        float v;

        if ((upper & 0x80000000u) == 0) {
            continue;
        }
        v = as_float(word(i));
        if (sincos_near(v, 1.5707963f)) {
            sincos_half_pi = v;
            found |= 1;
        } else if (sincos_near(v, 3.1415926f)) {
            sincos_pi = v;
            found |= 2;
        } else if (sincos_near(v, -1.0f / 6.0f)) {
            sincos_c[0] = v;
            found |= 4;
        } else if (sincos_near(v, 1.0f / 120.0f)) {
            sincos_c[1] = v;
            found |= 8;
        } else if (sincos_near(v, -1.0f / 5040.0f) || (v < -1.9e-4f && v > -2.0e-4f)) {
            sincos_c[2] = v;
            found |= 16;
        } else if (v > 2.5e-6f && v < 2.9e-6f) {
            sincos_c[3] = v;
            found |= 32;
        }
    }
    (void)found;
}

/* The microprogram's sine of an angle in radians (entry 0xC90). */
static float vu0_sine(float a) {
    float x, folded, x2, x3, x5, x7, x9, sum;

    if (!sincos_ready) {
        sincos_init();
    }
    /* min(x, pi - x), then max with -pi - x */
    folded = sincos_pi - a;
    x = a < folded ? a : folded;
    folded = -sincos_pi - a;
    x = x > folded ? x : folded;
    x2 = x * x;
    x3 = x * x2;
    x5 = x3 * x2;
    x7 = x5 * x2;
    x9 = x7 * x2;
    sum = x;
    sum = sum + x3 * sincos_c[0];
    sum = sum + x5 * sincos_c[1];
    sum = sum + x7 * sincos_c[2];
    sum = sum + x9 * sincos_c[3];
    return sum;
}

/* FastCos(a): the sine of a + pi/2 (entry 0xC80). */
float func_001F9F90(float a) {
    if (!sincos_ready) {
        sincos_init();
    }
    return vu0_sine(a + sincos_half_pi);
}

/* FastSin(a) (entry 0xC90). */
float func_001F9FA8(float a) {
    return vu0_sine(a);
}

/*
 * The game's movie player (the boot logos, the story movies between levels): `a0` the movie's first
 * sector on the disc, `a1` its size in bytes, then the two buffers the console decodes into, which the
 * port does not need. Played in the window by the port's MPEG player (game/common, media/), in the
 * language channel 0; Start skips it, as readMpeg lets it outside normal play.
 */
int func_0023B670(int a0, int a1, int a2, gaddr a3, int a4) {
    (void)a2;
    (void)a3;
    (void)a4;
    openrac_game_play_movie((uint32_t)a0, (uint32_t)a1, 0, 1);
    return 0;
}

/*
 * The VIF1 DMA interrupt handler (hand-written assembly, installed by DMAC_VIF1_Enable). On the
 * console a display list's fence tags interrupt here and each clears its bit in the fence word at
 * D_00160FE0 that the game waits on; the end of the list clears all five. The port sends no list to
 * hardware (the renderer takes it as it is), so every transfer has ended at once.
 */
void func_00235118(void) {
    uint32_t fences;
    memcpy(&fences, G(OPENRAC_DATA(RAC1_ADDR(0x00160FE0u))), 4);
    fences &= ~0x1Fu;
    memcpy(G(OPENRAC_DATA(RAC1_ADDR(0x00160FE0u))), &fences, 4);
}

/*
 * printf (newlib, in the executable): the game's debug output. Its format string goes to the log
 * as it is, without the arguments, until the formatter (sprintf is translated) is shared with it.
 */
int func_00116078(gaddr format, ...) {
    const char *text = (const char *)G(format);
    fprintf(stderr, "[game] %s", text);
    return 0;
}

/*
 * snd_FlushSoundCommands (989snd, matched C in the decompilation, replaced here). There is no
 * IOP: a finished disc read is reported through the shared library, and the commands in the
 * buffer being filled go to the port's sound engine (openrac/sound_engine.h), each callback then
 * called with its reply as the IOP's answer would be. The buffer is left as snd_SendCurrentBatch
 * leaves it (no command, all its 0xFFC bytes free), so the game never waits for the IOP. It is
 * emptied before the commands run, as a callback may send more. PAL addresses: the buffer in use
 * D_0015EDC0, the counts D_0015EDA0[2] (pointers; the commands follow the count word, each
 * u16 command, u16 size, data padded to 4 bytes), the space left D_0015EDA8[2], the callbacks
 * D_0015EDB0[2] (pointers to {u32 function, u32, u64 argument}).
 */
int func_0012DDC0(void) {
    uint32_t cur, count_at, callbacks_at, count = 0, used = 0, free_bytes = 0xFFC, zero = 0;
    uint8_t batch[0x1000];
    struct {
        uint32_t function;
        uint32_t pad;
        uint64_t argument;
    } callbacks[0x100];
    openrac_lib_snd_FlushSoundCommands();
    memcpy(&cur, G(RAC1_ADDR(0x0015EDC0u)), 4);
    cur &= 1;
    memcpy(&count_at, G(RAC1_ADDR(0x0015EDA0u) + cur * 4u), 4);
    memcpy(&callbacks_at, G(RAC1_ADDR(0x0015EDB0u) + cur * 4u), 4);
    if (count_at != 0) {
        memcpy(&count, G(count_at), 4);
        memcpy(&used, G(RAC1_ADDR(0x0015EDA8u) + cur * 4u), 4);
        used = used <= 0xFFC ? 0xFFC - used : 0;
        if (count > 0x100) {
            count = 0x100;
        }
        memcpy(batch, G(count_at + 4u), used);
        if (callbacks_at != 0) {
            memcpy(callbacks, G(callbacks_at), count * sizeof callbacks[0]);
        } else {
            memset(callbacks, 0, sizeof callbacks);
        }
        memcpy(G(count_at), &zero, 4);
    }
    memcpy(G(RAC1_ADDR(0x0015EDA8u) + cur * 4u), &free_bytes, 4);
    for (uint32_t i = 0, at = 0; i < count && at + 4 <= used; ++i) {
        uint16_t command, size;
        memcpy(&command, batch + at, 2);
        memcpy(&size, batch + at + 2, 2);
        if (at + 4u + size > used) {
            break;
        }
        const uint32_t reply = openrac_sound_command(command, batch + at + 4, size);
        /* OPENRAC_SOUND_LOG: each command the game sends its sound driver, with its first words and
         * the reply, on stderr. */
        static int sound_log = -1;
        if (sound_log < 0) {
            sound_log = getenv("OPENRAC_SOUND_LOG") != NULL;
        }
        if (sound_log) {
            uint32_t w[4] = {0, 0, 0, 0};
            memcpy(w, batch + at + 4, size < sizeof w ? size : sizeof w);
            fprintf(stderr, "[debug] sound command %#x (%u bytes): %#x %#x %#x %#x -> %#x\n", command, size, w[0],
                    w[1], w[2], w[3], reply);
        }
        at += (4u + size + 3u) & ~3u;
        if (callbacks[i].function != 0) {
            GFN(void (*)(int, long long), callbacks[i].function)((int)reply, (long long)callbacks[i].argument);
        }
    }
    return 0;
}

#if !RAC1_OMIT(game_D_0023B578)
/*
 * The function at 0x0023B578, which the pause menu stores in its mobys (+0x74) as their update:
 * inside what the catalogue calls func_0023B510 (whose start is not code), so the decompilation
 * names it D_0023B578 and has no C for it. If flag 2 of +0x70 is set, +0x54 becomes 0 when +0x58
 * is above 0 and 1 otherwise, and +0x58 is cleared; then func_0023B5D0 runs. Written from its
 * instructions.
 */
void game_D_0023B578(gaddr moby) {
    if (*(uint8_t *)G(moby + 0x70u) & 2) {
        float v;
        float zero = 0.0f, result;
        memcpy(&v, G(moby + 0x58u), 4);
        result = zero < v ? zero : 1.0f;
        memcpy(G(moby + 0x54u), &result, 4);
        memcpy(G(moby + 0x58u), &zero, 4);
    }
    func_0023B5D0(moby);
}
#endif

/*
 * The conversions the game's formats use (d i u x X o c s f e g %, with flags, width and
 * precision), from host arguments: a string argument is a game address. At most `size` - 1
 * characters go to `out`, which is terminated; the count written is returned. sprintf below and
 * the kernel library's prints (rac1-pal/host/kprintf.c) format with it.
 */
int rac1_vformat(char *out, size_t size, const char *f, va_list args) {
    size_t n = 0;

    if (size == 0) {
        return 0;
    }
    while (*f != '\0' && n + 1 < size) {
        char spec[32];
        size_t k = 0;
        char piece[512];
        int made = 0;

        if (*f != '%') {
            out[n++] = *f++;
            continue;
        }
        spec[k++] = *f++;
        while (*f != '\0' && strchr("-+ #0123456789.", *f) != NULL && k + 2 < sizeof(spec)) {
            spec[k++] = *f++;
        }
        while (*f == 'l' || *f == 'h') {
            f++;  /* the game's int and long are passed as host ints here */
        }
        if (*f == '\0') {
            break;
        }
        spec[k++] = *f;
        spec[k] = '\0';
        switch (*f++) {
            case 'd':
            case 'i':
            case 'u':
            case 'x':
            case 'X':
            case 'o':
            case 'c':
                made = snprintf(piece, sizeof(piece), spec, va_arg(args, int));
                break;
            case 's': {
                const gaddr s = (gaddr)va_arg(args, int);
                made = snprintf(piece, sizeof(piece), spec, s != 0 ? (const char *)G(s) : "(null)");
                break;
            }
            case 'f':
            case 'e':
            case 'g':
                made = snprintf(piece, sizeof(piece), spec, va_arg(args, double));
                break;
            case '%':
                made = snprintf(piece, sizeof(piece), "%%");
                break;
            default:
                made = 0;
                break;
        }
        if (made > 0) {
            size_t take = (size_t)made < sizeof(piece) ? (size_t)made : sizeof(piece) - 1;
            if (n + take >= size) {
                take = size - 1 - n;
            }
            memcpy(out + n, piece, take);
            n += take;
        }
    }
    out[n] = '\0';
    return (int)n;
}

/*
 * sprintf (newlib, in the executable). The translated formatter passes a va_list into game
 * memory, which hostgen cannot carry, so the port formats here (rac1_vformat). The result is
 * written into game memory at `buffer`.
 */
int func_00116248(gaddr buffer, gaddr format, ...) {
    char out[1024];
    int n;
    va_list args;

    va_start(args, format);
    n = rac1_vformat(out, sizeof(out), (const char *)G(format), args);
    va_end(args);
    memcpy(G(buffer), out, (size_t)n + 1);
    return n;
}

/*
 * FontPrintWindow in the large (mode 1) and small (mode 2) font: the effect texture's TEX0 from
 * GetEffectTex and the font's glyph table. The decompilation's matching C keeps the TEX0 in an int,
 * which is right on the console (the register holds all 64 bits) but cuts it to 32 here, so the
 * glyphs would be drawn with a wrong palette and size; the port passes it whole.
 */
void func_001F7560(gaddr a, gaddr b, gaddr c, gaddr d) {
    func_001F7070(a, b, c, d, func_001F4868(1), OPENRAC_DATA(RAC1_ADDR(0x001DF3D0u)));
}

void func_001F75D0(gaddr a, gaddr b, gaddr c, gaddr d) {
    func_001F7070(a, b, c, d, func_001F4868(2), OPENRAC_DATA(RAC1_ADDR(0x001DF770u)));
}
