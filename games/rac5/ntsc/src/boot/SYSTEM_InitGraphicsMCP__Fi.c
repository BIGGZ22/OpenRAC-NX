extern char gMCPRenderTargetA[];       /* 0x01EF69E0 */
extern char gMCPRenderTargetB[];       /* 0x01EF69F8 */
extern int gMCPValue571C;              /* 0x01EF571C */
extern unsigned char gMCPFlag5720;     /* 0x01EF5720 */
extern void SYSTEM_InitGSSettings__FP12RenderTargetT0bUiT2(char *a, char *b, int flag,
                                                           unsigned int value, char *c);
extern void SYSTEM_SetClearColor__FUi(unsigned int color);

void SYSTEM_InitGraphicsMCP__Fi(int unused) {
    SYSTEM_InitGSSettings__FP12RenderTargetT0bUiT2(gMCPRenderTargetA, gMCPRenderTargetB,
                                                   gMCPFlag5720, gMCPValue571C + 2,
                                                   (char *)1);
    SYSTEM_SetClearColor__FUi(0);
}
