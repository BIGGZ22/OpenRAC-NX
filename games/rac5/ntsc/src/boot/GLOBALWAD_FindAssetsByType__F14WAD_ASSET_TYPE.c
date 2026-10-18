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
extern void *WAD_FindAssetsByType__F14WAD_ASSET_TYPEP10WAD_HEADER(int type,
                                                                  void *header);

void *GLOBALWAD_FindAssetsByType__F14WAD_ASSET_TYPE(int type) {
    void *asset = 0;
    if (g_McpGlobals.field38 != 0) {
        asset = WAD_FindAssetsByType__F14WAD_ASSET_TYPEP10WAD_HEADER(
            type, (void *)g_McpGlobals.field60);
    }
    if (asset == 0) {
        asset = WAD_FindAssetsByType__F14WAD_ASSET_TYPEP10WAD_HEADER(
            type, (void *)g_McpGlobals.field48);
    }
    return asset;
}
