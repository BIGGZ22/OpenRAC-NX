/* 0 of 27 places match - last line: 00_aranos_tutorial: 144 bytes, expected 152
   Fills a 0x40 stack block through the engine, copies its last 16 bytes into the record at
   +0x10, hands the record's +0xC0 and then the record itself to two more setters, queues one
   five-argument request and stamps four resident words.  Everything matches except the
   16-byte copy: retail materialises BOTH addresses in registers (`addiu $a0,$s1,0x10`,
   `addiu $v1,$sp,0x30`) and then uses `lq`/`sq` at offset 0, this cc1 folds both offsets into
   the `lq 0x30($sp)` / `sq 0x10($s1)` pair, two instructions fewer. */

#include "include_asm.h"

typedef int V4_F6a70b950;

extern void func_3114E0(int, int, void *);
extern void func_2EEBD8(void *, void *);
extern void func_312D18(void *);
extern void func_2AA200(int, int, int, int, int);

extern char ref_L00_002D5000_00[];
extern char ref_L00_002D5000_01[];

void func_L00_002D5000(int a0, char *a1)
{
    char block[0x40];

    func_3114E0(a0, 4, block);
    *(V4_F6a70b950 *)(a1 + 0x10) = *(V4_F6a70b950 *)(block + 0x30);
    func_2EEBD8(a1 + 0xC0, block);
    func_312D18(a1);
    func_2AA200((int)ref_L00_002D5000_00, (int)ref_L00_002D5000_01,
                *(int *)(a1 + 0x24), 0, a0);
    *(int *)(a1 + 0x5C) = (int)ref_L00_002D5000_01;
    *(int *)(a1 + 0x58) = (int)ref_L00_002D5000_01;
    *(int *)(a1 + 0x40) = 0;
    *(int *)(a1 + 0x44) = 0;
}
