extern int SND_Fixup__FUiPFP13WadLocationIDUi14WAD_ASSET_TYPE13WadLocationID_P15WAD_FILE_HEADER(
    unsigned int id, void *callback);
extern void GLOBALWAD_FindAssetsInWadBySignature__FP13WadLocationIDUi14WAD_ASSET_TYPE13WadLocationID(void);

int GLOBALWAD_FindSound__FUi(unsigned int id) {
    return SND_Fixup__FUiPFP13WadLocationIDUi14WAD_ASSET_TYPE13WadLocationID_P15WAD_FILE_HEADER(
        id, (void *)GLOBALWAD_FindAssetsInWadBySignature__FP13WadLocationIDUi14WAD_ASSET_TYPE13WadLocationID);
}
