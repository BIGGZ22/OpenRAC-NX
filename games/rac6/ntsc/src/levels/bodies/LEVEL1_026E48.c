/* LEVEL1_026E48 -- LEVEL1.REL 0x026E48, 128 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   128 octets.
 *   L'instance unique du gestionnaire de cartes memoire
 *   (_14MemCardManager$s_singletonInstance, 0x256714) est interrogee : le
 *   drapeau vaut (f3C0 < 3) quand f704 est nul, et zero sinon.  Le drapeau
 *   non nul court-circuite les trois appels de remise a jour.
 *   Le drapeau est un `unsigned long long` : c'est ce type qui empeche le
 *   compilateur de plier `v & 0xFF` en `v != 0`.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct MCM MCM;

struct MCM {
    unsigned char pad000[0x3C0];
    int f3C0;
    unsigned char pad3C4[0x704 - 0x3C0 - 4];
    int f704;
};

extern MCM *D_00256714;

extern void UPDATE_BeginFrame__Fb(int n);
extern void FADEOVERLAY_Render__Fv(void);
extern void HUD_ResetHudRenderStates__Fv(void);
extern void HUD_SetGuWidgetRenderState__Fv(void);
extern void HUD_RenderTopMost__Fv(void);
extern void HUD_RenderUberTopNoMatterWhat__Fv(void);
extern void SYSTEM_EndFrame__Fv(void);

void func_026E48(void)
{
    unsigned long long v;
    int t;

    UPDATE_BeginFrame__Fb(1);
    FADEOVERLAY_Render__Fv();
    HUD_ResetHudRenderStates__Fv();
    HUD_SetGuWidgetRenderState__Fv();

    v = 0;
    if (D_00256714->f704 == 0) {
        t = (D_00256714->f3C0 < 3);
        v = t;
    }
    if (v & 0xFF)
        goto skip;

    HUD_ResetHudRenderStates__Fv();
    HUD_RenderTopMost__Fv();
    HUD_RenderUberTopNoMatterWhat__Fv();
skip:
    SYSTEM_EndFrame__Fv();
}
