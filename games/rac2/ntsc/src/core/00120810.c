#include "common.h"

/* libgcc libgcc2.c: old_find_exception_handler, the translation-unit-local
   handler search of the pre-DWARF exception tables (GPL version 2-or-later
   with the libgcc linking exception; the same public source the qualified
   compiler is built from, gnu-ee-binutils-gcc-1.1, gcc/libgcc2.c, whose
   revision is the trunk of 1999-09-09). The table is a chain of 12-byte
   entries -- start_region, end_region, exception_handler -- ended by a -1
   start_region. In a tie the later (inner) entry wins. */

typedef struct
{
  void *start_region;
  void *end_region;
  void *exception_handler;
} old_exception_table;

void *func_00120810(void *pc, old_exception_table *table) {
    if (table) {
        int pos;
        int best = -1;

        /* We can't do a binary search because the table isn't guaranteed
           to be sorted from function to function.  */
        for (pos = 0; table[pos].start_region != (void *)-1; ++pos) {
            if (table[pos].start_region <= pc && table[pos].end_region > pc) {
                /* This can apply.  Make sure it is at least as small as
                   the previous best.  */
                if (best == -1 ||
                    (table[pos].end_region <= table[best].end_region &&
                     table[pos].start_region >= table[best].start_region))
                    best = pos;
            }
            /* But it is sorted by starting PC within a function.  */
            else if (best >= 0 && table[pos].start_region > pc)
                break;
        }
        if (best != -1)
            return table[best].exception_handler;
    }

    return (void *)0;
}
