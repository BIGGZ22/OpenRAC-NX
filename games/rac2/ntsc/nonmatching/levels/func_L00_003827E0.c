/* Best small-tier attempt: 524 bytes versus 304. The switch and table-loop control flow,
 * register allocation, and the state-2 stack quadword setup remain different. */
#include "include_asm.h"

typedef int TI128 __attribute__((mode(TI)));
typedef struct {
    unsigned short first;
    unsigned short flags;
    void *table;
    unsigned short second;
    unsigned short reserved;
} AranosEntry;

extern unsigned int D_00139948[];
extern unsigned char D_L00_0028B620[];
extern unsigned char D_L00_002870C0[];
extern AranosEntry D_L00_0021CEF0[];
extern unsigned char *D_L00_001B1E9C __attribute__((sda));
extern unsigned int **D_L00_001AA830 __attribute__((sda));
extern unsigned int *D_L00_001C95D0;
extern void func_002A7750(char *, int, int);
extern void func_002B0A68(char *, int, void *, unsigned char, float, float, float, float);
extern void func_002E5CF8(int, int, char *);
extern void func_0029F338(char *);

void func_L00_003827E0(char *obj) {
    char *child;
    char *table;
    unsigned int *list;
    unsigned int value;
    unsigned int current;
    int i;
    int index;
    TI128 words;
    float *floats;

    child = *(char **)(obj + 0x68);
    switch (*(unsigned char *)(obj + 0x20)) {
    case 0:
        if (D_00139948[0] != 0) {
            for (i = 0; i < 0x1C; i++) {
                index = D_00139948[i];
                D_L00_0021CEF0[i].first = *(unsigned short *)(D_L00_0028B620 + index);
                D_L00_0021CEF0[i].flags = 3;
                D_L00_0021CEF0[i].table = D_L00_002870C0;
                D_L00_0021CEF0[i].second = *(unsigned short *)(D_L00_0028B620 + index + 4);
            }
        }
        D_L00_0021CEF0[i].first = 0;
        *(int *)(obj + 0x40) = 0;
        *(unsigned int *)(obj + 0x30) |= 0x8000;
        list = *D_L00_001AA830;
        if (list != 0) {
            if (*list == (unsigned int)D_L00_001C95D0) {
                *(int *)(obj + 0x40) = 0;
            } else {
                for (i = 1; list[i] != 0; i++) {
                    if (list[i] == (unsigned int)D_L00_001C95D0) {
                        *(int *)(obj + 0x40) = i;
                        break;
                    }
                }
            }
        }
        func_002A7750(obj, 1, -1);
        return;
    case 1:
        if (*(int *)(child + 4) > 0 && *(int *)(child + 8) > 0) {
            table = D_L00_001B1E9C;
            value = *(unsigned char *)(table + (*(int *)(child + 4) << 8) + 0x20);
            if (table == 0 || value == 0xFE || value == 0xFD) {
                func_002A7750(obj, 2, -1);
            } else {
                value = *(unsigned char *)(table + (*(int *)(child + 8) << 8) + 0x20);
                if (value == 0xFE || value == 0xFD) func_002A7750(obj, 2, -1);
            }
        }
        return;
    case 2:
        words = 2;
        floats = (float *)&words;
        floats[1] = 0.1f;
        func_002B0A68(obj, 1, &words, 0xFF, 3.0f, 6.0f, 3.0f, 6.0f);
        func_002E5CF8(0, 0, obj);
        func_0029F338(obj);
        return;
    default:
        return;
    }
}
