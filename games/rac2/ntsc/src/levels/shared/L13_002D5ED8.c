#include "include_asm.h"

INCLUDE_ASM("asm/levels", func_L13_002D5ED8);

#ifndef RAC2_T_TABLE_389848
#define RAC2_T_TABLE_389848
typedef struct {
    int entries[64];    /* +0x000 */
    int count;          /* +0x100 */
} Table_389848;
#endif

#ifndef RAC2_T_OBJ_389848
#define RAC2_T_OBJ_389848
typedef struct {
    char pad[0xAA];
    short key;          /* +0xAA */
} Obj_389848;
#endif

extern Table_389848 ref_L13_00232450_00;

/* Adds the object's key to the level's list when it is not already there: the count at +0x100
 * bounds a linear search over the words at +0, and an absent key is appended at the count
 * before the count is bumped. */
void func_L13_00389848(Obj_389848 *object)
{
    int found = 0;
    int i;

    for (i = 0; i < ref_L13_00232450_00.count; i++) {
        if (ref_L13_00232450_00.entries[i] == object->key) {
            found = 1;
            break;
        }
    }
    if (found == 0) {
        ref_L13_00232450_00.entries[ref_L13_00232450_00.count] = object->key;
        ++ref_L13_00232450_00.count;
    }
}

typedef struct {
    char pad_00[0x20];        /* +0x000 */
    unsigned char state;      /* +0x020 */
    char pad_21[0x0F];
    unsigned char flag;       /* +0x030 */
    char pad_31[0x37];
    Table_389848 *table;      /* +0x068 */
} Obj_38C1B8;

extern int func_002ACBC8(void *, int);
extern void func_0027D330(int);
extern void func_L00_002B30E8(void);
extern void ref_L13_0038C1B8_00(int, int);
extern char D_00189EA0[];
extern char D_00189E20[];

/* The state byte at +0x20 drives the work: state 0 clears the flag at +0x30 and moves the
 * object to state 1, state 1 walks the sixty-four entries of the table at +0x68 in two
 * halves of thirty-two, and any other state does nothing.  D_00189EA0 is the pointer the
 * entry test hands to func_002ACBC8, and the map selection block D_00189E20 sits 0x80
 * before it. */
void func_L13_0038C1B8(Obj_38C1B8 *a0)
{
    Table_389848 *t = a0->table;
    int *e;
    unsigned char *q;
    unsigned char *m;
    int i;
    int j;
    int v;

    switch (a0->state) {
    case 0:
        a0->state = 1;
        a0->flag = 0xFF;
        break;
    case 1:
        q = (unsigned char *)D_00189E20;
        e = t->entries + 32;
        for (i = 0; i < 32; i++) {
            v = t->entries[i];
            if (v < 0)
                continue;
            if (func_002ACBC8(D_00189EA0, v) == 0)
                continue;
            if (t->count != 0) {
                *(float *)(q + 0x24C8) = 0.25f;
                continue;
            }
            m = (unsigned char *)D_00189EA0 - 0x80;
            if (m[0x22B4] != 0) {
                func_0027D330(10);
                func_L00_002B30E8();
            } else if (*(int *)(m + 0x2294) != 0x74) {
                ref_L13_0038C1B8_00(0x74, 1);
            }
        }
        for (j = 0; j < 32; j++) {
            v = e[j];
            if (v < 0)
                continue;
            if (func_002ACBC8(D_00189EA0, v) == 0)
                continue;
            func_0027D330(10);
            func_L00_002B30E8();
        }
        break;
    }
}

INCLUDE_ASM("asm/levels", func_L13_003C5908);

INCLUDE_ASM("asm/levels", func_L13_003ECD48);

INCLUDE_ASM("asm/levels", func_L13_003ECEA8);

INCLUDE_ASM("asm/levels", func_L13_00406E30);
