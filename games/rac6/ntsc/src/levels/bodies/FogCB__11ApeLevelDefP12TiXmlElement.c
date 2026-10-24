/* FogCB__11ApeLevelDefP12TiXmlElement -- FE.REL 0x000016A0, 288 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   FogCB__11ApeLevelDefP12TiXmlElement -- FE.REL 0x0016A0, 288 bytes.
 *   Body translated from the retail module.
 *   FogCB__11ApeLevelDefP12TiXmlElement  (FE.REL 0x0016A0, 288 octets)
 *   Retail: addiu $sp,-0x40 / lui $v0,0x25 / sd $s0,0x10 / sd $s1,0x18
 *   lw $s0,0x6790($v0) / daddu $s1,$a0 / sd $ra,0x20
 *   beqz $s0,+0xE4 / swc1 $f20,0x30
 *   sw $zero,0x0($sp) / jal QueryIntAttribute__C12TiXmlElementPCcPii
 *   jal QueryBoolAttribute__C12TiXmlElementPCcPbb
 *   jal QueryFloatAttribute__C12TiXmlElementPCcPff (x2)
 *   jal QueryRGBAAttribute__C12TiXmlElementPCcP7ApeRGBARC7ApeRGBA
 *   jal QueryIntAttribute__C12TiXmlElementPCcPii
 *   sb $v1,0x1E0($s0) / return 1
 *   The fog settings of the level definition that was loaded last are read from
 *   the XML element; the index read from the element selects the slot, the zero
 *   float default lives across the calls in the callee-saved register $f20.
 *   Placement: module-relative offset of the disc module; the other
 *   placements of the same body are listed in config/levels/bodies.tsv.
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
    char pad_00[0x1E0];
    unsigned char fogSet;           /* 0x1E0 */
    unsigned char fogFlags[3];      /* 0x1E1 */
    float fogValues[4];             /* 0x1E4 */
    ApeRGBA fogColours[2];          /* 0x1F4 */
    int fogInts[4];                 /* 0x1FC */
};

extern ApeLevelDef *_11ApeLevelDef$s_lastLevelDef;

/* The XML attribute names of the retail module; they have no exported name. */
extern char D_00111BF8[];
extern char D_00111C00[];
extern char D_00111C08[];
extern char D_00111C20[];
extern char D_00111C28[];
extern char D_00111C30[];

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

int FogCB__11ApeLevelDefP12TiXmlElement(ApeLevelDef *self, TiXmlElement *element)
{
    ApeLevelDef *def = _11ApeLevelDef$s_lastLevelDef;
    int slot;

    if (!def) {
        return 0;
    }

    {
        float fallback = 0.0f;

        slot = 0;
        QueryIntAttribute__C12TiXmlElementPCcPii((const TiXmlElement *)self,
                                                 D_00111C20, &slot, 0);
        QueryBoolAttribute__C12TiXmlElementPCcPbb((const TiXmlElement *)self,
                                                   D_00111C28,
                                                   &def->fogFlags[slot], 0);
        QueryFloatAttribute__C12TiXmlElementPCcPff((const TiXmlElement *)self,
                                                   D_00111BF8,
                                                   &def->fogValues[slot],
                                                   fallback);
        QueryFloatAttribute__C12TiXmlElementPCcPff((const TiXmlElement *)self,
                                                   D_00111C00,
                                                   &def->fogValues[slot + 2],
                                                   fallback);
        QueryRGBAAttribute__C12TiXmlElementPCcP7ApeRGBARC7ApeRGBA(
            (const TiXmlElement *)self, D_00111C08, &def->fogColours[slot],
            &_12ApeRGBAConst$WHITE);
        QueryIntAttribute__C12TiXmlElementPCcPii((const TiXmlElement *)self,
                                                 D_00111C30,
                                                 &def->fogInts[slot], 0);
        def->fogSet = 1;
    }

    return 1;
}
