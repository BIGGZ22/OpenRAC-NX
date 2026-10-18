/* RAC5 (2026-10-17, w11/s3) sn_reg_frame 0x01EDC1B0..0x01EDC1C4 (20 o) — profil sdk (Sony 2.9).
 * Retail : lui a0,0x1F0 ; lui a1,0x1FF ; addiu a0,a0,0x6E0 ;
 * j __register_frame_info_table ; addiu a1,a1,-0x51D0 (appel frere, sans trame).
 * Les deux adresses sont ecrites base + decalage signe : le retail emploie
 * addiu (-0x51D0), qu'un litteral non signe rendrait en ori. */
extern void __register_frame_info_table(void *object, void *table);
extern char D_01F00000[];      /* 0x01F00000 */
extern char D_01FF0000[];      /* 0x01FF0000 */

void sn_reg_frame(void) {
    __register_frame_info_table(D_01F00000 + 0x6E0, D_01FF0000 - 0x51D0);
}
