extern int SpiralSphere_RegisterSystem__F10DRAW_Layeri15DRAW_ZWriteMode17DRAW_ColorKeyMode14DRAW_BloomMode14DRAW_BlendMode(
    int layer, void *group, int zwrite, int colorkey, int bloom, int blend);
extern void *gShrinkBeamGroup;   /* 0x01F69BBC */
extern int gShrinkBeamHandle;    /* 0x01EF0D74 */

int ShrinkBeam_RegisterSystems__Fv(void) {
    int h = SpiralSphere_RegisterSystem__F10DRAW_Layeri15DRAW_ZWriteMode17DRAW_ColorKeyMode14DRAW_BloomMode14DRAW_BlendMode(
        0xB, gShrinkBeamGroup, 0, 0, 1, 1);
    gShrinkBeamHandle = h;
    return h;
}
