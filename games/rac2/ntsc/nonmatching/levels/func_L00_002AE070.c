/* 0 of 27 places match - last line: 30_insomniac_museum: instruction at +0x4c differs
  21 of 22 instructions. The last address is `addu $a2,$a2,$v1` in retail, `addu $v1,$v1,$a2` here:
  the `addu` operand order of the tail pointer. Every spelling tried (flat, [idx], int* pointer,
  index-first) gave the same order. */

float func_L00_002AE070(int a0)
{
    char *base = D_00189E20_2;
    int i = 31;
    int x;

    if (a0 >= 32)
        i = 31;
    else
        i = a0;
    if (*(int *)(base + 0x23C0) < i)
        i = *(int *)(base + 0x23C0);
    x = *(int *)(base + 0x23BC) - i;
    return *(float *)(base + 0x233C + ((x + 31) % 32) * 4);
}
