extern void *LOADER_FindAssetsBySignature__FUi14WAD_ASSET_TYPE(unsigned int signature,
                                                               int type);
extern void *TEXMGR_LoadTexture__FP15WAD_FILE_HEADER(void *asset);

void *LOADER_LoadTexture__FUi14WAD_ASSET_TYPE(unsigned int signature, int type) {
    return TEXMGR_LoadTexture__FP15WAD_FILE_HEADER(
        LOADER_FindAssetsBySignature__FUi14WAD_ASSET_TYPE(signature, type));
}
