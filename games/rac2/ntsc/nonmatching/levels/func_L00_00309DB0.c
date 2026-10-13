/* 7 instructions in retail, 12 in ours; no jr ra in the retail bytes (addiu sp,+0x20 / daddu / addiu sp,+0x40), so no C of this shape can equal them. Catalogue size or split is suspect. Stub below is a guess only. */
extern int D_L00_001A904C_1 __attribute__((sda));
void func_L00_00309DB0(int argument)
{
    if (D_L00_001A904C_1 != 0) {
        func_0034CE28(D_L00_001A904C_1 + 36808, argument);
    }
}
