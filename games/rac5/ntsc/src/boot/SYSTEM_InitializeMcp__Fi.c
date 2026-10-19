/* w12/b21 — SYSTEM_InitializeMcp__Fi
 * Reconstructed from the retail disassembly: load the IOP modules and the
 * sound driver, reset the GS/DMA path and wire the matrix stack, displist
 * database, VRAM, texture manager and the VSync interrupt.
 */
typedef struct MATRIX4 { float m[16]; } MATRIX4;
typedef struct DLIB_DISPLIST_DB DLIB_DISPLIST_DB;

extern char g_iopModule1[];   /* 0x01F1EFA8 */
extern char g_iopModule2[];   /* 0x01F1EFB0 */
extern char g_iopModule3[];   /* 0x01F1EFB8 */
extern char g_iopModule4[];   /* 0x01F1EFC0 */
extern char g_iopModule5[];   /* 0x01F1EFC8 */
extern char g_iopModule6[];   /* 0x01F1EFD0 */
extern char g_soundModule[];  /* 0x01F1EFD8 */
extern MATRIX4 g_matrixStack; /* 0x01F77970 */
extern MATRIX4 g_projection;  /* 0x01EF56C0 */
extern DLIB_DISPLIST_DB g_displistDb; /* 0x01F78B70 */

int sceSifInitIopHeap(void);
int SYSTEM_LoadIOPModule(char *name);
void exit(int code);
int sceSdRemoteInit(void);
int sceSifLoadModule(const char *name, int argc, const char *argv);
void MEM_InitializeMemory(void);
void sceDmaReset(int mode);
void sceGsResetPath(void);
void sceGsSyncPath(int mode, int timeout);
void SetMatrixStack(MATRIX4 *stack, unsigned int a, unsigned int b,
                    unsigned int c, unsigned int d, unsigned int e,
                    unsigned int f);
void MatrixMode(int mode);
void LoadMatrix(MATRIX4 *matrix);
void dlib_displist_db_init(DLIB_DISPLIST_DB *db, int mode);
void VRAM_Initialize(void);
void TEXMGR_Initialize(void);
void VsyncIntHand__Fi(void);
int AddIntcHandler(int cause, void (*handler)(void), int arg);
int EnableIntc(int cause);

void SYSTEM_InitializeMcp__Fi(int mode) {
    sceSifInitIopHeap();
    sceSifInitIopHeap();

    if (!SYSTEM_LoadIOPModule(g_iopModule1))
        exit(0);
    if (!SYSTEM_LoadIOPModule(g_iopModule2))
        exit(0);
    if (!SYSTEM_LoadIOPModule(g_iopModule3))
        exit(0);
    if (!SYSTEM_LoadIOPModule(g_iopModule4))
        exit(0);
    if (!SYSTEM_LoadIOPModule(g_iopModule5))
        exit(0);
    if (!SYSTEM_LoadIOPModule(g_iopModule6))
        exit(0);

    sceSdRemoteInit();
    if (sceSifLoadModule(g_soundModule, 0, 0) < 0)
        exit(0);

    MEM_InitializeMemory();
    sceDmaReset(1);
    sceGsResetPath();
    sceGsSyncPath(0, 0);

    SetMatrixStack(&g_matrixStack, 8, 8, 0x20, 8, 8, 8);
    MatrixMode(5);
    LoadMatrix(&g_projection);

    dlib_displist_db_init(&g_displistDb, mode);
    VRAM_Initialize();
    TEXMGR_Initialize();
    AddIntcHandler(2, VsyncIntHand__Fi, 0);
    EnableIntc(2);
}
