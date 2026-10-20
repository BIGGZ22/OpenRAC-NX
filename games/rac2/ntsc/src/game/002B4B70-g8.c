/* func_002B4B70 returns -1 when D_0018C2EC is zero; otherwise 1 when the stack D_001B1D50 (depth
 * D_001AA100) is empty or D_001A8BB8 is not -2; otherwise it stores its two arguments in
 * D_001AA0F8 and D_001AA0FC, pops the top word of the stack into D_001A8BB8, clears D_001AA104 and
 * returns 0.
 *
 * Built with -G8 (the file name says so), where the three spellings of a global give retail's
 * three kinds of access:
 *   - `sda`: cc1 emits a bare symbol after its `.extern`, and gas makes it a $gp access;
 *   - `nosda` (D_0018C2EC): cc1 splits the address itself, a `lui` into a temporary and the load
 *     off its `%lo`;
 *   - a plain extern of at most 8 bytes (D_001A8BB8): a bare symbol whose `.extern` comes last, so
 *     gas expands it as `lui $r,%hi ; lw $r,%lo($r)` (a store goes through $at).
 * The stack is declared with 2 elements. A sized array is one `la` in cc1's output, which gas
 * expands to an adjacent lui + addiu; with `D_001B1D50[]` cc1 splits the pair itself and the
 * scheduler moves the lui away from its addiu.
 */
extern int D_001AA100 __attribute__((sda));
extern int D_001AA0F8 __attribute__((sda));
extern int D_001AA0FC __attribute__((sda));
extern int D_001AA104 __attribute__((sda));
extern int D_0018C2EC __attribute__((nosda));
extern int D_001A8BB8;
extern int D_001B1D50[2];

int func_002B4B70(int value, int index)
{
    int count = D_001AA100;
    int status;

    if (D_001A8BB8 != -2)
        status = 1;
    else
        status = 0;
    if (count == 0)
        status = 1;
    if (D_0018C2EC == 0)
        status = -1;
    if (status == 0) {
        D_001AA0F8 = value;
        D_001AA0FC = index;
        D_001A8BB8 = D_001B1D50[--D_001AA100];
        D_001AA104 = 0;
    }
    return status;
}
