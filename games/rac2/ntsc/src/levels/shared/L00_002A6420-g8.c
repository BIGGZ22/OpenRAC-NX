#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;

/* The resident state block, and the hero position block the tail works from: both are
   incomplete arrays, so the address stays materialised absolutely. */
extern char D_00189E20_2[];
extern char D_00189EA0[];

/* The float the frame publishes, and its source in the level's literal pool, read through
   $gp. The published one is materialised absolutely in the retail bytes. */
extern float D_L00_001A7CF8 __attribute__((sda));
extern float D_L00_0025CFEC __attribute__((nosda));

extern void func_2D24E8(void);
extern void func_2B36E0(void);
extern void func_2ACA08(void);
extern void func_2ACF88(void *a0, void *a1, void *a2);
extern void func_2B8930(void);
extern void func_2CB3A8(void);
extern void func_2AE0C8(void);
extern void func_2B9CA8(void);
extern void func_2A5368(void);
extern void func_L00_002A9600(void);
extern void func_L00_002A7958(void);
extern void func_L00_002A7748(void);
extern void func_2A5840(void);
extern void func_2A5CB0(void);
extern void func_2A6008(void);
extern void func_35F3A8(void *a0, void *a1, void *a2);
extern void func_3080D0(void *a0);
extern void func_L00_002A9760(void);
extern void func_319684(void *a0);

/* One frame of the level's frame update: three openers, a block that runs while the mode
   byte is 1, then that byte gates the sub-updates twice more and gates the arms block, which
   republishes the ambient float and hands an armed record to a helper before clearing it.
   The closing tail always runs, from the hero position block. */
void func_L00_002A65D8(void)
{
    char *base;
    char *hero;

    func_2D24E8();
    func_2B36E0();
    func_2ACA08();
    base = D_00189E20_2;

    if (*(u8 *)(base + 0x22B4) == 1) {
        func_2ACF88(base + 0x80, base + 0x90, base + 0x550);
        func_2B8930();
    }

    func_2CB3A8();
    func_2AE0C8();
    if (*(u8 *)(base + 0x22B4) == 0)
        return;

    func_2B9CA8();
    func_2A5368();
    if (*(u8 *)(base + 0x22B4) == 0)
        return;

    func_L00_002A9600();
    func_L00_002A7958();
    func_L00_002A7748();

    if (*(u8 *)(base + 0x22B4) == 1) {
        int record;

        D_L00_0025CFEC = D_L00_001A7CF8;
        func_2A5840();
        func_2A5CB0();
        func_2A6008();
        record = *(int *)(base + 0x24E4);
        if (record != 0) {
            func_35F3A8((char *)record + 0x10, 0, *(void **)(base + 0x2290));
            *(u16 *)(base + 0x2520) = 0x39;
            *(u16 *)(base + 0x2522) = *(u16 *)(base + 0x24E4);
        }
        *(int *)(base + 0x24E4) = 0;
    }

    hero = D_00189EA0;
    func_3080D0(hero);
    func_L00_002A9760();
    func_319684(*(void **)(hero + 0x2210));
}
