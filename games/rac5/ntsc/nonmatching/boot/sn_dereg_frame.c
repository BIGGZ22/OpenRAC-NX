/* RAC5 (2026-10-17, w11/s3) sn_dereg_frame 0x01EDC1C8..0x01EDC1D4 (12 o) — profil sdk (Sony 2.9).
 * Retail : lui a0,0x1F0 ; j __deregister_frame_info ; addiu a0,a0,0x6E0.
 * Adresse ecrite base + decalage signe (addiu cote retail). */
extern void __deregister_frame_info(void *object);
extern char D_01F00000[];      /* 0x01F00000 */

void sn_dereg_frame(void) {
    __deregister_frame_info(D_01F00000 + 0x6E0);
}
