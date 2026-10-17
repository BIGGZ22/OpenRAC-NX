/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * The memory card library (libmc). A card is a folder (card_dir for the
 * first port, card_dir.slot2 for the second, which counts as inserted only if the
 * folder exists), and the game's save folder and files are ordinary files in
 * it, so saves can be copied to and from other tools.
 *
 * The library's calls start a command and sceMcSync reports its result; here
 * every command finishes when it is issued, and sceMcSync hands back its
 * result the first time it is polled. The structures and codes are the
 * library's public interface as the game uses them
 * (docs/port/RAC1_PAL_SURVEY.md, section 7). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sys/stat.h>

#if defined(_WIN32)
#include <direct.h>
#include <io.h>
#define make_dir(p) _mkdir(p)
#else
#include <dirent.h>
#include <unistd.h>
#define make_dir(p) mkdir((p), 0755)
#endif

#ifndef S_ISDIR
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif

#include "openrac/game_host.h"
#include "openrac/game_lib.h"

/* The command numbers sceMcSync reports. */
enum {
    MC_GET_INFO = 0x01,
    MC_OPEN = 0x02,
    MC_CLOSE = 0x03,
    MC_SEEK = 0x04,
    MC_READ = 0x05,
    MC_WRITE = 0x06,
    MC_MKDIR = 0x0B,
    MC_CHDIR = 0x0C,
    MC_GET_DIR = 0x0D,
    MC_DELETE = 0x0F,
    MC_FORMAT = 0x10,
    MC_UNFORMAT = 0x11,
};

/* Results. */
#define MC_OK 0
#define MC_NO_CARD (-10)
#define MC_NOT_FOUND (-4)
#define MC_EXISTS (-4)
#define MC_FAILED (-5)

/* Directory entry attributes. */
#define MC_ATTR_FILE 0x8497
#define MC_ATTR_DIR 0x8427

static int pending_cmd;
static int pending_result;
static int have_pending;

static int finish(int cmd, int result) {
    pending_cmd = cmd;
    pending_result = result;
    have_pending = 1;
    return 0; /* the command was started */
}

/* Each card's current folder (sceMcChdir), relative to its root without
 * slashes at either end: "" is the root. A name that does not start with '/'
 * is in it. */
static char current_dir[2][512];

/* The first port's card is card_dir itself, the folder the launcher's save
 * manager shows (launcher/core/src/saves.rs); the second port's is beside it,
 * card_dir.slot2, and counts as inserted only if it exists. */
static int card_path(int port, const char* name, char* out, size_t size) {
    const char* root = openrac_game_card_dir != NULL ? openrac_game_card_dir : "memcard";
    const char* dir = *name == '/' ? "" : current_dir[port != 0];
    while (*name == '/') {
        name++;
    }
    return snprintf(out, size, "%s%s%s%s%s%s", root, port == 0 ? "" : ".slot2", *dir ? "/" : "", dir,
                    *name ? "/" : "", name)
           < (int)size;
}

static int card_present(int port) {
    char path[1024];
    if (openrac_game_no_card) {
        return 0;
    }
    struct stat st;
    if (!card_path(port, "", path, sizeof path)) {
        return 0;
    }
    if (port == 0) {
        make_dir(path); /* the first card is always inserted */
    }
    return stat(path, &st) == 0;
}

/* sceMcInit */
int openrac_lib_sceMcInit(void) {
    return 0;
}

/* sceMcGetInfo(port, slot, type, free, format): a formatted 8 MB card. */
int openrac_lib_sceMcGetInfo(int port, int slot, gaddr type, gaddr free_clusters, gaddr format) {
    (void)slot;
    if (!card_present(port)) {
        return finish(MC_GET_INFO, MC_NO_CARD);
    }
    if (type != 0) {
        GREF(int, type) = 2; /* a PS2 memory card */
    }
    if (free_clusters != 0) {
        GREF(int, free_clusters) = 7000;
    }
    if (format != 0) {
        GREF(int, format) = 1;
    }
    return finish(MC_GET_INFO, MC_OK);
}

/* ---- Files ---- */

#define MAX_FILES 8
static FILE* files[MAX_FILES];

/* sceMcOpen(port, slot, name, mode): mode 1 read, 2 write, 3 both, 0x200 create. */
int openrac_lib_sceMcOpen(int port, int slot, gaddr name, int mode) {
    (void)slot;
    char path[1024];
    if (!card_present(port) || !card_path(port, (const char*)G(name), path, sizeof path)) {
        return finish(MC_OPEN, MC_NO_CARD);
    }
    int fd = -1;
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i] == NULL) {
            fd = i;
            break;
        }
    }
    if (fd < 0) {
        return finish(MC_OPEN, -7);
    }
    FILE* f = NULL;
    if (mode & 0x200) {
        f = fopen(path, "r+b");
        if (f == NULL) {
            f = fopen(path, "w+b");
        }
    } else if (mode & 2) {
        f = fopen(path, "r+b");
    } else {
        f = fopen(path, "rb");
    }
    if (f == NULL) {
        return finish(MC_OPEN, MC_NOT_FOUND);
    }
    files[fd] = f;
    return finish(MC_OPEN, fd);
}

