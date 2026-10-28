/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef OPENRAC_CARD_STORAGE_H
#define OPENRAC_CARD_STORAGE_H
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#ifndef S_ISDIR
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif
#if defined(_WIN32)
#include <direct.h>
#define card_mkdir(p) _mkdir(p)
#else
#define card_mkdir(p) mkdir((p), 0755)
#endif

/* Create missing parents too. Device roots such as sdmc:/ already exist;
 * never try to create the device name as a directory. */
static int card_directory(const char* path) {
    char part[1024];
    if (!path || !*path || strlen(path) >= sizeof part) return 0;
    struct stat st;
    if (stat(path, &st) == 0) return S_ISDIR(st.st_mode);
    strcpy(part, path);
    char* start = strchr(part, ':');
    start = start ? start + 1 : part;
    for (char* p = start; ; ++p) {
        if (*p != '/' && *p != '\0') continue;
        const char end = *p;
        *p = '\0';
        if (p > start && card_mkdir(part) != 0 && errno != EEXIST) return 0;
        *p = end;
        if (!end) break;
    }
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}
#endif
