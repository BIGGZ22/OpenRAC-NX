extern char g_projMat[]; /* 0x01F7A880 */
extern void rac5_setup_projection_1E89630(void *projection, float a, float b,
                                          float c, float d);

void DISPLAY_SetupProjection__Fffff(float fov, float aspect, float near_plane,
                                    float far_plane) {
    rac5_setup_projection_1E89630(g_projMat, fov, aspect, near_plane, far_plane);
}