/* sceMcClose(fd) */
int openrac_lib_sceMcClose(int fd) {
    if (fd < 0 || fd >= MAX_FILES || files[fd] == NULL) {
        return finish(MC_CLOSE, MC_FAILED);
    }
    fclose(files[fd]);
    files[fd] = NULL;
    return finish(MC_CLOSE, MC_OK);
}

/* sceMcSeek(fd, offset, whence) */
int openrac_lib_sceMcSeek(int fd, int offset, int whence) {
    if (fd < 0 || fd >= MAX_FILES || files[fd] == NULL || fseek(files[fd], offset, whence) != 0) {
        return finish(MC_SEEK, MC_FAILED);
    }
    return finish(MC_SEEK, (int)ftell(files[fd]));
}

/* sceMcRead(fd, buffer, size) */
int openrac_lib_sceMcRead(int fd, gaddr buffer, int size) {
    if (fd < 0 || fd >= MAX_FILES || files[fd] == NULL) {
        return finish(MC_READ, MC_FAILED);
    }
    return finish(MC_READ, (int)fread(G(buffer), 1, (size_t)size, files[fd]));
}

/* sceMcWrite(fd, buffer, size) */
int openrac_lib_sceMcWrite(int fd, gaddr buffer, int size) {
    if (fd < 0 || fd >= MAX_FILES || files[fd] == NULL) {
        return finish(MC_WRITE, MC_FAILED);
    }
    const size_t n = fwrite(G(buffer), 1, (size_t)size, files[fd]);
    fflush(files[fd]);
    return finish(MC_WRITE, (int)n);
}

/* sceMcMkdir(port, slot, name) */
int openrac_lib_sceMcMkdir(int port, int slot, gaddr name) {
    (void)slot;
    char path[1024];
    struct stat st;
    if (!card_present(port) || !card_path(port, (const char*)G(name), path, sizeof path)) {
        return finish(MC_MKDIR, MC_NO_CARD);
    }
    if (stat(path, &st) == 0) {
        return finish(MC_MKDIR, MC_EXISTS);
    }
    return finish(MC_MKDIR, make_dir(path) == 0 ? MC_OK : MC_FAILED);
}

/* sceMcDelete(port, slot, name): a file, or an empty folder. */
int openrac_lib_sceMcDelete(int port, int slot, gaddr name) {
    (void)slot;
    char path[1024];
    if (!card_present(port) || !card_path(port, (const char*)G(name), path, sizeof path)) {
        return finish(MC_DELETE, MC_NO_CARD);
    }
    if (remove(path) == 0) {
        return finish(MC_DELETE, MC_OK);
    }
#if defined(_WIN32)
    return finish(MC_DELETE, _rmdir(path) == 0 ? MC_OK : MC_NOT_FOUND);
#else
    return finish(MC_DELETE, rmdir(path) == 0 ? MC_OK : MC_NOT_FOUND);
#endif
}

/* sceMcFormat, sceMcUnformat: a folder is always formatted. */
int openrac_lib_sceMcFormat(int port, int slot) {
    (void)slot;
    return finish(MC_FORMAT, card_present(port) ? MC_OK : MC_NO_CARD);
}

int openrac_lib_sceMcUnformat(int port, int slot) {
    (void)slot;
    return finish(MC_UNFORMAT, card_present(port) ? MC_OK : MC_NO_CARD);
}

/* ---- Directories ---- */

/* One entry of sceMcGetDir's table, 64 bytes. */
typedef struct {
    uint8_t created[8]; /* reserved, second, minute, hour, day, month, year (16 bits) */
    uint8_t modified[8];
    uint32_t size;
    uint16_t attributes;
    uint16_t reserved1;
    uint32_t reserved2;
    uint32_t pda;
    char name[32];
} mc_entry;

#define MAX_ENTRIES 64
static mc_entry listing[MAX_ENTRIES];
static int listing_count, listing_next;

static void stamp(uint8_t out[8], time_t t) {
    struct tm local;
#if defined(_WIN32)
    localtime_s(&local, &t);
#else
    localtime_r(&t, &local);
#endif
    out[0] = 0;
    out[1] = (uint8_t)local.tm_sec;
    out[2] = (uint8_t)local.tm_min;
    out[3] = (uint8_t)local.tm_hour;
    out[4] = (uint8_t)local.tm_mday;
    out[5] = (uint8_t)(local.tm_mon + 1);
    out[6] = (uint8_t)((local.tm_year + 1900) & 0xFF);
    out[7] = (uint8_t)((local.tm_year + 1900) >> 8);
}

static void add_entry(const char* dir, const char* name) {
    char path[1200];
    struct stat st;
    if (listing_count >= MAX_ENTRIES) {
        return;
    }
    snprintf(path, sizeof path, "%s/%s", dir, name);
    mc_entry* e = &listing[listing_count++];
    memset(e, 0, sizeof *e);
    if (stat(path, &st) == 0) {
        stamp(e->created, st.st_mtime);
        stamp(e->modified, st.st_mtime);
        e->size = S_ISDIR(st.st_mode) ? 0 : (uint32_t)st.st_size;
        e->attributes = S_ISDIR(st.st_mode) ? MC_ATTR_DIR : MC_ATTR_FILE;
    }
    snprintf(e->name, sizeof e->name, "%s", name);
}

