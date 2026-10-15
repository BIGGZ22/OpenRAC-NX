#include "include_asm.h"

/* Level 23 globals the retail code reaches through $gp. */
extern char *D_L23_001B1484 __attribute__((sda));

extern int D_L23_001B13EC __attribute__((sda));

extern int D_L23_001B1658 __attribute__((sda));

extern int D_L23_001B15A4 __attribute__((sda));

extern int D_L23_001B178C __attribute__((sda));

extern int D_L23_001B1790 __attribute__((sda));

extern int D_L23_001B1794 __attribute__((sda));

extern int D_L23_001B1798 __attribute__((sda));

extern int D_L23_001B3788;

extern int D_L23_001B38C0;

extern int func_002E5CF8(int index, int a1, char *a2);

extern void func_0027DDC0(int, int, int, int);

extern int D_L23_001B1694 __attribute__((sda));

extern int D_L23_001B1698 __attribute__((sda));

extern float D_L23_001B14E4 __attribute__((sda));

extern void func_31A108(int);

extern void func_L23_0041A648(char *);

extern void func_L23_0042AC70(void);

/* Reports whether a word of the object reached through the pointer global is set. */
int func_L23_0041B948(void) {
    return *(int *)(*(char **)(D_L23_001B1484 + 0x68) + 0x604) != 0;
}

/* Zero unless the counter is positive and below the limit. */
int func_L23_0041C0C0(void) {
    if (D_L23_001B1694 > 0 && D_L23_001B1694 < D_L23_001B1698)
        return 1;
    return 0;
}

/* One for the two measured state bytes, zero otherwise. */
int func_L23_0041E260(void) {
    unsigned char state = *(unsigned char *)(D_L23_001B1484 + 0x20);

    if (state == 0x10 || state == 0xC)
        return 1;
    return 0;
}



/* Stores one in the state at +0x68, after the measured shared call. */
void func_L23_0041B8F8(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);

    if (D_L23_001B13EC == 0 && D_L23_001B1484 != 0)
        func_002E5CF8(0x10, 0, D_L23_001B1484);
    *(int *)(state + 0x604) = 1;
}

/* Clears the measured slot of the four state pointers and of the argument state. */
void func_L23_0041BFF0(char *arg0) {
    char *state = *(char **)(D_L23_001B1484 + 0x68);
    int *slot;
    int i;

    slot = *(int **)(state + 0x610);
    if (slot != 0)
        *(int *)((char *)slot + 0x98) = 0;
    slot = *(int **)(state + 0x614);
    if (slot != 0)
        *(int *)((char *)slot + 0x98) = 0;
    slot = (int *)(state + 0x358);
    for (i = 4; i >= 0; i--) {
        if (*slot != 0)
            *(int *)(*slot + 0x98) = 0;
        slot++;
    }
    *(int *)(arg0 + 0x98) = 0;
}

/* Calls the measured shared helper with the measured ratio expression. */
void func_L23_0041E3C8(void) {
    func_0027DDC0(D_L23_001B1790, D_L23_001B1794, D_L23_001B1798,
                  (D_L23_001B15A4 - D_L23_001B3788) * D_L23_001B178C / D_L23_001B15A4);
}

/* Stores the incoming float in the level global reached through $gp. */
void func_L23_0041B960(float arg0) { D_L23_001B14E4 = arg0; }

/* Copies the measured word of every live slot into the object the argument names. */
void func_L23_0041C040(char *arg0) {
    char *state = *(char **)(D_L23_001B1484 + 0x68);
    char *slot;
    char **p;
    int i;

    slot = *(char **)(state + 0x610);
    if (slot != 0)
        *(int *)(slot + 0x98) = *(int *)(*(char **)(slot + 0x24) + 0x10);
    slot = *(char **)(state + 0x614);
    if (slot != 0)
        *(int *)(slot + 0x98) = *(int *)(*(char **)(slot + 0x24) + 0x10);
    p = (char **)(state + 0x358);
    for (i = 4; i >= 0; i--) {
        if (*p != 0)
            *(int *)(*p + 0x98) = *(int *)(*(char **)(*p + 0x24) + 0x10);
        p++;
    }
    *(int *)(arg0 + 0x98) = *(int *)(*(char **)(arg0 + 0x24) + 0x10);
}

/* Releases the three measured handles of the level state. */
void func_L23_0041D930(void) {
    char *state;

    func_L23_0041A648(D_L23_001B1484);
    state = *(char **)(D_L23_001B1484 + 0x68);
    if (*(char **)(state + 0x620) != 0) {
        func_31A108((int)*(char **)(state + 0x620));
        *(char **)(state + 0x620) = 0;
    }
    if (*(char **)(state + 0x624) != 0) {
        func_L23_0042AC70();
        func_31A108((int)*(char **)(state + 0x624));
        *(char **)(state + 0x624) = 0;
    }
    if (*(char **)(state + 0x614) != 0) {
        func_31A108((int)*(char **)(state + 0x614));
        *(char **)(state + 0x614) = 0;
    }
}

/* Zero while the counter is negative. */
int func_L23_0041C0B0(void) {
    if (D_L23_001B1694 < 0)
        return 1;
    return 0;
}
