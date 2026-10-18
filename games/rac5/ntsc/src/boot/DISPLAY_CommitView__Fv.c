extern char g_projMat[];   /* 0x01F8A880 */
extern char g_viewMat[];   /* 0x01F8A840 */
extern void MatrixMode__Q34scee3Psp3GumQ44scee3Psp2Gu11EMatrixType(int mode);
extern void LoadMatrix__Q34scee3Psp3GumPC14ScePspFMatrix4(const void *matrix);
extern void UpdateMatrix__Q34scee3Psp3Gumv(void);

void DISPLAY_CommitView__Fv(void) {
    MatrixMode__Q34scee3Psp3GumQ44scee3Psp2Gu11EMatrixType(0);
    LoadMatrix__Q34scee3Psp3GumPC14ScePspFMatrix4(g_projMat);
    MatrixMode__Q34scee3Psp3GumQ44scee3Psp2Gu11EMatrixType(1);
    LoadMatrix__Q34scee3Psp3GumPC14ScePspFMatrix4(g_viewMat);
    UpdateMatrix__Q34scee3Psp3Gumv();
}
