/* func_001157E8 -- SCUS_976.23 0x001157E8, 44 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   SCUS_976.23 : func_001157E8 (44 octets)
 *   Range les quatre mots (type, signe, exposant, mantisse) dans une
 *   structure et la passe a func_114C78.
 *
 * Placement: virtual address in the boot executable; the other
 * placements of the same body are listed in config/boot/bodies.tsv.
 * Proof: the pinned chain (sdk991111:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

extern void func_114C78(void *parts);

struct Parts
{
    int type;
    int sign;
    int exponent;
    int mantissa;
};

void func_001157E8(int type, int sign, int exponent, int mantissa)
{
    struct Parts parts;

    parts.type = type;
    parts.sign = sign;
    parts.exponent = exponent;
    parts.mantissa = mantissa;
    func_114C78(&parts);
}
