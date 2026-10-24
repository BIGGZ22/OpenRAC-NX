/* LEVEL15_0053D0 -- LEVEL15.REL 0x0053D0, 208 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   QueryFloatAttribute__C12TiXmlElementPCcPff -- LEVEL15.REL 0x000053D0, 208 bytes.
 *   Body translated from the retail module (LEVEL15.REL).
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015-gprfirst) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct TiXmlElement TiXmlElement;
typedef int TiXmlBinAttrType;

struct TiXmlElement {
    unsigned char pad00[0x1C];
    int flags;
};

extern const void *Attribute__C12TiXmlElementPCcR16TiXmlBinAttrType(
    TiXmlElement const *self, char const *name, TiXmlBinAttrType *type);
extern int sscanf(char const *text, char const *format, float *value);
extern char D_27AEE8[];

int QueryFloatAttribute__C12TiXmlElementPCcPff(
    TiXmlElement const *self, char const *name, float *out, float def)
{
    TiXmlBinAttrType type;
    const void *attr;

    if (self->flags & 1) {
        attr = Attribute__C12TiXmlElementPCcR16TiXmlBinAttrType(self, name, &type);
        if (attr) {
            switch (type) {
            case 0:
                if (sscanf((char const *)attr, D_27AEE8, out) != 1) {
                    *out = def;
                    return 2;
                }
                return 0;
            case 1:
                *out = (float)(*(int const *)attr);
                return 0;
            case 2:
                *out = *(float const *)attr;
                return 0;
            default:
                *out = def;
                return 2;
            }
        }
    }
    *out = def;
    return 1;
}
