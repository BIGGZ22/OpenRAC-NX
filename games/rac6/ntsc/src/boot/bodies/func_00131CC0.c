/* func_00131CC0 -- SCUS_976.23 0x00131CC0, 424 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   SCUS_976.23 : func_00131CC0 (424 octets)
 *   Cadre 0x40, slots de 8 octets (sd/ld) : bande `gnu-ee-20261015`.
 *   Reprise de func_00131B18 (meme forme, appels legerement differents).
 *   L'ABI du boot passe les 5e et 6e arguments en $t0 et $t1.
 *   Le gros chemin tombe (fall-through) : c'est lui qui donne le `bnez` du retail.
 *
 * Placement: virtual address in the boot executable; the other
 * placements of the same body are listed in config/boot/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

extern void func_12F028(int a0, int a1, int a2);
extern void func_132118(int a0, int a1, int a2, int a3);
extern void func_12F850(int a0, int a1, int a2, int a3);
extern void func_132200(int a0, int a1, int a2, int a3);
extern void func_12E480(int a0, int a1, int a2);
extern void func_12FA60(int a0, int a1, int a2, int a3);
extern void func_1308E8(int a0, int a1);
extern void func_132388(int a0);
extern void func_131468(int a0, int a1);
extern void func_1324C0(int a0);
extern void func_132538(int a0);
extern void func_132350(int a0);

void func_00131CC0(int n, int p, int q, int r, int t)
{
    int m;

    if (n >= 0x21) {
        m = n >> 2;
        func_12F028(n, p, t + (r - m) * 4);

        if (n >= 0x201) {
            func_132118(m, p, r, t);
            func_12F850(m, p + m * 4, r, t);
            func_132118(m, p + m * 8, r, t);
            func_132118(m, p + m * 12, r, t);
        } else if (m >= 0x21) {
            func_12FA60(n, p, r, t);
        } else {
            func_132200(n, p, r, t);
        }

        func_12E480(n, q, p);
    } else {
        if (n >= 9) {
            if (n == 0x20) {
                func_1308E8(p, t - 0x20 + r * 4);
                func_132388(p);
            } else {
                func_131468(p, t);
                func_1324C0(p);
            }
        } else {
            if (n == 8)
                func_132538(p);
            else if (n == 4)
                func_132350(p);
        }
    }
}
