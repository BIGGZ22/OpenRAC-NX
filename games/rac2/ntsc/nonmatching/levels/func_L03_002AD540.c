/* func_L03_002AD540 (548 bytes, 8 places) -- NEAR MISS, not integrated.

   The level's own start-up, read from the retail bytes:
   - the two resident flags at 0x189E20+0x2160 and +0x2162 are cleared; the
     flag at +0x22B4 then decides the path (the small call when it is set, the
     level's load and spawn list when it is clear);
   - the level's load, its spawn list, its own record setup and its kind setup
     run, and the resident word at 0x1A8F00 (2 or 6) picks the kind's start-up;
   - the kind's own kind list then runs, the record at +0x22B5 decides which
     of the two resident helpers runs, the level's own table of records is read
     and the level's spawner is handed the result.

   Measured: 560 bytes against 548, 0 of 8 places match.  What is left:
   - retail keeps the page of the resident block in a callee-saved register
     ($s1 = 0x19000) and re-materialises the block's address from it
     (`addiu $a0,$s1,-0x61E0`) for the three int fields at +0x2290/+0x2294/
     +0x229C, while this compiler folds the offsets into the loads off the byte
     pointer ($s0) and never keeps the page in a register: 560 bytes against
     548, the frame is 0x10 against 0x20.
   - the branch that picks between the two resident helpers is `bnez` here and
     `beqz` in retail, with the two blocks the other way round; the source
     spelling that produces retail's `beqz` (the "else" branch taken) was not
     found.
   - the int fields are reached through a second pointer (`ip`) so that the
     offsets fold into the loads; going through the byte pointer directly gives
     retail's `lw $v1,0x229C($a0)` shape but then the address is materialised
     as a `lui`/`lw` pair against the symbol instead of from the page
     register.

   What was measured on the way: the branch polarity of the first test is
   fixed by the source (`if (flag != 0) { the small call } else { the level }`
   gives retail's `beqz` over the level's body), the second call of the pair
   is func_L00_002D3068 and the first is func_L00_002D2E68, the level's own
   table is at 0x189EA0 (the resident block + 0x80) and the resident word is
   at 0x1A8F00, and the three int fields are the block's +0x2290, +0x2294
   and +0x229C.
*/
extern int D_0018F00;

extern unsigned char D_L03_00189E20[];

extern void func_L00_002A65D8(void);
extern void func_L00_002D24E8(void);
extern void func_L00_002B36E0(void);
extern void func_L03_002B2798(void);
extern void func_L00_002ACF88(void *, void *, void *);
extern void func_L00_002ADB40(void);
extern void func_L03_002CCB70(void);
extern void func_L00_002D2E68(void);
extern void func_L00_002D3068(void);
extern void func_L00_002AE0C8(void);
extern void func_L00_002B9CA8(void);
extern void func_L00_002AC1A0(void);
extern void func_L00_002AB1D8(void);
extern void func_L00_002AB420(void);
extern void func_L00_002ABBA0(void);
extern void func_L00_002AB948(void);
extern void func_L00_002A5518(void);
extern void func_L00_002A5840(void);
extern void func_L00_002B7718(void);
extern void func_L00_002A5368(void);
extern void func_L00_002A9460(void);
extern void func_L00_002AA178(void);
extern void func_L00_002B2120(int);
extern void func_L00_002B2318(void);
extern void func_L00_002B2200(void);
extern void func_L00_002AFBC0(void);
extern int func_L00_002D6418(void);
extern void func_L00_002A7958(void);
extern void func_L00_002A7748(void);
extern void func_L00_002A67F8(void);
extern void func_L00_002D5EB8(void);
extern void func_L00_002A6420(void);
extern void func_L00_002D5158(void);
extern int func_L00_002A7000(void);
extern void func_L00_003080D0(void *);
extern void func_002A677C(int);
extern void func_L01_002A9298(void);
extern void func_L00_002A9760(void);

void func_L03_002AD540(void)
{
    unsigned char *s0 = D_L03_00189E20;
    int *ip = (int *)D_L03_00189E20;

    *(short *)(s0 + 0x2162) = 0;
    *(short *)(s0 + 0x2160) = 0;
    if (*(unsigned char *)(s0 + 0x22B4) != 0) {
        func_L00_002A65D8();
    } else {
        func_L00_002D24E8();
        func_L00_002B36E0();
        func_L03_002B2798();
        func_L00_002ACF88(s0 + 0x80, s0 + 0x90, s0 + 0x550);
        func_L00_002ADB40();
        func_L03_002CCB70();
        if (D_0018F00 == 2) {
            return;
        } else if (D_0018F00 == 6) {
            if (*(unsigned char *)(s0 + 0x22B5) != 0)
                func_L00_002D2E68();
            else
                func_L00_002D3068();
            return;
        } else {
            func_L00_002AE0C8();
            if (*(unsigned char *)(s0 + 0x22B4) != 0)
                return;
            func_L00_002B9CA8();
            func_L00_002AC1A0();
            func_L00_002AB1D8();
            func_L00_002AB420();
            func_L00_002ABBA0();
            func_L00_002AB948();
            func_L00_002A5518();
            func_L00_002A5840();
            func_L00_002B7718();
            func_L00_002A5368();
            func_L00_002A9460();
            func_L00_002AA178();
            func_L00_002B2120(0);
            func_L00_002B2318();
            func_L00_002B2200();
            func_L00_002AFBC0();
            if (*(unsigned char *)(s0 + 0x22B5) == 0) {
                if (func_L00_002D6418() == 0)
                    func_L00_002D3068();
                else
                    func_L00_002D2E68();
            } else {
                func_L00_002D2E68();
            }
            func_L00_002A7958();
            func_L00_002A7748();
            func_L00_002A67F8();
            func_L00_002D5EB8();
            func_L00_002A6420();
            func_L00_002D5158();
            if (ip[0x8A7] == 0x16
                || ip[0x8A7] == 0x1F
                || ip[0x8A5] == 0x31) {
                if (func_L00_002A7000() != 0)
                    func_L00_003080D0(D_L03_00189E20 + 0x80);
            } else {
                func_L00_003080D0(D_L03_00189E20 + 0x80);
            }
            func_002A677C(ip[0x8A4]);
            func_L01_002A9298();
            func_L00_002A9760();
        }
    }
}
