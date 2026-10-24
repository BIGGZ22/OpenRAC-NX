/* func_001157B0 -- SCUS_976.23 0x001157B0, 56 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   SCUS_976.23 : func_001157B0 (56 octets)
 *   Decompose le flottant en quatre mots (type, signe, exposant,
 *   mantisse) grace a func_114D88, inverse le signe stocke, puis
 *   passe la structure a func_114C78.
 *
 * Placement: virtual address in the boot executable; the other
 * placements of the same body are listed in config/boot/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

extern void func_114D88(void *source, void *parts);
extern void func_114C78(void *parts);

struct Parts
{
    int type;
    int sign;
    int exponent;
    int mantissa;
};

void func_001157B0(float value)
{
    float bits = value;
    struct Parts parts;

    func_114D88(&bits, &parts);
    parts.sign = !parts.sign;
    func_114C78(&parts);
}
