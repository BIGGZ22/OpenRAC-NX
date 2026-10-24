/* QueryFloatAttribute__C12TiXmlElementPCcPff -- LEVEL1.REL 0x000053D0, 208 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   - lit un drapeau entier a 0x1C du TiXmlElement ; s'il est nul,
 *   *out = def et retour 1 ;
 *   - appelle Attribute(name, &type) ; le type est un TiXmlBinAttrType ecrit
 *   dans une variable locale de la pile (0($sp)) ; si l'attribut n'existe
 *   pas, *out = def et retour 1 ;
 *   - type 0 : sscanf(texte, "%f", out) ; retour 0, ou *out = def et retour 2
 *   si l'analyse echoue ;
 *   - type 1 : *out = (float)*(int*)attribut ;
 *   - type 2 : *out = *(float*)attribut ;
 *   - tout autre type : *out = def et retour 2.
 *   La valeur par defaut (un flottant) vit dans $f20 : elle doit survivre aux
 *   deux appels ; les deux conversions de l'attribut sont faites dans $f0.
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
extern char D_28E168[];

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
                if (sscanf((char const *)attr, D_28E168, out) != 1) {
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
