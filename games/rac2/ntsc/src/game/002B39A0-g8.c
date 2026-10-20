/* func_002B39A0 picks one of the candidates D_001B17BC[index] lists. It returns 0 when index is
 * outside 0..*(int *)D_001B17C0 - 1, when D_001B1688 is below 2 or when the list is empty. Before
 * that last test it writes the short 10 at +0x0A of the 0x30-byte record 0x30 * (index + 1) of the
 * table D_001B17C0. A list of one gives that candidate's target and stores its value through out;
 * a longer one is rated with func_002B38C8 (against 9999999.0f to begin with) and gives the target
 * and value of the lowest rating.
 *
 * Built with -G8 (the file name says so). D_001B17BC, D_001B17C0 and D_001B1688 are plain externs
 * of at most 8 bytes: bare symbols for cc1 (`lui $r,%hi ; lw $r,%lo($r)` in ordinary flow), which
 * gas turns into one-instruction $gp accesses wherever the load sits in a branch delay slot, as
 * retail's do. See func_002B4B70 (002B4B70-g8.c).
 */
typedef struct {
    char pad_00[0x0A];
    short mark;
    char pad_0C[0x24];
} Record30_2B39A0;

typedef struct {
    char *target;
    unsigned char count;
    unsigned char value;
} Pick8_2B39A0;

extern Record30_2B39A0 *D_001B17C0;
extern Pick8_2B39A0 **D_001B17BC;
extern int D_001B1688;
extern float func_002B38C8(char *, char *);

char *func_002B39A0(char *self, int index, int *out)
{
    int i;
    char *result;
    float best;

    if (index < 0 || !(index < *(int *)D_001B17C0) || D_001B1688 < 2)
        return 0;
    D_001B17C0[index + 1].mark = 10;
    if (D_001B17BC[index]->count == 0)
        return 0;
    if (D_001B17BC[index]->count == 1) {
        *out = D_001B17BC[index]->value;
        return D_001B17BC[index]->target;
    }
    best = 9999999.0f;
    result = 0;
    for (i = 0; i < D_001B17BC[index]->count; i++) {
        if (D_001B17BC[index][i].target != 0) {
            float d = func_002B38C8(self, D_001B17BC[index][i].target);
            if (d < best) {
                best = d;
                result = D_001B17BC[index][i].target;
                *out = D_001B17BC[index][i].value;
            }
        }
    }
    return result;
}
