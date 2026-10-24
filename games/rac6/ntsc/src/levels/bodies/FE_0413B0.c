/* FE_0413B0 -- FE.REL 0x0413B0, 128 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Lecture d'un entier non signe dans une chaine, avec bornage de la valeur
 *   entre "min" et "max" (borne haute ignoree si "max" vaut 0).
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

extern int sscanf(const char *str, const char *format, ...);
extern const char g_scan_format_u[];

unsigned long long func_0413B0(const char *str,
                               unsigned int *out,
                               unsigned int min,
                               unsigned int max)
{
    if (sscanf(str, g_scan_format_u, out) != 1)
        return 0;

    if (*out < min)
        *out = min;

    if (max != 0) {
        if (max < *out)
            *out = max;
    }

    return 1;
}
