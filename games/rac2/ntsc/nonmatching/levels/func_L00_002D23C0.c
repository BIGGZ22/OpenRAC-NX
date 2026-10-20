/* 0 of 27 places match - last line: 00_aranos_tutorial: 180 bytes, expected 176
   Candidate is one instruction over.  Seeds a mode record on the resident object: writes the
   caller's index and index+1 into the record bytes 0x40/0x41, clamps 0x41 with the selected
   table entry's own count, clears 0x44, publishes the index at 0x43/0x42, and stores the two
   helper returns at 0x58/0x5C.  Retail keeps the caller's index in the CALLER-saved $a2
   (its live range ends at the `func_310498` call, whose delay slot holds the last use), this
   cc1 parks it in $s0 and so emits a `sd`/`ld` pair.  Retail also folds the index scale into
   the address (`sll $v1,$a2,2` before the base add) where this cc1 materialises the index in
   $a3 first, and it compares the clamped byte with the signed `slt` where this cc1 emits
   `sltu` + `andi 0xFF` (the byte is unsigned).  Measured after that first tour: writing the
   two index stores BEFORE the helper call does move the index into the caller-saved `$t0`
   and drops the `sd`/`ld` pair, but the size stays 180 and the pair is still not retail's
   `$a2`/`$v1`, so the wall is the allocator's register choice, not the live range.  Needs
   the index in $a2 with the scale folded into $v1, and the signed `slt`. */

typedef int s32;
typedef unsigned char u8;

extern char *D_0018C0B0;
extern void func_00310498(void *);
extern int func_002A4DB0(void *, int, int, void *);
extern char D_001B4400[];

void func_L00_002D23C0(int a0, int a1)
{
    char *o = D_0018C0B0;
    unsigned char t;

    *(unsigned char *)(o + 0x41) = a1 + 1;
    *(unsigned char *)(o + 0x40) = a1;
    t = *(unsigned char *)(*(int *)(*(int *)(o + 0x24) + (a0 << 2) + 0x48) + 0x10) - 1;
    if (t < (unsigned char)(a1 + 1))
        *(unsigned char *)(o + 0x41) = t;
    *(int *)(o + 0x44) = 0;
    *(unsigned char *)(o + 0x43) = a0;
    func_00310498(o);
    *(unsigned char *)(o + 0x42) = a0;
    *(int *)(o + 0x58) = func_002A4DB0(o, *(unsigned char *)(o + 0x42),
                                       *(unsigned char *)(o + 0x40), D_001B4400);
    *(int *)(o + 0x5C) = func_002A4DB0(o, *(unsigned char *)(o + 0x43),
                                       *(unsigned char *)(o + 0x41), D_001B4400 + 0x600);
}
