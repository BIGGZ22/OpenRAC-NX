typedef struct { char pad[0xF8]; float fov; float last_fov; } tCAMERA;

extern tCAMERA *camera_current;

void CAMERA_StoreThisFrameLastUpdatedFov__Fv(void) {
    camera_current->last_fov = camera_current->fov;
}
