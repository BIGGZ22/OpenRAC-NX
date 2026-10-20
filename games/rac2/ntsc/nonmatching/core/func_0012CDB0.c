/* from src/core/0012C9C8.c: C that compiles to the exact retail size (496
 * bytes) and matches instruction for instruction except at one point, three
 * times over.
 *
 * This is rac1-decomp's func_00128F90 (_pictureCodingExtension, an MPEG-2
 * picture coding extension parser) transcribed with the region's +0x3E20
 * offset; every struct offset (0xD4, 0x164..0x188), every bit count and every
 * IPU_CTRL (0x10002010) mask agrees with retail, and so does the whole
 * prologue, the tail's sibling call (`j func_12C878`) and both epilogues.
 *
 * The residual: for the three read-modify-writes that are NOT the first one,
 * retail sinks the store into the delay slot of the following `jal` while
 * this cc1 puts the store before the call and fills the slot with either a
 * `nop` or the next call's `addiu $a1,$zero,0x1`:
 *
 *   retail : addiu $a1,1 ; lui $at,0x1000 ; jal ; sw $a2,0x2010($at)
 *   here A : addiu $a1,1 ; lui $at,0x1000 ; sw ; jal ; nop      (volatile store)
 *   here B : lui $at,0x1000 ; sw ; jal ; addiu $a1,1            (plain store)
 *
 * A (stores volatile, reads volatile, named temps, as rac1 writes it) gets
 * the argument order right and loses the delay slot; B (single statement, or
 * named temp, with a plain store) gets the delay slot right and loses the
 * argument order - the store is never the instruction this cc1 moves into the
 * slot. Mixing a volatile read with a plain store, swapping the `|` operands,
 * and the `sdk -O2 -g2 -gstabs` profile (which fits the neighbouring unit
 * func_0012D808) all change nothing else. The game compiler 2.95.3 is the
 * wrong compiler here: its prologue differs at +0x0.
 *
 * WAVE 4: the store is not "sunk late" for a scheduling reason - it is an
 * 8-byte insn. Compiling this body with `-da` through the game's cc1 (the same
 * toolchain tools/build.py drives) writes the three store lines as
 *
 *     sw  $6,268443664          (268443664 = 0x10002010)
 *
 * i.e. ONE RTL insn with an absolute constant address, which the ASSEMBLER
 * expands to `lui $at,0x1000 ; sw $a2,0x2010($at)` - two words where the delay
 * slot has room for one, so reorg can never place it in the slot. The first
 * read-modify-write (the one that already matches) reaches its array through
 * $a3 instead: its address is CSEd into a register for the read and the write
 * of the same statement, so its store is a 4-byte `sw $v1,0($a3)` and IS
 * filled. The three failing stores use two different materialisations because
 * the read and the store of a volatile pair are not CSEd into one register
 * (the read keeps the address in $a2, the store reaches it through $at).
 *
 * Measured and rejected in wave 4: `-mno-split-addresses` on a dedicated file
 * (`src/core/0012CDB0.c`, `config/cflags.txt`) - it does NOT change this store
 * out of the macro form and it moves the frame to 0x10 against retail's 0x20
 * (the six incoming-argument stores stop landing in the jal delay slots), so
 * the whole prologue breaks while the three IPU stores stay put.
 * What would close it: a source shape that makes this cc1 keep the address of
 * the store in a general register (the $a3 form) without disturbing the frame.
 */

/* rac1-decomp src/core/00125630.c: func_00128F90 (_pictureCodingExtension).
   Reached from the extension dispatcher func_0012CA68: extension id 0x8 calls
   this function (func_00130300's table) -- it parses an MPEG-2 picture coding
   extension: the four f_codes, intra DC precision, q_scale_type, intra VLC
   format and alternate scan go straight into IPU_CTRL (0x10002010), the rest
   into the decoder state. The composite display fields are skipped. */
typedef struct {
    unsigned char pad_0[0xD4];
    int pict_struct0;               /* 0xD4 */
    unsigned char pad_D8[0x8C];
    int f_code00;                   /* 0x164 */
    int f_code01;                   /* 0x168 */
    int f_code10;                   /* 0x16C */
    int f_code11;                   /* 0x170 */
    int picture_structure;          /* 0x174 */
    int top_field_first;            /* 0x178 */
    int frame_pred_frame_dct;       /* 0x17C */
    int concealment_mvs;            /* 0x180 */
    int repeat_first_field;         /* 0x184 */
    int progressive_frame;          /* 0x188 */
} PicExt_0012CDB0;

extern int func_0012C878(void *arg0, int arg1);

void func_0012CDB0(void *arg) {
    PicExt_0012CDB0 *arg0 = arg;
    int temp_2_35;
    int temp_6_65;
    int temp_6_77;
    int temp_6_89;

    arg0->f_code00 = func_0012C878(arg0, 4);
    arg0->f_code01 = func_0012C878(arg0, 4);
    arg0->f_code10 = func_0012C878(arg0, 4);
    arg0->f_code11 = func_0012C878(arg0, 4);
    *(int *)0x10002010 = (*(int *)0x10002010 & 0xFFFCFFFF) | (func_0012C878(arg0, 2) << 0x10);
    temp_2_35 = func_0012C878(arg0, 2);
    arg0->picture_structure = temp_2_35;
    if (arg0->pict_struct0 == 0) {
        arg0->pict_struct0 = temp_2_35;
    }
    arg0->top_field_first = func_0012C878(arg0, 1);
    arg0->frame_pred_frame_dct = func_0012C878(arg0, 1);
    arg0->concealment_mvs = func_0012C878(arg0, 1);
    temp_6_65 = (*(volatile unsigned int *)0x10002010 & 0xFFBFFFFF) | (func_0012C878(arg0, 1) << 0x16);
    *(volatile unsigned int *)0x10002010 = temp_6_65;
    temp_6_77 = (*(volatile unsigned int *)0x10002010 & 0xFFDFFFFF) | (func_0012C878(arg0, 1) << 0x15);
    *(volatile unsigned int *)0x10002010 = temp_6_77;
    temp_6_89 = (*(volatile unsigned int *)0x10002010 & 0xFFEFFFFF) | (func_0012C878(arg0, 1) << 0x14);
    *(volatile unsigned int *)0x10002010 = temp_6_89;
    arg0->repeat_first_field = func_0012C878(arg0, 1);
    func_0012C878(arg0, 1);
    arg0->progressive_frame = func_0012C878(arg0, 1);
    if (func_0012C878(arg0, 1) != 0) {
        func_0012C878(arg0, 1);
        func_0012C878(arg0, 3);
        func_0012C878(arg0, 1);
        func_0012C878(arg0, 7);
        func_0012C878(arg0, 8);
    }
}
