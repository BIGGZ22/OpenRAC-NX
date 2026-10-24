/* Serialize__14AglTextureBaseRQ33IFF7Texture6Header -- FE.REL 0x06AFB8, 64 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Le lecteur IFF fournit trois champs (mot 32 bits, deux demi-mots) copies
 *   dans la texture, puis le nom (chaine a l'offset 11 du bloc) est passe a
 *   SetName avec le drapeau "false".
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct AglTextureBase {
    unsigned char unknown_000[0x30];
    unsigned int field_30;
    unsigned short field_34;
    unsigned short field_36;
} AglTextureBase;

typedef struct IFF {
    unsigned int word0;
    unsigned short word4;
    unsigned short word6;
    unsigned char unknown_008[3];
    char name[1];
} IFF;

extern void SetName__14AglTextureBasePCcb(AglTextureBase *self,
                                          const char *name,
                                          int flag);

void Serialize__14AglTextureBaseRQ33IFF7Texture6Header(AglTextureBase *self,
                                                       IFF *reader,
                                                       void *header)
{
    (void)header;

    self->field_30 = reader->word0;
    self->field_34 = reader->word4;
    self->field_36 = reader->word6;

    SetName__14AglTextureBasePCcb(self, reader->name, 0);
}
