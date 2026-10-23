/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * The disc library (libcdvd). The game reads its data by absolute sector
 * from the disc (docs/port/RAC1_PAL_SURVEY.md, section 6), so the port reads
 * the same sectors from the player's disc image that the extractor kept
 * (iso_data/<game>/disc.iso). A read completes before the call returns, so the
 * game's polling (sceCdSync) finds it done. */
#define _FILE_OFFSET_BITS 64
#include <stdio.h>
#include <time.h>

#include "openrac/game_host.h"
#include "openrac/game_lib.h"
#include "openrac/host_env.h"

#define SECTOR 2048
#define MAX_PATCHES 64

static FILE* disc;
static int last_error;

/* Edited levels (--mods): runs of sectors read from a file instead of the
 * disc. patches.tsv has one per line, "first sector<TAB>sectors<TAB>file",
 * the file beside the list; lines starting with # are skipped. */
static struct {
    unsigned first, count;
    FILE* file;
} patches[MAX_PATCHES];
static int patch_count = -1; /* -1: the list has not been read yet */

static void load_patches(void) {
    char path[4096], line[4096], name[2048];
    FILE* list;
    patch_count = 0;
    if (openrac_game_mods_dir == NULL) {
        return;
    }
    snprintf(path, sizeof path, "%s/patches.tsv", openrac_game_mods_dir);
    list = fopen(path, "r");
    if (list == NULL) {
        fprintf(stderr, "[error] no list of edited levels at %s\n", path);
        return;
    }
    while (fgets(line, sizeof line, list) != NULL && patch_count < MAX_PATCHES) {
        unsigned first, count;
        if (line[0] == '#' || sscanf(line, "%u\t%u\t%2047[^\r\n]", &first, &count, name) != 3) {
            continue;
        }
        snprintf(path, sizeof path, "%s/%s", openrac_game_mods_dir, name);
        patches[patch_count].file = fopen(path, "rb");
        if (patches[patch_count].file == NULL) {
            fprintf(stderr, "[error] cannot open the edited level %s\n", path);
            continue;
        }
        patches[patch_count].first = first;
        patches[patch_count].count = count;
        fprintf(stderr, "[info] sectors %u to %u come from %s\n", first, first + count - 1, path);
        patch_count++;
    }
    fclose(list);
}

/* Overwrites the sectors of a finished disc read that an edited level replaces. */
static void apply_patches(unsigned sector, unsigned count, unsigned char* out) {
    int p;
    unsigned i;
    if (patch_count < 0) {
        load_patches();
    }
    for (p = 0; p < patch_count; p++) {
        for (i = 0; i < count; i++) {
            unsigned s = sector + i;
            if (s < patches[p].first || s >= patches[p].first + patches[p].count) {
                continue;
            }
            if (fseek(patches[p].file, (long)(s - patches[p].first) * SECTOR, SEEK_SET) != 0
                || fread(out + (size_t)i * SECTOR, 1, SECTOR, patches[p].file) != SECTOR) {
                fprintf(stderr, "[error] the edited level is too short for sector %u\n", s);
            }
        }
    }
}

static FILE* open_disc(void) {
    if (disc == NULL && openrac_game_disc_image != NULL) {
        disc = fopen(openrac_game_disc_image, "rb");
        if (disc == NULL) {
            fprintf(stderr, "[error] cannot open the disc image %s\n", openrac_game_disc_image);
        }
    }
    return disc;
}

/* sceCdInit */
int openrac_lib_sceCdInit(int mode) {
    (void)mode;
    return open_disc() != NULL;
}

/* sceCdDiskReady: 2 is "ready". */
int openrac_lib_sceCdDiskReady(int mode) {
    (void)mode;
    return open_disc() != NULL ? 2 : 6;
}

/* sceCdMmode: the media type (DVD); nothing to set. */
int openrac_lib_sceCdMmode(int media) {
    (void)media;
    return 1;
}

/* sceCdRead(sector, count, buffer, mode) */
int openrac_lib_sceCdRead(unsigned int sector, unsigned int count, gaddr buffer, gaddr mode) {
    (void)mode;
    FILE* f = open_disc();
    /* OPENRAC_TRACE_CD=1: each read, to see where the game's data lands. */
    if (openrac_env_string("OPENRAC_TRACE_CD") != NULL) {
        fprintf(stderr, "[debug] CD read %u sectors at %u -> %08x\n", count, sector, (unsigned)buffer);
    }
    if (f == NULL) {
        last_error = 0x01; /* any non-zero error: the game retries */
        return 0;
    }
#if defined(_WIN32)
    /* Windows' C library has no fseeko; _fseeki64 is its 64-bit seek. */
    if (_fseeki64(f, (long long)sector * SECTOR, SEEK_SET) != 0
#else
    if (fseeko(f, (off_t)sector * SECTOR, SEEK_SET) != 0
#endif
        || fread(G(buffer), SECTOR, count, f) != count) {
        fprintf(stderr, "[error] disc read of %u sectors at %u failed\n", count, sector);
        last_error = 0x32;
        return 0;
    }
    apply_patches(sector, count, (unsigned char*)G(buffer));
    last_error = 0;
    return 1;
}

/* sceCdSync: 0 is "done" (every read is). */
int openrac_lib_sceCdSync(int mode) {
    (void)mode;
    return 0;
}

/* sceCdGetError */
int openrac_lib_sceCdGetError(void) {
    return last_error;
}

/* sceCdBreak */
int openrac_lib_sceCdBreak(void) {
    return 1;
}

static uint8_t bcd(int v) {
    return (uint8_t)(((v / 10) << 4) | (v % 10));
}

/* sceCdReadClock: the console's clock, in BCD: status, second, minute, hour,
 * a pad byte, day, month, year (two digits). */
int openrac_lib_sceCdReadClock(gaddr clock) {
    time_t now = time(NULL);
    struct tm local;
#if defined(_WIN32)
    localtime_s(&local, &now);
#else
    localtime_r(&now, &local);
#endif
    uint8_t* c = (uint8_t*)G(clock);
    c[0] = 0;
    c[1] = bcd(local.tm_sec);
    c[2] = bcd(local.tm_min);
    c[3] = bcd(local.tm_hour);
    c[4] = 0;
    c[5] = bcd(local.tm_mday);
    c[6] = bcd(local.tm_mon + 1);
    c[7] = bcd(local.tm_year % 100);
    return 1;
}
