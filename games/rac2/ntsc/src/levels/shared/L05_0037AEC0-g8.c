#include "include_asm.h"

/* Built with -G8: the spawn counter at 0x1AC4D8 and the object table base at 0x1B251C are
   reached through $gp in retail, so those two bodies live here. */

extern int D_L05_001AC4D8 __attribute__((sda));

extern char *D_L05_001B251C;

extern int D_L05_00225DD0[];

extern int D_L05_00225E50[];

extern int D_L05_00225F50[];

extern char *D_L05_001B2528 __attribute__((nosda));

extern int D_L05_00221D10[];

extern int D_L05_001FB220[];

extern int func_002ED688(char *object);

extern int func_L00_004272A0(void);

extern void func_L05_0037AE40(char *object);

extern void func_L05_0037AEC0(char *object, int flag);

extern void func_0029F338(void *object);

extern void func_L05_0037AFF8(int entry);

extern void func_L05_0037B108(void);

/* Runs the constructor over every registered entry, then clears the two shared counters. */
void func_L05_0037B210(void)
{
    int i;

    for (i = 0; i < D_L05_001AC4D8; i++)
        func_L05_0037AFF8(D_L05_00225DD0[i]);
    D_L05_00221D10[4] = 0;
    D_L05_00221D10[3] = 0;
    func_L05_0037B108();
}

/* Runs the entry's list: every registered object with the same id gets its callback, then
   every object the list names is looked up and set. */
void func_L05_0037AFF8(int entry)
{
    unsigned char *obj = (unsigned char *)entry;
    int count;
    int i;
    unsigned short *p;
    int off;

    if (D_L05_001FB220[obj[0x21]] == 0)
        return;

    count = func_L00_004272A0();
    for (i = 0; i < count; i++) {
        char *e = (char *)D_L05_00225DD0[i];

        if (*(short *)(e + 0xAA) == *(short *)(obj + 0xAA)) {
            void (*fn)(char *) = (void (*)(char *))D_L05_00225F50[i];

            if (fn != 0)
                fn(e);
        }
    }

    p = (unsigned short *)D_L05_001FB220[obj[0x21]];
    do {
        char *base;

        off = (p[0] & 0x7FFF) << 8;
        base = D_L05_001B251C;
        if (func_002ED688(base + off) != 0) {
            base = D_L05_001B251C;
            func_L05_0037AE40(base + off);
        }
    } while (*(short *)p++ >= 0);
}

/* Offers the entry to every registered object with the same id, then walks the entry's list
   and offers each object it names to the handler. */
void func_L05_0037B280(char *object, int handler)
{
    unsigned char *obj = (unsigned char *)object;
    int count;
    int i;
    unsigned short *p;
    int off;

    if (D_L05_001FB220[obj[0x21]] == 0)
        return;

    count = func_L00_004272A0();
    for (i = 0; i < count; i++) {
        char *e = (char *)D_L05_00225DD0[i];

        if (*(short *)(e + 0xAA) == *(short *)(obj + 0xAA)) {
            void (*fn)(char *) = (void (*)(char *))D_L05_00225E50[i];

            if (fn != 0)
                fn(e);
        }
    }

    p = (unsigned short *)D_L05_001FB220[obj[0x21]];
    do {
        char *base;

        off = (p[0] & 0x7FFF) << 8;
        base = D_L05_001B251C;
        if (func_002ED688(base + off) != 0) {
            base = D_L05_001B251C;
            func_L05_0037AEC0(base + off, handler);
        }
    } while (*(short *)p++ >= 0);
}

/* Walks the live-object list and hands the objects of the listed ids to the shared handler. */
extern void func_L05_0037B108(void);

/* Finds the free entry of the level list that belongs to id and spawns it there. */
INCLUDE_ASM("asm/levels", func_L05_0037B440);
