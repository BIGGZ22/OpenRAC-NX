extern char D_L00_0021D140[];
extern char D_L00_0021DDB0[];
extern int D_L00_001B2370[];
extern void func_00282A60(void *dst, long value, int size);

/* Clears two 0xC70-byte blocks and one 0x40-byte block. */
void func_L00_003588A8(void)
{
    func_00282A60(D_L00_0021D140, 0, 0xC70);
    func_00282A60(D_L00_0021DDB0, 0, 0xC70);
    func_00282A60(D_L00_001B2370, 0, n);
}
