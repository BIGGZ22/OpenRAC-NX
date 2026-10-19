extern int g_mcptrans_current;
extern void MCPTRANS_UpdateCamera__Ff(float dt);
extern void MCPTRANS_SHRINK_UpdateCamera__Ff(float dt);
extern void MCPTRANS_FALL_UpdateCamera__Ff(float dt);
extern void MCPTRANS_DREAM_UpdateCamera__Ff(float dt);

void MCP_UpdateCameraCB__FPfT014CameraSystemIDPv(float *dt, int id, void *ctx) {
    switch (g_mcptrans_current) {
    case 0:
        MCPTRANS_DREAM_UpdateCamera__Ff(0.050000004f);
        break;
    case 3:
        MCPTRANS_SHRINK_UpdateCamera__Ff(0.050000004f);
        break;
    case 4:
        MCPTRANS_FALL_UpdateCamera__Ff(0.050000004f);
        break;
    default:
        MCPTRANS_UpdateCamera__Ff(0.050000004f);
        break;
    }
}
