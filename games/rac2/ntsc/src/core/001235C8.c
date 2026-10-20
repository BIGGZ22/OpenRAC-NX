#include "common.h"

/* libgcc frame.c: fde_merge (GPL version 2-or-later with the libgcc linking
   exception; the same public source the qualified compiler is built from,
   gnu-ee-binutils-gcc-1.1, gcc/frame.c). Merge the two sorted FDE vectors V1
   and V2, leaving the result in V1. fde_compare is the file's static inline,
   (saddr) x->pc_begin - (saddr) y->pc_begin. */

typedef struct
{
  u32 length;
  s32 CIE_delta;
  void *pc_begin;
  u32 pc_range;
} fde;

typedef struct
{
  fde **array;
  u32 count;
} fde_vector;

static __inline__ int fde_compare(fde *x, fde *y) {
    return (int)x->pc_begin - (int)y->pc_begin;
}

void func_001235C8(fde_vector *v1, const fde_vector *v2) {
    u32 i1, i2;
    fde *fde2;

    i2 = v2->count;
    if (i2 > 0) {
        i1 = v1->count;
        do {
            i2--;
            fde2 = v2->array[i2];
            while (i1 > 0 && fde_compare(v1->array[i1 - 1], fde2) > 0) {
                v1->array[i1 + i2] = v1->array[i1 - 1];
                i1--;
            }
            v1->array[i1 + i2] = fde2;
        } while (i2 > 0);
        v1->count += v2->count;
    }
}
