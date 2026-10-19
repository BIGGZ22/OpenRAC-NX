extern char g_viewMat[]; /* 0x01F7A840 */
extern void rac5_set_view_matrix_1E896D8(void *destination, const void *source);

void DISPLAY_SetViewMatrixFromCameraMatrix__FPC7MATRIX4(const void *camera_matrix) {
    rac5_set_view_matrix_1E896D8(g_viewMat, camera_matrix);
}