static int matches(const char* pattern, const char* name) {
    if (strcmp(pattern, "*") == 0) {
        return 1;
    }
    return strcmp(pattern, name) == 0;
}

static void list(int port, const char* spec) {
    char dir_part[512], pattern[64], path[1024];
    const char* slash = strrchr(spec, '/');
    listing_count = listing_next = 0;
    if (slash == NULL) {
        snprintf(dir_part, sizeof dir_part, "%s", "");
        snprintf(pattern, sizeof pattern, "%s", spec);
    } else {
        snprintf(dir_part, sizeof dir_part, "%.*s", (int)(slash - spec), spec);
        snprintf(pattern, sizeof pattern, "%s", slash + 1);
    }
    if (!card_path(port, dir_part, path, sizeof path)) {
        return;
    }
#if defined(_WIN32)
    char query[1100];
    struct _finddata_t found;
    snprintf(query, sizeof query, "%s/*", path);
    intptr_t h = _findfirst(query, &found);
    if (h == -1) {
        return;
    }
    do {
        if (matches(pattern, found.name)) {
            add_entry(path, found.name);
        }
    } while (_findnext(h, &found) == 0);
    _findclose(h);
#else
    DIR* d = opendir(path);
    if (d == NULL) {
        return;
    }
    struct dirent* ent;
    while ((ent = readdir(d)) != NULL) {
        if (matches(pattern, ent->d_name)) {
            add_entry(path, ent->d_name);
        }
    }
    closedir(d);
#endif
}

/* sceMcGetDir(port, slot, name, mode, max entries, table): mode 0 starts a
 * listing, otherwise it continues one. */
int openrac_lib_sceMcGetDir(
    int port, int slot, gaddr name, unsigned int mode, int max, gaddr table
) {
    (void)slot;
    if (!card_present(port)) {
        return finish(MC_GET_DIR, MC_NO_CARD);
    }
    if (mode == 0) {
        list(port, (const char*)G(name));
    }
    int n = 0;
    while (n < max && listing_next < listing_count) {
        memcpy(G(table + (gaddr)n * sizeof(mc_entry)), &listing[listing_next++], sizeof(mc_entry));
        n++;
    }
    return finish(MC_GET_DIR, n);
}

/* sceMcChdir(port, slot, path, old path): enters a folder of the card, from
 * its root when the path starts with '/'; "." and ".." work as usual. A folder
 * that does not exist leaves the current one and reports not found. The old
 * path, when asked for, is written as an absolute one. */
int openrac_lib_sceMcChdir(int port, int slot, gaddr path, gaddr old_path) {
    (void)slot;
    if (!card_present(port)) {
        return finish(MC_CHDIR, MC_NO_CARD);
    }
    char* cur = current_dir[port != 0];
    if (old_path != 0) {
        snprintf((char*)G(old_path), 1024, "/%s", cur);
    }
    const char* p = (const char*)G(path);
    char next[512];
    snprintf(next, sizeof next, "%s", *p == '/' ? "" : cur);
    while (*p) {
        while (*p == '/') {
            p++;
        }
        size_t len = strcspn(p, "/");
        if (len == 0 || (len == 1 && p[0] == '.')) {
            /* nothing */
        } else if (len == 2 && p[0] == '.' && p[1] == '.') {
            char* slash = strrchr(next, '/');
            if (slash != NULL) {
                *slash = '\0';
            } else {
                next[0] = '\0';
            }
        } else {
            size_t used = strlen(next);
            if (used + len + 2 > sizeof next) {
                return finish(MC_CHDIR, MC_FAILED);
            }
            snprintf(next + used, sizeof next - used, "%s%.*s", used ? "/" : "", (int)len, p);
        }
        p += len;
    }
    char full[1024];
    struct stat st;
    char saved[512];
    snprintf(saved, sizeof saved, "%s", cur);
    snprintf(cur, sizeof current_dir[0], "%s", next);
    if (!card_path(port, "", full, sizeof full) || stat(full, &st) != 0 || !S_ISDIR(st.st_mode)) {
        snprintf(cur, sizeof current_dir[0], "%s", saved);
        return finish(MC_CHDIR, MC_NOT_FOUND);
    }
    return finish(MC_CHDIR, MC_OK);
}

/* sceMcSync(mode, command, result): 1 when a command has finished (always,
 * once), -1 when none is running. */
int openrac_lib_sceMcSync(int mode, gaddr cmd, gaddr result) {
    (void)mode;
    if (!have_pending) {
        return -1;
    }
    have_pending = 0;
    if (cmd != 0) {
        GREF(int, cmd) = pending_cmd;
    }
    if (result != 0) {
        GREF(int, result) = pending_result;
    }
    return 1;
}
