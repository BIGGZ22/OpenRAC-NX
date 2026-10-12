/* attempt kept for the next worker - func_L09_00326E90
   closest attempt: two instructions over. gcc substitutes the incoming parameter for
   the loaded control word (daddu $v0,$a0,$0) instead of returning the word already in $v0.
   verify with: tools/check.py func_L09_00326E90 --diff
*/
extern int func_002AD980(void);
extern int D_0018C0B0;

int func_L09_00326E90(int value)
{
    int state;

    if (value == D_0018C0B0)
        return D_0018C0B0;
    state = func_002AD980();
    if (state == 255 || state == 10 || state == 18 || state == 53 || state == 14
        || state == 17 || state == 12)
        return 1;
    return 0;
}
