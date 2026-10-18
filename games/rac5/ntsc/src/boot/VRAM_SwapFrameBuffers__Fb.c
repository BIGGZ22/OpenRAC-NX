/* Candidate only: the full-program build and compiler profile are not qualified. */
extern unsigned char g_vram_field20;
extern int g_vram_field24;
extern void sceDisplayWaitVblankStart__Fv(void);
extern int sceDisplayGetVcount__Fv(void);

void VRAM_SwapFrameBuffers__Fb(int flag) {
    if (g_vram_field20) {
        sceDisplayWaitVblankStart__Fv();
        if (sceDisplayGetVcount__Fv() == g_vram_field24 + 1) {
            sceDisplayWaitVblankStart__Fv();
        }
        g_vram_field24 = sceDisplayGetVcount__Fv();
    }
}
