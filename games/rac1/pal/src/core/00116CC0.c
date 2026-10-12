#include "common.h"
#include "callees.h"
#include "structs.h"

/*
 * core_text object 0x116CC0-0x116D30. Boundaries are retail's linker fill
 * (0xCDCDCDCD) between objects; see docs/DECOMP_PROGRESS.md.
 *
 * newlib's strstr.o (the SDK's libc.a). Built with Sony's 2.9-ee
 * (Makefile.sn, EE29_CORE), like libc.a.
 */

/* Declarations in scope here before the split. */
extern long func_00116F68(int arg0, int arg1, int arg2);
extern int D_0015ED10;
extern void *D_0012F86C NOT_SDA;
extern int func_001162B8(void *arg0, void *arg1, void *arg2);
extern int func_00116320(void *arg0, void *arg1, void *arg2);
extern long func_001163A0(void *arg0, void *arg1, void *arg2);
extern void func_00116408(void *arg0);
extern void func_00113968(void);
extern void func_00114438(void *, void *);
extern char D_00152470[];
extern int func_00119088();
extern long func_00116108_wide(int *errOut, void *a, void *b, void *c)
    __asm__("func_00116108");
extern long func_001188C8_wide(int *errOut, void *a, void *b, void *c)
    __asm__("func_001188C8");
extern long func_00114518_wide(int *errOut, void *a, void *b, void *c)
    __asm__("func_00114518");
extern int func_00112468(int *errOut, int arg1);

/*
 * strstr, newlib's text. Exact under 2.9-ee. (A goto-structured decode of
 * the same algorithm was reverted under 2.95.3, which duplicated the
 * `return 0` / `return searchee` tails instead of sharing them.)
 */
char *func_00116CC0(const char *searchee, const char *lookfor) {
    if (*searchee == 0)
    {
        if (*lookfor)
            return (char *) 0;
        return (char *) searchee;
    }

    while (*searchee)
    {
        unsigned int i;
        i = 0;

        while (1)
        {
            if (lookfor[i] == 0)
            {
                return (char *) searchee;
            }

            if (lookfor[i] != searchee[i])
            {
                break;
            }
            i++;
        }
        searchee++;
    }

    return (char *) 0;
}

INCLUDE_ASM("asm/nonmatchings/core_text", func_00116D2C);
