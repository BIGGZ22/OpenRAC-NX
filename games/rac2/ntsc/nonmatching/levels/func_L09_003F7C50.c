/* attempt kept for the next worker - func_L09_003F7C50
   closest attempt: one instruction out of place - retail loads the descriptor before the
   $s0 comparisons, gcc sinks that lw into the bne delay slot.
   verify with: tools/check.py func_L09_003F7C50 --diff
*/
extern int func_L09_0033E690_1(char *object, int index, float value);
extern int func_L09_00329178_1(char *object, int index, int value, int flag);

int func_L09_003F7C50(char *object, int index, int value)
{
    char *descriptor = *(char **)(object + 0x68);

    if (index == -1) {
        *(unsigned short *)(descriptor + 0x1C8) = index;
        *(int *)(descriptor + 0x2DC) = index;
        return -1;
    }
    if (func_L09_00329178_1(object, index, value,
                            func_L09_0033E690_1(object + 0x10, index, 0.0f)) != 0)
        return *(short *)(descriptor + 0x1C8);
    *(unsigned short *)(descriptor + 0x1C8) = 0;
    return -1;
}
