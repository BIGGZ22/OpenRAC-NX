/* LevelUpBolt_RegisterSystem__F10DRAW_Layeri15DRAW_ZWriteMode17DRAW_ColorKeyMode14DRAW_BloomMode14DRAW_BlendMode_LEVEL14_0491D0 -- LEVEL14.REL 0x0491D0, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   32 octets.
 *   Sept arguments : les quatre premiers en $a0-$a3, les trois suivants en
 *   $t0-$t2 (convention EABI) ; les deux derniers sont reprises telles
 *   quelles des arguments recus.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct DRAW_Layer DRAW_Layer;
typedef int DRAW_ZWriteMode;
typedef int DRAW_ColorKeyMode;
typedef int DRAW_BloomMode;
typedef int DRAW_BlendMode;

extern void func_19D468(void *a, int b, int c, int d, int e, int f, void *g);
extern unsigned int LevelUpBolt_data;

void LevelUpBolt_RegisterSystem__F10DRAW_Layeri15DRAW_ZWriteMode17DRAW_ColorKeyMode14DRAW_BloomMode14DRAW_BlendMode(
    DRAW_Layer *layer,
    int param1,
    DRAW_ZWriteMode z_write_mode,
    DRAW_ColorKeyMode color_key_mode,
    DRAW_BloomMode bloom_mode,
    DRAW_BlendMode blend_mode)
{
    func_19D468(layer, param1, z_write_mode, color_key_mode,
                bloom_mode, blend_mode, &LevelUpBolt_data);
}
