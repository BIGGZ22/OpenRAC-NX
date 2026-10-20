#include "common.h"
#include "structs.h"

extern int func_00131760(int);

typedef struct { char b[0xC]; } Cfg12;

extern Cfg12 r1_D_00153D40 NOT_SDA;

/* Qwen draft (branch qwen/autonomy, bb52125e), checked by tools/check.py */
void func_00132DF0(int, int, int, long, int);

   /* current buffer */
extern short r1_D_0015EDC4;

/* rac1-decomp src/core/0012CC90.c: func_0012D500 */
/* Convert the sceCdCLOCK fields (second..year) in place with the helper
   above. Exact under 2.9-ee, which saves $16 before $31 as retail does;
   2.95.3 saved them the other way round (4/104), the prologue-order
   residual that pointed at the SDK compiler in the first place. */
void func_00131780(unsigned char *p) {
    p[7] = func_00131760(p[7]);
    p[6] = func_00131760(p[6]);
    p[5] = func_00131760(p[5]);
    p[3] = func_00131760(p[3]);
    p[2] = func_00131760(p[2]);
    p[1] = func_00131760(p[1]);
}
extern void func_001317E8(unsigned char *p);
extern int func_00131730(int);

void func_001317E8(unsigned char *clock) {
    clock[7] = func_00131730(clock[7]);
    clock[6] = func_00131730(clock[6]);
    clock[5] = func_00131730(clock[5]);
    clock[3] = func_00131730(clock[3]);
    clock[2] = func_00131730(clock[2]);
    clock[1] = func_00131730(clock[1]);
}


/* rac1-decomp src/core/0012CC90.c: func_0012D5D0 */
/*
 * Tick the sceCdCLOCK at s (binary fields: [5] day, [6] month, [7] year)
 * forward by one day, with a working copy of the 12-byte month-length
 * table r1_D_00153D40 (February stretched to 29 when year & 3 == 0), the
 * month rolling 12 -> 1 and the year 99 -> 0.
 *
 * The leap test reads the year into an int. Retail loads the year twice
 * (for the leap test, and again at the year roll-over); 2.9-ee's GCSE
 * would reuse a first byte-typed load for the second (SIZE 184/180, and
 * the whole reason this function once looked like 2.95.3 code). An int
 * load is a zero-extending load, a different expression to GCSE, so
 * the later byte load stays. (This C is exact under 2.95.3 as well.)
 */
void func_00131850(unsigned char *s) {
    Cfg12 days;
    unsigned char m;
    int y = s[7];

    days = r1_D_00153D40;
    s[5] = s[5] + 1;
    if ((y & 3) == 0) {
        days.b[1] = 0x1D;
    }
    if (s[5] > days.b[s[6] - 1]) {
        s[5] = 1;
        s[6] = s[6] + 1;
        m = s[6];
        if (m == 13) {
            if (s[7] == 0x63) {
                s[7] = 0;
            } else {
                s[7] = s[7] + 1;
            }
            s[6] = 1;
        }
    }
}

/* rac1-decomp src/core/0012CC90.c: func_0012D688 */
/*
 * Tick the sceCdCLOCK at s back by one day: the same month-length table,
 * the day counting down to 0 rolls the month back (1 -> 12) and the year
 * with it (0 -> 99), then the day is reloaded from the table.
 *
 * Levers: the leap test on an int copy of the year, as in func_00131850
 * (2.9-ee's GCSE otherwise reuses the first year load: 61/164);
 * `s[5] = s[5] - 1;` before the leap block (a block boundary gives
 * retail's reload of s[5]); the month as `if (--s[6] == 0)` (stores the
 * raw decrement and tests the masked copy in another register); the
 * year as an if/else with one store per arm, which cross-jumping merges
 * into retail's single `sb`. Exact under both compilers.
 */
void func_00131908(unsigned char *s) {
    Cfg12 days;
    int y = s[7];

    days = r1_D_00153D40;
    s[5] = s[5] - 1;
    if ((y & 3) == 0) {
        days.b[1] = 0x1D;
    }
    if (s[5] == 0) {
        if (--s[6] == 0) {
            if (s[7] == 0) {
                s[7] = 0x63;
            } else {
                s[7] = s[7] - 1;
            }
            s[6] = 0xC;
        }
        s[5] = days.b[s[6] - 1];
    }
}

/* rac1-decomp src/core/0012CC90.c: func_0012D730 */
/*
 * Advance the hour ([3]); at 24 it wraps to 0 and the day ticks forward.
 * Retail's call is a bare tail `j func_00131850` on the wrap path only,
 * with a plain `jr $ra` on the other: a conditional sibling call, which
 * 2.9-ee emits from this plain C. (Under 2.95.3, which has no sibling
 * calls, it was 8 bytes over.)
 */
void func_001319B0(unsigned char *s) {
    s[3] = s[3] + 1;
    if (s[3] == 0x18) {
        s[3] = 0;
        func_00131850(s);
    }
}

/* rac1-decomp src/core/0012CC90.c: func_0012D760 */
/* The backward analog: the hour counts down, and at 0 it becomes 23 and
   the day ticks back, again through a conditional tail call. */
