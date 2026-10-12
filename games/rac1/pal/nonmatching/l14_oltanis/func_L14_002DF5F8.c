/* NON_MATCHING func_L14_002DF5F8 -- src/overlays/l14_oltanis/vendor_002ACCC0.c
 * Best so far: SIZE ours 200 / retail 192, checked 2026-10-12.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   That is the 4-byte shortfall.
 *   3. Register numbers agree with retail in p4/p5/p7/p8 (rotation $17, data $16).
 *   Diagnosis: a coalescing/allocator tie between the qcopy source temporary and
 *   the rotation variable (gcc 2.95 -O2). None of the source wordings changed the
 *   tie. Stopped at the rule-7 point (more than three variants in a row at the same
 *   bytes). A way out would need a source form that keeps the qcopy source
 *   temporary from being coalesced with the variable. I did not find one with
 *   plain C.
 */
extern void func_L00_002617B0(char *, void *, void *, void *);
extern float D_L14_0015F660[] MACRO_ADDR;
extern short D_L14_00161BCC;
// Initializes a platform and updates its movement transform.
void func_L14_002DF5F8(char *moby) {
 float old[4];
 char *data = *(char **)(moby + 0x78);
 char *rotation;
 qcopy(old, moby + 0x40);
 rotation = moby + 0x40;
 if (*(unsigned char *)(moby + 0x20) == 0) {
  *(float *)(moby + 0x18) += 0.35f;
  moby[0x20] = 1;
  *(unsigned short *)(moby + 0x34) |= 0x100;
  moby[0xBC] = 0;
  *(int *)(data + 0x5C) = 1;
  *(int *)(data + 0x70) = 0;
  qzero(data + 0x60);
  qzero(moby + 0x40);
 }
 func_L14_002DF6B8(moby, *(float *)&D_L14_00161BCC, 8.0f);
 func_L00_002617B0(data + 0x20, D_L14_0015F660, old, rotation);
}
