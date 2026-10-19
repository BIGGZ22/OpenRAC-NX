typedef struct {
    char pad[0x2DC];
    void *singlePlayer;
    int state2E0;
} tHeroSkin;

extern tHeroSkin g_McpGlobals;
extern void *PS2HEAP_Malloc__FUi(unsigned int);

int HEROSKIN_InitSinglePlayer__Fv(void) {
    if (g_McpGlobals.singlePlayer != 0)
        return 1;
    g_McpGlobals.singlePlayer = PS2HEAP_Malloc__FUi(0x89000);
    return g_McpGlobals.singlePlayer != 0;
}
