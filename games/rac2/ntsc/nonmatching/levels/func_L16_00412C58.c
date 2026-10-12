/* Walks a rotating candidate index over the measured byte table until a free slot
   is found, arms it, and clears the whole window when none is. */
void func_L16_00412C58(u8 *arg, s32 start, s32 end)
{
    u8 *s3;
    u8 *tbl;
    s32 n;
    s32 i;
    s32 k;

    s3 = *(u8 **)(arg + 0x68);
    if (end < start)
        return;
    n = end - start + 1;
    k = func_313EA0(n);
    tbl = s3 + 0x310;
    for (i = 0; i < n; i++) {
        if (tbl[start + k] == 0) {
            func_L16_00412BA8(arg, start + k);
            return;
        }
        k = (k + 1) % n;
    }
    for (i = 0; i < n; i++)
        *((u8 *)s3 + start + 0x310 + i) = 0;
    func_L16_00412BA8(arg, start + k);
}
