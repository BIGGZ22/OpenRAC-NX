/* Not attempted in this run.
 *
 * src/core/0012E110.c line 46. 448 bytes, and per symbol_addrs a "chained
 * IPU colour conversion in cod/022FA8" whose error string D_0013BCC0 reads
 * "CSC handler error"; func_0012E608 (compiled C since task 2055) calls it.
 * The body is ordinary C - no VU0, no MMI, no `sync` - so it is a C target;
 * it just had no upstream source to transcribe from, unlike the two
 * functions this run did finish.
 *
 * What is known from the head: `addiu $a3,$zero,0x3FF` before the frame is
 * even built, then `div $zero,$a2,$a3` - the row argument is divided by 1023
 * into a quotient/remainder pair, and the 0xF constants that follow are the
 * IPU colour-conversion selection words. rac1's libmpeg neighbour
 * src/core/0012A2F0.c holds _doCSC and its callers (func_0012A720 is the
 * same address as this one minus the usual +0x3C58), but that file is
 * INCLUDE_ASM over this range in rac1 too, so it gives the idioms and not
 * the body.
 */
