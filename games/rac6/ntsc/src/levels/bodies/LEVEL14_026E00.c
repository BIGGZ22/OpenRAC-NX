/* LEVEL14_026E00 -- LEVEL14.REL 0x026E00, 128 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   L'instance unique du gestionnaire de cartes memoire (D_0027FE94) est
 *   interrogee : le drapeau vaut (champ_3C0 < 3) quand le champ_704 est nul,
 *   et zero sinon. Le drapeau non nul court-circuite les trois appels de
 *   remise a jour.
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

extern MCM *D_0027FE94;

extern void func_026308(int n);
extern void func_07E7A8(void);
extern void func_081A50(void);
extern void func_081CC8(void);
extern void func_08BE08(void);
extern void func_08BE58(void);
extern void func_067728(void);

void func_026E00(void)
{
    unsigned long long v;
    int t;

    func_026308(1);
    func_07E7A8();
    func_081A50();
    func_081CC8();

    v = 0;
    if (D_0027FE94->f704 == 0) {
        t = (D_0027FE94->f3C0 < 3);
        v = t;
    }
    if (v & 0xFF)
        goto skip;

    func_081A50();
    func_08BE08();
    func_08BE58();
skip:
    func_067728();
}
