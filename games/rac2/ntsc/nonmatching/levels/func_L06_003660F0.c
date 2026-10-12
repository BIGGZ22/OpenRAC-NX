/* DRAFT - best attempt, 1 scheduling difference left (272/280 bytes would be...)
 * check.py: func_L06_003660F0: 0 of 1 places match
 * Left difference: retail loads the flag with `lui $v0,0x1A ; lbu $v0,0x7A47($v0)`
 * back to back and tests $v0; this source gives `lui $v0,%hi ; lbu $v1,%lo($v0)`
 * with the load scheduled after the prologue saves. Everything else is identical.
 * Tried: store order (0x28,0x20,0x24) then (0x20,0x24,0x28) - the second one matches
 * retail; volatile on the flag (no change).
 * Note: 0x001A7A47 is below 0x1A7C80, so it is BOOT data (D_001A7A47), declared without sda
 * because retail reads it with lui/%lo, not through $gp.
 */
extern unsigned char D_001A7A47;
extern void func_0029F978(void);
extern void func_0029F9F0(int a0, char *a1);

void func_L06_003660F0(int a0, int a1, char *a2, float v)
{
    if (D_001A7A47 != 0) {
        if (*(unsigned char *)(a2 + 1) == 0)
            func_0029F978();
        *(float *)(a2 + 0x20) = v;
        *(float *)(a2 + 0x24) = v;
        *(float *)(a2 + 0x28) = v;
    } else if (*(unsigned char *)(a2 + 1) != 0) {
        func_0029F9F0(a0, a2);
    }
}
