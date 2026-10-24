/* WarpScreenCB__11ApeLevelDefP12TiXmlElement_LEVEL13_000022F0 -- LEVEL13.REL 0x000022F0, 268 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   WarpScreenCB__11ApeLevelDefP12TiXmlElement -- LEVEL13.REL 0x00022F0, 268 bytes.
 *   Body translated from the retail module.
 *   WarpScreenCB__11ApeLevelDefP12TiXmlElement  (LEVEL13.REL 0x00022F0, 268 octets)
 *   Retail: addiu $sp,-0x30 / lui $v0,0x40 / sd $s0,0x0 / sd $s1,0x8
 *   lw $s0,0x5BA8($v0) / daddu $s1,$a0 / sd $ra,0x10
 *   beqz $s0,+0xD0 / swc1 $f20,0x20 / mtc1 $zero,$f20
 *   jal QueryBoolAttribute__C12TiXmlElementPCcPbb
 *   jal QueryFloatAttribute__C12TiXmlElementPCcPff (x4)
 *   jal QueryIntAttribute__C12TiXmlElementPCcPii (x2)
 *   jal QueryRGBAAttribute__C12TiXmlElementPCcP7ApeRGBARC7ApeRGBA
 *   return 1
 *   The warp screen settings of the level definition that was loaded last are
 *   read from the XML element; the zero float default lives across the calls,
 *   so it is kept in the callee-saved register $f20.
 *   Placement: module-relative offset of the disc module.
 *   Proof: the gprfirst route (gnu-ee-20261015-gprfirst:-O2 -G0) compiles this
 *   source to the retail bytes of that placement, byte for byte.  The program-
 *   level gate is still pending, so the body is proved, not promoted.
 *   The layout of the structures is the one this body reads; it is not
 *   confirmed by the data of the disc yet.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015-gprfirst:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct TiXmlElement TiXmlElement;
typedef struct ApeLevelDef ApeLevelDef;

typedef struct ApeRGBA {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
} ApeRGBA;

struct ApeLevelDef {
    char pad_00[0x241];
    unsigned char warpUsed;        /* 0x241 */
    char pad_242[2];
    float warpNear;                /* 0x244 */
    float warpFar;                 /* 0x248 */
    float warpScale;               /* 0x24C */
    float warpTime;                /* 0x250 */
    int warpMode;                  /* 0x254 */
    int warpIndex;                 /* 0x258 */
    ApeRGBA warpColour;            /* 0x25C */
};

extern ApeLevelDef *_11ApeLevelDef$s_lastLevelDef;

/* The XML attribute names of the retail module; they have no exported name.
 * The retail loads each of them with "lui $a1,0x28" and a negative offset,
 * so their link address carries the page and the low half. */
extern char D_0027F778[];
extern char D_0027F7D8[];
extern char D_0027F940[];
extern char D_0027F950[];
extern char D_0027F960[];
extern char D_0027F970[];
extern char D_0027F980[];
extern char D_0027F990[];

extern ApeRGBA _12ApeRGBAConst$WHITE;

extern int QueryBoolAttribute__C12TiXmlElementPCcPbb(const TiXmlElement *element,
                                                       const char *name,
                                                       unsigned char *value,
                                                       unsigned char fallback);
extern int QueryIntAttribute__C12TiXmlElementPCcPii(const TiXmlElement *element,
                                                     const char *name,
                                                     int *value, int fallback);
extern int QueryFloatAttribute__C12TiXmlElementPCcPff(const TiXmlElement *element,
                                                       const char *name,
                                                       float *value, float fallback);
extern int QueryRGBAAttribute__C12TiXmlElementPCcP7ApeRGBARC7ApeRGBA(
    const TiXmlElement *element, const char *name, ApeRGBA *value,
    const ApeRGBA *fallback);

int WarpScreenCB__11ApeLevelDefP12TiXmlElement(ApeLevelDef *self,
                                                TiXmlElement *element)
{
    ApeLevelDef *def = _11ApeLevelDef$s_lastLevelDef;

    if (!def) {
        return 0;
    }

    {
        float fallback = 0.0f;

        QueryBoolAttribute__C12TiXmlElementPCcPbb((const TiXmlElement *)self,
                                                   D_0027F778,
                                                   &def->warpUsed, 0);
        QueryFloatAttribute__C12TiXmlElementPCcPff((const TiXmlElement *)self,
                                                   D_0027F940,
                                                   &def->warpNear, fallback);
        QueryFloatAttribute__C12TiXmlElementPCcPff((const TiXmlElement *)self,
                                                   D_0027F950,
                                                   &def->warpFar, fallback);
        QueryIntAttribute__C12TiXmlElementPCcPii((const TiXmlElement *)self,
                                                 D_0027F960, &def->warpMode, 0);
        QueryIntAttribute__C12TiXmlElementPCcPii((const TiXmlElement *)self,
                                                 D_0027F970, &def->warpIndex, 0);
        QueryFloatAttribute__C12TiXmlElementPCcPff((const TiXmlElement *)self,
                                                   D_0027F980,
                                                   &def->warpScale, fallback);
        QueryFloatAttribute__C12TiXmlElementPCcPff((const TiXmlElement *)self,
                                                   D_0027F990,
                                                   &def->warpTime, fallback);
        QueryRGBAAttribute__C12TiXmlElementPCcP7ApeRGBARC7ApeRGBA(
            (const TiXmlElement *)self, D_0027F7D8, &def->warpColour,
            &_12ApeRGBAConst$WHITE);
    }

    return 1;
}
