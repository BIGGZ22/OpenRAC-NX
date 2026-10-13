/* attempt kept for the next worker - func_L09_003EA2D0
   closest attempt: the descriptor now lives in $s3 across the calls; something in the
   call registers or the epilogue still differs around +0x34.
   verify with: tools/check.py func_L09_003EA2D0 --diff
*/
extern unsigned int D_L09_001FE6C0[];
extern int func_L06_00382850_1(char *object, unsigned int entry, float value);
extern int func_002B6228_1(char *object, unsigned int entry, int value, int flag);

int func_L09_003EA2D0(char *object, int index, int value)
{
    char *descriptor = *(char **)(object + 0x68);
    unsigned int entry = D_L09_001FE6C0[index];

    if (entry == 0) {
        *(unsigned short *)(descriptor + 0x118) = 0;
        return -1;
    }
    func_002B6228_1(object, entry, value,
                    func_L06_00382850_1(object + 0x10, entry, 0.0f));
    return *(short *)(descriptor + 0x118);
}
