#include "include_asm.h"

typedef unsigned char u8;
typedef short s16;

/* The hero state block, and the second block the frame counter lives in. Both are
   resident (below 0x1A7C80), so every level sees them at the same address. */
extern char D_00189E20[];
extern char D_0018C958[];

/* A byte global read as an absolute (its low half has bit 15 set, so it needs the
   %hi/%lo pair), and the two counters read the same way. */
extern unsigned char D_0013965E __attribute__((nosda));
extern int D_001A79F0;
extern int D_001A7390;

/* The counter the level publishes in its literal pool: read absolutely in the
   first block and through $gp in the second, so it needs two names. */
extern int D_L18_001AFBB0;
extern int D_L18_001AFBB0_sda __attribute__((sda));

/* The resident counter, likewise read absolutely once and through $gp once. */
extern int D_001A7390_sda __attribute__((sda));

extern void func_L00_002D24E8(void);
extern void func_L18_002D0C00(void);
extern void ref_L18_002C41F0_00(void);
extern int func_00282978(int a0);
extern void ref_L18_002C41F0_01(int a0, int a1);
extern int func_00288E18(int a0, int a1);
extern void ref_L18_002C41F0_02(void);
extern void func_L00_002AE0C8(void);
extern void func_L00_002B9CA8(void);
extern void func_L00_002A5368(void);
extern void func_L00_002A9600(void);
extern void func_L00_002A7958(void);
extern void func_L00_002A7748(void);
extern void func_L00_003080D0(void *a0);
extern void func_L18_00428100(void *a0);
extern void func_L00_002A9760(void);
extern void func_002A677C(void *a0);

/* One frame of level 18's hero update. Three openers run, then the hero block
   gates a block of work: the state byte has to be 4, the level's own counter has
   to be clear, the hero has to be idle and its state word must not be 0x6F. Inside
   that, a helper may arm the hero (the state word is cleared and a message is
   sent), then the frame counter and a byte global gate a second block that keeps
   a timestamp in step with the resident one. The closing half always runs: the
   sub-updates are gated twice more by the state byte, and the last of them hands
   the hero's record to a helper. */
void func_L18_002C41F0(void)
{
    char *base;
    char *hero;
    char *blk;
    unsigned int now;
    unsigned int then;

    func_L00_002D24E8();
    func_L18_002D0C00();
    ref_L18_002C41F0_00();

    base = D_00189E20;
    if (*(u8 *)(base + 0x22B4) == 4) {
        if (D_L18_001AFBB0 == 0) {
            if (*(s16 *)(base + 0x1E0) == 0) {
                if (*(int *)(base + 0x2294) != 0x6F) {
                    if (func_00282978(*(int *)(*(int *)(base + 0x2290) + 0x68) + 0x3BC) != 0) {
                        *(int *)(base + 0x24CC) = 0;
                        ref_L18_002C41F0_01(0x91, 1);
                    }
                    if (D_001A79F0 == 0x14) {
                        if (D_0013965E == 0) {
                            blk = D_0018C958;
                            if (D_001A7390 - *(unsigned int *)(blk + 0x7900) >= 0xB5) {
                                if (func_00288E18(0xA15, 0x6C) != 0) {
                                    now = D_001A7390_sda;
                                    then = *(unsigned int *)(blk + 0x7900);
                                    if (then < now) {
                                        *(unsigned int *)(blk + 0x7900) = now;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    ref_L18_002C41F0_02();

    base = D_00189E20;
    func_L00_002AE0C8();
    if (*(u8 *)(base + 0x22B4) == 0)
        return;
    func_L00_002B9CA8();
    func_L00_002A5368();
    if (*(u8 *)(base + 0x22B4) == 0)
        return;
    func_L00_002A9600();
    func_L00_002A7958();
    func_L00_002A7748();
    func_L00_003080D0(base + 0x80);
    if (*(u8 *)(base + 0x22B4) == 4 && D_L18_001AFBB0_sda == 0 && *(s16 *)(base + 0x1E0) == 0 &&
        *(int *)(base + 0x2294) != 0x6F) {
        func_L18_00428100(*(int *)(base + 0x2290));
    }
    func_L00_002A9760();
    hero = D_00189E20;
    func_002A677C(*(int *)(hero + 0x2290));
}

/* The group id lists and the entity bank base are read absolutely, the resident group count
   through $gp. */
extern unsigned int D_L18_001FB320[];
extern char *D_L18_001B281C;
extern int D_L18_001B2838 __attribute__((sda));
extern void func_L18_003F7A88(unsigned char *a0);

/* Walks the id list of the object's group and hands every entity it names to
   func_L18_003F7A88. Retail's loop is not rotated (the test sits below the body); the second
   `q != 0` in the condition keeps the test from being copied in front of the loop. */
void func_L18_003F7AB8(unsigned char *arg0)
{
    unsigned short *q;
    unsigned char *e;

    if (arg0[0x21] == 0xFF)
        return;
    q = (unsigned short *)D_L18_001FB320[arg0[0x21]];
    while (arg0[0x21] != 0xFF && arg0[0x21] <= D_L18_001B2838 && q != 0 &&
           ((q != 0 && (short)q[-1] >= 0) || q == (unsigned short *)D_L18_001FB320[arg0[0x21]])) {
        e = (unsigned char *)(D_L18_001B281C + ((*q & 0x7FFF) << 8));
        if (e == 0)
            break;
        func_L18_003F7A88(e);
        q++;
    }
}
