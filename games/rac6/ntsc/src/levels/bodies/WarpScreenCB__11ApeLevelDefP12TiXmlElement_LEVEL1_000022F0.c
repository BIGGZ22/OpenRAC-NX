/* WarpScreenCB__11ApeLevelDefP12TiXmlElement_LEVEL1_000022F0 -- LEVEL1.REL 0x000022F0, 268 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   - charge le pointeur global D_4140E8 (l'objet de niveau) ; s'il est nul,
 *   retour 0 ;
 *   - interroge l'element XML passe en premier parametre (les requetes
 *   TiXmlElement recoivent $a0) et ecrit les resultats dans l'objet de
 *   niveau : amplitudeX/amplitudeY/frequencyX/frequencyY (flottants),
 *   gridDimX/gridDimY (entiers), un booleen et une couleur ;
 *   - retour 1.
 *   La constante 0.0f vit dans $f20 (callee-saved : elle doit survivre aux
 *   huit appels).
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
typedef struct ApeLevelDef ApeLevelDef;

struct ApeRGBA {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};

struct ApeLevelData {
    unsigned char pad00[0x241];
    unsigned char flag241;
    unsigned char pad242[2];
    float f244;
    float f248;
    float f24C;
    float f250;
    int f254;
    int f258;
    struct ApeRGBA f25C;
};

extern struct ApeLevelData *D_4140E8;
extern struct ApeRGBA D_3E7158;

extern char D_28D578[];
extern char D_28D740[];
extern char D_28D750[];
extern char D_28D760[];
extern char D_28D770[];
extern char D_28D780[];
extern char D_28D790[];
extern char D_28D5D8[];

extern int QueryBoolAttribute__C12TiXmlElementPCcPbb(
    TiXmlElement const *self, char const *name, unsigned char *out, unsigned char def);
extern int QueryFloatAttribute__C12TiXmlElementPCcPff(
    TiXmlElement const *self, char const *name, float *out, float def);
extern int QueryIntAttribute__C12TiXmlElementPCcPii(
    TiXmlElement const *self, char const *name, int *out, int def);
extern int QueryRGBAAttribute__C12TiXmlElementPCcP7ApeRGBARC7ApeRGBA(
    TiXmlElement const *self, char const *name, struct ApeRGBA *out, struct ApeRGBA *def);

int WarpScreenCB__11ApeLevelDefP12TiXmlElement(
    TiXmlElement *element, ApeLevelDef *self)
{
    struct ApeLevelData *lvl = D_4140E8;

    (void)self;
    if (!lvl)
        return 0;

    QueryBoolAttribute__C12TiXmlElementPCcPbb(
        element, D_28D578, &lvl->flag241, 0);
    QueryFloatAttribute__C12TiXmlElementPCcPff(
        element, D_28D740, &lvl->f244, 0.0f);
    QueryFloatAttribute__C12TiXmlElementPCcPff(
        element, D_28D750, &lvl->f248, 0.0f);
    QueryIntAttribute__C12TiXmlElementPCcPii(
        element, D_28D760, &lvl->f254, 0);
    QueryIntAttribute__C12TiXmlElementPCcPii(
        element, D_28D770, &lvl->f258, 0);
    QueryFloatAttribute__C12TiXmlElementPCcPff(
        element, D_28D780, &lvl->f24C, 0.0f);
    QueryFloatAttribute__C12TiXmlElementPCcPff(
        element, D_28D790, &lvl->f250, 0.0f);
    QueryRGBAAttribute__C12TiXmlElementPCcP7ApeRGBARC7ApeRGBA(
        element, D_28D5D8, &lvl->f25C, &D_3E7158);

    return 1;
}
