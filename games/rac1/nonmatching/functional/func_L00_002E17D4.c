/* functional: high. Returns 1 when bit 0 of tbl[0xBC] is set, else 0. Retail reads only its third argument ($6) and returns 0 or 1 in $2 (the addiu $2,1 delay slot and the daddu $2,0,0 fall-through give exactly that); the first two arguments are unused. No host-specific behaviour involved. */
int func_L00_002E17D4(int a0, int a1, unsigned char *tbl) {
    (void)a0;
    (void)a1;
    return (tbl[0xBC] & 1) ? 1 : 0;
}
