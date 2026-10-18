/* RAC5 (2026-10-17, w11/s3) SetT2_COUNT 0x01ED92B0..0x01ED92C0 (16 o) — profil sdk (Sony 2.9).
 * Retail : a1 = a0 ; lui a0,0xB000 ; j SetT2 ; ori a0,a0,0x1000 (appel frere). */
extern void SetT2(volatile unsigned int *reg, unsigned int value);

void SetT2_COUNT(unsigned int value) {
    SetT2((volatile unsigned int *)0xB0001000, value);
}
