#include "common.h"
#include "structs.h"

/* Count the records of the chain at `record` that are usable: each record
   carries its own size in word 0 and the chain ends at a zero size, exactly
   as the catalogue walker func_00123978 next door. */
int func_00123930(int *record) {
    int count = 0;
    if (record[0] != 0) {
        do {
            if (record[1] != 0 && record[2] != 0) {
                count++;
            }
            record = (int *)((char *)record + record[0] + 4);
        } while (record[0] != 0);
    }
    return count;
}
