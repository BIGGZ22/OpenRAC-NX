/* CONTROLLER_GetButtonsPressedWithAnalog__FUi -- FE.REL 0x00EE40, 64 bytes.
 *
 * Returns the pressed buttons of a controller context or-ed with the analog-button mask, and 0 when the context is not active.
 *
 * Evidence:
 *   Le mot global (adresse module 0xF4E04) contient le masque des touches
 *   analogiques ajoute aux boutons presses.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

extern unsigned int g_ControllerAnalogButtons;

extern unsigned int CONTROLLER_CheckContextActive__FUi(unsigned int index);
extern unsigned int CONTROLLER_GetButtonsPressed__FUi(unsigned int index);

unsigned int CONTROLLER_GetButtonsPressedWithAnalog__FUi(unsigned int index)
{
    if (!CONTROLLER_CheckContextActive__FUi(index))
        return 0;

    return CONTROLLER_GetButtonsPressed__FUi(index)
           | g_ControllerAnalogButtons;
}
