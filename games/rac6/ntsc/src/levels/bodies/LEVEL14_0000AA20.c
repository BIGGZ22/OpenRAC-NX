/* LEVEL14_0000AA20 -- LEVEL14.REL 0x0000AA20, 360 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Preuve : route astra-divpad:-O2 -G0, comparaison octet pour octet au
 *   retail au placement vise (*** EXACT ***, 360/360 octets).  La route
 *   gprfirst seule s'arrete a 0xAAB4 (les deux nop avant la racine carree).
 *   Variante de CAMDRIVER_PolarToCart : meme normalisation d'angle, meme
 *   extraction du sinus par sqrt(1-cos^2), mais le resultat ne garde que
 *   x et y, et le signe de s2 est pris en compte dans le produit.
 *   ang  = a1 - floorf(a1 / (2 pi) + 0.5f) * (2 pi)
 *   c1 = cosf(ang), s1 = +/- sqrtf(1 - c1*c1)
 *   c2 = cosf(a3),  s2 = +/- sqrtf(1 - c2*c2)
 *   t  = a4*s2 + a2*c2
 *   out->x = t*c1 ; out->y = t*s1
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (astra-divpad:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct VEC3 { float x, y, z; } VEC3;

extern float floorf(float x);
extern float cosf(float x);
extern float sqrtf(float x);

void func_00AA20(VEC3 *out, float ang1, float ang2, float rad1, float rad2)
{
    float ang = ang1 - floorf(ang1 * 0.15915494f + 0.5f) * 6.2831855f;
    float c1 = cosf(ang);
    float s1 = sqrtf(1.0f - c1 * c1);
    float c2, s2, t;

    if (ang < 0.0f)
        s1 = -s1;
    c2 = cosf(ang2);
    s2 = sqrtf(1.0f - c2 * c2);
    if (ang2 < 0.0f)
        s2 = -s2;

    t = rad2 * s2 + rad1 * c2;
    out->x = t * c1;
    out->y = t * s1;
}
