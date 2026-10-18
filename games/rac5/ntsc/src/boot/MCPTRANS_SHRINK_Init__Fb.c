extern void *mcptrans_shrink_gde;
extern unsigned char mcptrans_shrink_flag6F4;
extern void *GLOBALWAD_FindAssetsBySignature__FUi14WAD_ASSET_TYPE(unsigned int, int);
extern void LOAD_CHARACTER_SkinGdeFile__FP15WAD_FILE_HEADERPP14GDE_Header_PS2(void *, void *);
extern void ShrinkBeam_RegisterSystems__Fv(void);

void MCPTRANS_SHRINK_Init__Fb(int create) {
    void *asset = GLOBALWAD_FindAssetsBySignature__FUi14WAD_ASSET_TYPE(0x1545A13A, 5);

    if (asset != 0)
        LOAD_CHARACTER_SkinGdeFile__FP15WAD_FILE_HEADERPP14GDE_Header_PS2(asset,
                                                                         &mcptrans_shrink_gde);
    if (create != 0)
        ShrinkBeam_RegisterSystems__Fv();
    mcptrans_shrink_flag6F4 = 0;
}
