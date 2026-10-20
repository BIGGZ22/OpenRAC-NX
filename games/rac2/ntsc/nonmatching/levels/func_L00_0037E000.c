/* func_L00_0037E000 (192 bytes, 27 places) - best attempt, NOT MATCHED.
 *
 * Docker-free ruler (scratch/wslcmp.py): "00_aranos_tutorial: 176 bytes, expected 192".
 * The record walk (32 x 0x10 at 0x231340), the key test on the second word, the state tests
 * 0xFE/0xFD on the object's byte 0x20, the 0x1000 bit of its halfword 0x34 and the
 * func_00282948(record + 8) tail are all understood, and the `||` chain with a single
 * `entry->field04 = 0` is the right sense.  Residue:
 *   (a) this cc1 runs the loop DOWNWARDS (`addiu $s1,$zero,0x1F` / `addiu $s1,$s1,-0x1`,
 *       `bne`) where retail counts UP (`daddu $s1,$zero,$zero` / `addiu $s1,$s1,0x1`,
 *       `bnel`), which by itself accounts for the missing instructions;
 *   (b) retail keeps the record pointer in $a0 for the store (`sw $zero,0x4($a0)`, with
 *       `daddu $a0,$s0,$zero` before the first beql), mine stores off $s0;
 *   (c) the 0x1000 test is `beql` here with the store annulled, retail uses `beqz`.
 *
 * To retake: a loop form this cc1 keeps ascending - the descending countdown is the whole
 * 16-byte gap.
 */

typedef unsigned char u8;
typedef unsigned int u32;
typedef signed int s32;

typedef struct {
    u32 field00, field04;
    s32 field08;
    u8 gap0c[4];
} OrbitalPairTableView16;

extern u8 D_L00_00231340[];
extern int func_00282948(void *p);

void func_L00_0037E000(u32 a0)
{
    OrbitalPairTableView16 *entry = (OrbitalPairTableView16 *)D_L00_00231340;
    int i;

    for (i = 0; i < 32; i++, entry++) {
        if (entry->field04 == a0) {
            u8 *p = (u8 *)entry->field00;

            if ((*(unsigned short *)(p + 0x34) & 0x1000) == 0 || p == 0
                || *(u8 *)(p + 0x20) == 0xFE || *(u8 *)(p + 0x20) == 0xFD
                || func_00282948((char *)entry + 8) != 0) {
                entry->field04 = 0;
            }
        }
    }
}
