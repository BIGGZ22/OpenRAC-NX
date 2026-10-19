typedef struct {
    float x;
    float y;
    float z;
} VEC3;

extern float mcptrans_fall_velX;
extern float mcptrans_fall_velY;
extern float mcptrans_fall_velZ;
extern VEC3 mcptrans_fall_dstVec;
extern VEC3 mcptrans_fall_drag;
extern unsigned char mcptrans_fall_active;
extern void GCSPAR_StreckSetDragMotion__FRC4VEC3(const VEC3 *);
extern void MCPTRANS_FALL_SpeckEmitterUpdate__Fv(void);
extern void MCPTRANS_FALL_StartSound__Fv(void);

void MCPTRANS_FALL_Animate__Ff(float dt) {
    mcptrans_fall_velZ += mcptrans_fall_velY * dt;
    mcptrans_fall_dstVec.z += mcptrans_fall_velX * dt;
    GCSPAR_StreckSetDragMotion__FRC4VEC3(&mcptrans_fall_drag);
    MCPTRANS_FALL_SpeckEmitterUpdate__Fv();
    if (mcptrans_fall_active == 0) {
        mcptrans_fall_active = 1;
        MCPTRANS_FALL_StartSound__Fv();
    }
}
