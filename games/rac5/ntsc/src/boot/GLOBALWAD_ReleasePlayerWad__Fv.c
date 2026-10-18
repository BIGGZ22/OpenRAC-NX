typedef struct {
    unsigned char pad00[0x38];
    unsigned int field38;
    unsigned char pad3C[0xC];
    unsigned int field48;
    unsigned char pad4C[0x8];
    unsigned int field54;
    unsigned int field58;
    unsigned int field5C;
    unsigned int field60;
    unsigned int field64;
    unsigned int field68;
} tMcpGlobals;

extern tMcpGlobals g_McpGlobals;
extern int g_PlayerSoundBank;
extern void snd_UnloadBank(int bank);
extern int snd_FlushSoundCommands(void);
extern void DelayThread(unsigned int micros);
extern void MEM_free__FPvPcUi(void *pointer, char *name, unsigned int flags);

void GLOBALWAD_ReleasePlayerWad__Fv(void) {
    if (g_PlayerSoundBank != 0) {
        snd_UnloadBank(g_PlayerSoundBank);
        while (snd_FlushSoundCommands() != 0) {
            DelayThread(0x320);
        }
        g_PlayerSoundBank = 0;
    }
    if (g_McpGlobals.field60 != 0) {
        MEM_free__FPvPcUi((void *)g_McpGlobals.field60, 0, 0);
    }
    g_McpGlobals.field38 = 0;
    g_McpGlobals.field60 = 0;
    g_McpGlobals.field64 = 0;
    g_McpGlobals.field68 = 0;
}
