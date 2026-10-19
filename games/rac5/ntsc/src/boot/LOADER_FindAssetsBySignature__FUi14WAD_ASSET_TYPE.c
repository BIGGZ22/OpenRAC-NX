extern void *g_pLevelWadData;
extern void *WAD_FindAssetsBySignature__FUi14WAD_ASSET_TYPEP10WAD_HEADER(
    unsigned int signature, int type, void *header);
extern void *GLOBALWAD_FindAssetsBySignature__FUi14WAD_ASSET_TYPE(
    unsigned int signature, int type);

void *LOADER_FindAssetsBySignature__FUi14WAD_ASSET_TYPE(unsigned int signature,
                                                        int type) {
    void *asset = WAD_FindAssetsBySignature__FUi14WAD_ASSET_TYPEP10WAD_HEADER(
        signature, type, g_pLevelWadData);
    if (asset != 0) {
        return asset;
    }
    return GLOBALWAD_FindAssetsBySignature__FUi14WAD_ASSET_TYPE(signature, type);
}
