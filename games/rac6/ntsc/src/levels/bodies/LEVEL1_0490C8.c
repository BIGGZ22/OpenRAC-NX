/* LEVEL1_0490C8 -- LEVEL1.REL 0x0490C8, 256 bytes.
 *
 * Folds two angles back into ]-PI, PI]; the second one only when the flag of the object is set.
 *
 * Evidence:
 *   Normalisation de deux angles dans ]-PI, PI] :
 *   - f4  = f4  - angles->a   puis replie dans ]-PI, PI]
 *   - f8  = f8  + angles->b   puis replie dans ]-PI, PI]
 *   Le second angle n'est traite que si le drapeau 0x20 est non nul.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct Angles Angles;
typedef struct Obj Obj;

struct Angles {
    float a;
    float b;
};

struct Obj {
    Angles *angles;
    float f4;
    float f8;
    unsigned char unknown_00c[0x14];
    int flag;
};

void func_0490C8(Obj *self)
{
    float d = self->f4 - self->angles->a;

    if (d > 3.14159274f) {
        d -= 6.28318548f;
    } else if (d < -3.14159274f) {
        d += 6.28318548f;
    }
    self->f4 = d;
    if (self->flag == 0) {
        return;
    }
    {
        float d2 = self->f8 + self->angles->b;

        if (d2 > 3.14159274f) {
            d2 -= 6.28318548f;
        } else if (d2 < -3.14159274f) {
            d2 += 6.28318548f;
        }
        self->f8 = d2;
    }
}
