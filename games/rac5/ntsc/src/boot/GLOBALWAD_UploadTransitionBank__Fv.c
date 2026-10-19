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
extern unsigned int g_TransitionBankPtr;
extern unsigned int g_McpTransitionBank;
extern void *WAD_FindAssetsBySignature__FUi14WAD_ASSET_TYPEP10WAD_HEADER(
    unsigned int signature, int type, void *header);
extern int SND_FixupSoundSignaturesIOP__FPUiPvPFP13WadLocationIDUi14WAD_ASSET_TYPE13WadLocationID_P15WAD_FILE_HEADERb(
    unsigned int *data, void *buffer, void *callback, int flag);
extern void GLOBALWAD_FindAssetsInWadBySignature__FP13WadLocationIDUi14WAD_ASSET_TYPE13WadLocationID(void);

int GLOBALWAD_UploadTransitionBank__Fv(void) {
    unsigned char *asset = WAD_FindAssetsBySignature__FUi14WAD_ASSET_TYPEP10WAD_HEADER(
        0xAABBCC9F, 4, (void *)g_McpGlobals.field54);
    if (asset != 0) {
        g_TransitionBankPtr =
            SND_FixupSoundSignaturesIOP__FPUiPvPFP13WadLocationIDUi14WAD_ASSET_TYPE13WadLocationID_P15WAD_FILE_HEADERb(
                (unsigned int *)(asset + 0xC0), (void *)g_McpTransitionBank,
                (void *)GLOBALWAD_FindAssetsInWadBySignature__FP13WadLocationIDUi14WAD_ASSET_TYPE13WadLocationID,
                1);
    }
    return g_TransitionBankPtr != 0;
}
