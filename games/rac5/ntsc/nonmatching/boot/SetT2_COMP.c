/* RAC5 (2026-10-17, w11/s3) SetT2_COMP 0x01ED92D0..0x01ED92E0 (16 o) — profil sdk (Sony 2.9).
 * Retail : a1 = a0 ; lui a0,0xB000 ; j SetT2 ; ori a0,a0,0x1020 (appel frere). */
extern void SetT2(volatile unsigned int *reg, unsigned int value);

void SetT2_COMP(unsigned int value) {
    SetT2((volatile unsigned int *)0xB0001020, value);
}
