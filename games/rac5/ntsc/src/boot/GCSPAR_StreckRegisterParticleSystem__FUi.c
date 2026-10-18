extern void par_unk1E73E80(void);
extern int PAR_CreateSystemInternal__FP10PAR_SystemUiPFPviPC9PAR_GroupPC29PAR_UpdateAndTesselateContext_iiiiPFP9PAR_Group_v(
    void *, unsigned int, void *, int, int, int, int);
extern unsigned int PAR_RegisterSystem__FPC10PAR_System(void *);

unsigned int GCSPAR_StreckRegisterParticleSystem__FUi(unsigned int arg) {
    char system[0x10];
    unsigned int flags;

    flags = (arg & 0xFFFFFFC3u) | 4u;
    flags &= 0xFFFFFFFCu;
    PAR_CreateSystemInternal__FP10PAR_SystemUiPFPviPC9PAR_GroupPC29PAR_UpdateAndTesselateContext_iiiiPFP9PAR_Group_v(
        system, flags | 1u, par_unk1E73E80, 0x20, 0, -1, 0);
    return PAR_RegisterSystem__FPC10PAR_System(system);
}
