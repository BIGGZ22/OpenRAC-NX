/* Candidate only: the full-program build and compiler profile are not qualified. */
extern char g_vram_state[];
extern int g_vram_field18;
extern int g_vram_field1C;
extern void VRAM_InitGraphics__Fv(void);
extern void *memset(void *dst, int value, unsigned int size);

void VRAM_Initialize__Fv(void) {
    memset(g_vram_state, 0, 0x4C);
    g_vram_field18 = 0;
    g_vram_field1C = 0x2D8000;
    VRAM_InitGraphics__Fv();
}
