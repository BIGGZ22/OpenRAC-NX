/* func_002B49E0 starts a request for slot `index`. Its result is the first of these that applies:
 * -1 when D_0018C2EC is zero; -3 when index is not negative and D_001A8B88[index] is non-zero; -2
 * when mode is 1 and the stack D_001B1D50 already holds 8 words (D_001AA100); 0 when D_001A8BB8
 * holds -2; -1 otherwise. It clears *flag first when flag is not null. Only a result of 0 goes
 * on: it records a2, a3, flag and index in D_001AA0F8, D_001AA0FC, D_001AA104 and D_001A8BB8,
 * sets D_001B170C to 2 or 3 (and calls func_002858B0 with the same value when index is 4) for
 * index 3 or 4 under the D_001A79F0 / D_001C4F40 conditions below, and, when mode is 1, pushes
 * D_001A8BB0 on the stack.
 *
 * Built with -G8 (the file name says so); see func_002B4B70 (002B4B70-g8.c) for the three kinds
 * of access:
 *   - `sda` is a $gp access in every use (D_001AA100 and the four stores);
 *   - `nosda` is `lui` into a temporary and a load off its `%lo`;
 *   - a plain extern of at most 8 bytes is a bare symbol: `lui $r,%hi ; lw $r,%lo($r)` in
 *     ordinary flow and a one-instruction $gp access in a branch delay slot, which is why retail
 *     reads D_001A79F0 both ways.
 * The two arrays are declared with 2 elements so that their addresses are one `la` (lui + addiu)
 * rather than a high part held in a register of its own.
 */
extern int D_001AA100 __attribute__((sda));
extern int D_001AA0F8 __attribute__((sda));
extern int D_001AA0FC __attribute__((sda));
extern int D_001AA104 __attribute__((sda));
extern int D_001B170C;
extern int D_001A8BB8;
extern int D_001A79F0;
extern int D_001A8BB0;
extern int D_0018C2EC __attribute__((nosda));
extern int D_001C4F40 __attribute__((nosda));
extern int D_001A8B88[2];
extern int D_001B1D50[2];
extern void func_002858B0(int);

int func_002B49E0(int index, int mode, int a2, int a3, unsigned char *flag)
{
    int result;
    int entry;

    if (flag != 0)
        *flag = 0;
    result = -1;
    if (D_001A8BB8 == -2)
        result = 0;
    if (mode == 1 && !(D_001AA100 < 8))
        result = -2;
    entry = index >= 0 ? D_001A8B88[index] : 0;
    if (entry != 0)
        result = -3;
    if (D_0018C2EC == 0)
        result = -1;
    if (result != 0)
        return result;
    D_001AA0F8 = a2;
    D_001AA0FC = a3;
    D_001AA104 = (int)flag;
    D_001A8BB8 = index;
    if ((D_001A79F0 == 8 && D_001C4F40 == 1) || (D_001A79F0 == 0x13 && D_001C4F40 == 0)) {
        if ((unsigned)(index - 3) < 2) {
            D_001B170C = 2;
            if (index == 4)
                func_002858B0(2);
        }
    }
    if ((unsigned)(D_001A79F0 - 22) < 2) {
        if ((unsigned)(index - 3) < 2) {
            D_001B170C = 3;
            if (index == 4)
                func_002858B0(3);
        }
    }
    if (mode == 1) {
        D_001B1D50[D_001AA100++] = D_001A8BB0;
    }
    return result;
}
