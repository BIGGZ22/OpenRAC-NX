typedef struct {
    unsigned char pad[0xF8];
    float fov;
} tCAMERA;

extern tCAMERA *camera_current;
extern float DISPLAY_CalcFovX__Ff(float fov);
extern void CAMERA_SetFovX__Ff(float fov);

void CAMERA_SetFov__Ff(float fov) {
    camera_current->fov = fov;
    CAMERA_SetFovX__Ff(DISPLAY_CalcFovX__Ff(fov));
}
