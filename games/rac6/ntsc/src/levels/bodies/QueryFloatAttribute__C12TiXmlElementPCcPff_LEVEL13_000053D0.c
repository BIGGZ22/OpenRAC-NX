/* QueryFloatAttribute__C12TiXmlElementPCcPff_LEVEL13_000053D0 -- LEVEL13.REL 0x000053D0, 208 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   QueryFloatAttribute__C12TiXmlElementPCcPff -- LEVEL13.REL 0x000053D0, 208 bytes.
 *   Body translated from the retail module.
 *   QueryFloatAttribute__C12TiXmlElementPCcPff  (LEVEL13.REL 0x000053D0, 208 octets)
 *   Retail: addiu $sp,-0x40 / sd $s0,0x10 / swc1 $f20,0x30 / daddu $s0,$a2
 *   sd $s1,0x18 / sd $ra,0x20 / lw $v0,0x1C($a0) / andi $v0,$v0,1
 *   beqz $v0 / mov.s $f20,$f12 / jal Attribute__C12TiXmlElementPCcR16TiXmlBinAttrType
 *   lw $v1,0x0($sp) / beq $v1,$s1 / slti $v0,$v1,2 / beqz $v0
 *   beql $v1,$v0 / lwc1 $f0,0x0($a0) / jal sscanf / bnel $v0,$s1
 *   lwc1 $f0,0x0($a0) / cvt.s.w $f0,$f0 / return 0
 *   The attribute is looked up with its binary type; a string is parsed with
 *   "%f", an integer is converted, a float is copied.  The default float of
 *   the caller lives across the calls, so it is kept in the callee-saved
 *   register $f20.
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
typedef int TiXmlBinAttrType;

struct TiXmlElement {
    char pad_00[0x1C];
    unsigned int flags;             /* 0x1C */
};

extern void *Attribute__C12TiXmlElementPCcR16TiXmlBinAttrType(
    const TiXmlElement *element, const char *name, TiXmlBinAttrType *type);
extern int sscanf(const char *text, const char *format, ...);

/* The scan format of the retail module; it has no exported name. */
extern char D_00280368[];

int QueryFloatAttribute__C12TiXmlElementPCcPff(const TiXmlElement *element,
                                                const char *name,
                                                float *value, float fallback)
{
    TiXmlBinAttrType type;
    void *attr;
    float def = fallback;

    if (element->flags & 1) {
        attr = Attribute__C12TiXmlElementPCcR16TiXmlBinAttrType(element, name,
                                                                 &type);
        if (attr) {
            switch (type) {
            case 0:
                if (sscanf((const char *)attr, D_00280368, value) != 1) {
                    *value = def;
                    return 2;
                }
                return 0;
            case 1:
                *value = (float)*(int *)attr;
                return 0;
            case 2:
                *value = *(float *)attr;
                return 0;
            default:
                *value = def;
                return 2;
            }
        }
    }

    *value = def;
    return 1;
}