void func_001319E0(unsigned char *s) {
    unsigned char v = s[3];
    if (v != 0) {
        s[3] = v - 1;
        return;
    }
    s[3] = 0x17;
    func_00131908(s);
}

extern void func_001319B0(unsigned char *);
extern void func_001319E0(unsigned char *);

void func_00131A08(unsigned char *clock, int offset) {
    int minutes;
    func_00131780(clock);
    minutes = clock[2] + offset;
    if (minutes >= 0) {
        while (minutes > 60) {
            func_001319B0(clock);
            minutes -= 60;
        }
    } else {
        do {
            minutes += 60;
            func_001319E0(clock);
        } while (minutes < 0);
    }
    clock[2] = minutes;
    func_001317E8(clock);
}

extern int func_00131670(void);
extern int func_001316C8(void);
extern void func_00131A08(unsigned char *, int);

void func_00131A98(unsigned char *clock) {
    int offset;
    int zone = func_00131670();
    offset = func_001316C8();
    offset *= 60;
    offset -= 540;
    func_00131A08(clock, zone + offset);
}

ASM_FUNC("asm/core", func_00131AE8);

/* The exit thunk of symbol_addrs: eight bytes that jump to exit with $4
   forced to 0 in the delay slot, so the status the caller passes is discarded
   and the program always exits with 0. The call is a tail call, and the
   compiler puts the argument in the jump's delay slot. */
extern void func_0011FA20(int);

void func_00131CA8(void) {
    func_0011FA20(0);
}

INCLUDE_ASM("asm/core", func_00131CB0);

INCLUDE_ASM("asm/core", func_00131D98);

INCLUDE_ASM("asm/core", func_00132028);

INCLUDE_ASM("asm/core", func_001326D8);

INCLUDE_ASM("asm/core", func_00132210);


INCLUDE_ASM("asm/core", func_00132318);

INCLUDE_ASM("asm/core", func_00132498);

INCLUDE_ASM("asm/core", func_001325E0);

INCLUDE_ASM("asm/core", func_00132818);

void func_00132858(int value) {
    int payload[4];
    payload[0] = value;
    func_00132DF0(6, 4, (int)payload, 0, 0);
}

void func_00132888(int first, int second) {
    int payload[4];
    payload[0] = first;
    payload[1] = second;
    func_00132DF0(9, 8, (int)payload, 0, 0);
}

typedef struct { char bytes[24]; } CommandParameters24;

void func_001328C0(int value, const CommandParameters24 *parameters) {
    struct {
        int value;
        CommandParameters24 parameters;
    } payload;
    payload.value = value;
    if (parameters != 0)
        payload.parameters = *parameters;
    else
        *(int *)&payload.parameters = -1;
    func_00132DF0(96, 28, (int)&payload, 0, 0);
}

void func_00132938(int value) {
    int payload[4];
    payload[0] = value;
    func_00132DF0(11, 4, (int)payload, 0, 0);
}

INCLUDE_ASM("asm/core", func_00132968);

void func_001329B0(int first, int second, int third) {
    int payload[4];
    payload[0] = first;
    payload[1] = second;
    payload[2] = third;
    func_00132DF0(78, 12, (int)payload, 0, 0);
}

INCLUDE_ASM("asm/core", func_001329F0);

INCLUDE_ASM("asm/core", func_00132A58);

INCLUDE_ASM("asm/core", func_00132AA0);

void func_00132AF8(int value) {
    int payload[4];
    payload[0] = value;
    func_00132DF0(22, 4, (int)payload, 0, 0);
}

void func_00132B28(int param_1) {
  int auStack_20[4];

  auStack_20[0] = param_1;
  func_00132DF0(0x17, 4, (int)auStack_20, 0, 0);
  return;
}

void func_00132B58(int value, long fourth, int fifth) {
    int payload[4];
    payload[0] = value;
    func_00132DF0(25, 4, (int)payload, fourth, fifth);
}

INCLUDE_ASM("asm/core", func_00132B88);

INCLUDE_ASM("asm/core", func_00132C08);

INCLUDE_ASM("asm/core", func_00132C48);

INCLUDE_ASM("asm/core", func_00132DF0);


INCLUDE_ASM("asm/core", func_00133108);

/* rac1-decomp src/core/989snd.c: func_0012EC30 */
/* snd_UnkFunction_0012eaf0 */
void func_00133220(void) {
    *(int *)&r1_D_0015EDC4 = 1;
}

INCLUDE_ASM("asm/core", func_00133300);

INCLUDE_ASM("asm/core", func_00133340);

INCLUDE_ASM("asm/core", func_001333C0);

void func_00133400(int value) {
    int payload[4];
    payload[0] = value;
    func_00132DF0(46, 4, (int)payload, 0, 0);
}

void func_00133430(int value, long fourth, int fifth) {
    int payload[4];
    payload[0] = value;
    func_00132DF0(50, 4, (int)payload, fourth, fifth);
}

void func_00133460(int value, long fourth, int fifth) {
    int payload[4];
    payload[0] = value;
    func_00132DF0(79, 4, (int)payload, fourth, fifth);
}
