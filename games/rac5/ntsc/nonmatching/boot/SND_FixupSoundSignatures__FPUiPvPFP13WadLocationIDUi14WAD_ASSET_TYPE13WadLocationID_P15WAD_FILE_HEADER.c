/* Fix up the sound asset references of one loaded sound WAD block: load the
   banks, resolve cross references, then rewrite every entry through the
   caller's fixup callback. */

extern void snd_BankLoadFromEE_CB(void *wad, void *callback, int *result);
extern int snd_FlushSoundCommands(void);
extern void DelayThread(int usec);
extern void snd_GetLastLoadError(void);
extern void snd_ResolveBankXREFS(void);
extern unsigned int
SND_Fixup__FUiPFP13WadLocationIDUi14WAD_ASSET_TYPE13WadLocationID_P15WAD_FILE_HEADER(
    unsigned int id, void *callback);
extern void snd_LoadBankCallback(void); /* 0x01E91EA8 */

unsigned int
SND_FixupSoundSignatures__FPUiPvPFP13WadLocationIDUi14WAD_ASSET_TYPE13WadLocationID_P15WAD_FILE_HEADER(
    unsigned int *data, void *wad, void *callback) {
    int result;
    unsigned int count;
    unsigned int i;
    unsigned int fixed;

    if (data[0] != 0xAABBCCDDu)
        return 0;
    data++;
    if ((int)data[0] < 0)
        return 0;
    result = 0;
    snd_BankLoadFromEE_CB(wad, (void *)snd_LoadBankCallback, &result);
    while (snd_FlushSoundCommands())
        DelayThread(5000);
    if (!result)
        snd_GetLastLoadError();
    else
        snd_ResolveBankXREFS();
    count = data[0];
    for (i = 0; i < count; i++) {
            fixed = SND_Fixup__FUiPFP13WadLocationIDUi14WAD_ASSET_TYPE13WadLocationID_P15WAD_FILE_HEADER(
                data[i + 1], callback);
            if (fixed)
                *(unsigned int *)(fixed + 0xC) = result;
        data[i + 1] = fixed;
    }
    data[0] |= 0x80000000u;
    return result;
}
