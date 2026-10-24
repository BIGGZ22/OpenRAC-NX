/* RadialBlurCB__11ApeLevelDefP12TiXmlElement_LEVEL13_00001A88 -- LEVEL13.REL 0x00001A88, 184 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   RadialBlurCB__11ApeLevelDefP12TiXmlElement -- LEVEL13.REL 0x00001A88, 184 bytes.
 *   Body translated from the retail module.
 *   RadialBlurCB__11ApeLevelDefP12TiXmlElement  (LEVEL13.REL 0x00001A88, 184 octets)
 *   Retail: addiu $sp,-0x40 / lui $v0,0x40 / sd $s0,0x10 / sd $s1,0x18
 *   lw $s0,0x5BA8($v0) / daddu $s1,$a0 / sd $ra,0x20
 *   beqz $s0,+0x7C / swc1 $f20,0x30
 *   jal QueryBoolAttribute__C12TiXmlElementPCcPbb
 *   jal QueryIntAttribute__C12TiXmlElementPCcPii
 *   lbu $v1,0x0($sp) / sb $v1,0x22E($s0)
 *   jal QueryFloatAttribute__C12TiXmlElementPCcPff (x2)
 *   sb $v1,0x22C($s0) / return 1
 *   The callback fills the radial blur settings of the level definition that
 *   was loaded last; the zero float default lives across the two calls, so it
 *   is kept in the callee-saved register $f20.
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

struct ApeLevelDef {
    char pad_00[0x22C];
    unsigned char radialBlurSet;   /* 0x22C */
    unsigned char radialBlurUsed;   /* 0x22D */
    unsigned char radialBlurValue;  /* 0x22E */
    float radialBlurRadius;        /* 0x230 */
    float radialBlurAngle;         /* 0x234 */
};

extern ApeLevelDef *_11ApeLevelDef$s_lastLevelDef;

/* The XML attribute names of the retail module; they have no exported name.
 * The retail loads them with "lui $a1,0x28 / addiu $a1,$a1,-0x888", so the
 * link-time address of each name carries the low half of the page and the
 * offset is negative. */
extern char D_0027F778[];
extern char D_0027F850[];
extern char D_0027F858[];
extern char D_0027F868[];

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

int RadialBlurCB__11ApeLevelDefP12TiXmlElement(ApeLevelDef *self,
                                                TiXmlElement *element)
{
    ApeLevelDef *def = _11ApeLevelDef$s_lastLevelDef;
    int value;

    if (!def) {
        return 0;
    }

    {
        float fallback = 0.0f;

        QueryBoolAttribute__C12TiXmlElementPCcPbb((const TiXmlElement *)self,
                                                   D_0027F778,
                                                   &def->radialBlurUsed, 0);
        QueryIntAttribute__C12TiXmlElementPCcPii((const TiXmlElement *)self,
                                                 D_0027F850, &value, 0);
        def->radialBlurValue = (unsigned char)value;
        QueryFloatAttribute__C12TiXmlElementPCcPff((const TiXmlElement *)self,
                                                   D_0027F858,
                                                   &def->radialBlurRadius,
                                                   fallback);
        QueryFloatAttribute__C12TiXmlElementPCcPff((const TiXmlElement *)self,
                                                   D_0027F868,
                                                   &def->radialBlurAngle,
                                                   fallback);
        def->radialBlurSet = 1;
    }

    return 1;
}
