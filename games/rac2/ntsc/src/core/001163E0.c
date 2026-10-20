#include "common.h"

/* newlib's strstr, the "less than optimal" search rac1-decomp has as
   src/core/00116378.c: func_001163E0. The file it came from is an SDK unit, but
   this body only reproduces under the game compiler, which is the one that pads
   short loops (the retail assembler's rule). */
char *func_001163E0(const char *searchee, const char *lookfor) {
    unsigned int i;

    if (*searchee == 0) {
        if (*lookfor)
            return (char *)0;
        return (char *)searchee;
    }
    while (*searchee) {
        i = 0;
        while (1) {
            if (lookfor[i] == 0)
                return (char *)searchee;
            if (lookfor[i] != searchee[i])
                break;
            i++;
        }
        searchee++;
    }
    return (char *)0;
}
