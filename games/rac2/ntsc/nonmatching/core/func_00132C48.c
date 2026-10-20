/* from src/core/00131780.c: 989snd's snd_SendCommandSync
 * (symbol_addrs/usa-v2 0x00132c48, role CONFIRMED - blocking sceSifCallRpc on
 * the command channel 0x1A7040). Candidate is 432 bytes against retail's 420;
 * one compile was spent on the layout (see below) before the shared root cause
 * below was recognised, so it is recorded rather than chased further.
 *
 * The body is understood end to end and is written here in full: copy the
 * payload into the 0x1A70C0 staging buffer, drain the pump flag (gp-0x7B70),
 * func_001322B8(0x1A7080, 1), loop on func_0011D810(0x1A7040) printing through
 * func_00115D48(0x1A7578) while gp-0x7AF8 is clear, then the nine-argument
 * sceSifCallRpc - which is why the call takes (chan, fno, mode, send, ssize,
 * receive, rsize, end, param) with the 0x1A7040 channel, the command as fno,
 * mode 1, 0x1A7080 as the reply area and 0xC as its size - followed by a
 * func_00132220() drain loop, a gp-indexed table walk (table gp-0x7B50 indexed
 * by gp-0x7B30, entry field 0) and the conditional func_00133108 flush.
 *
 * The residual is the same one func_00119AC0 hit, and it is not specific to
 * this unit: this cc1 CSEs every symbolic address (`D_001A7040` three times,
 * `D_001A70C0` twice, `D_001A7080` twice) into callee-saved registers and
 * keeps them live across the calls, so the frame is 0x50 with $s0-$s6 saved
 * where retail's is 0x30 with $s0-$s2 and re-materialises each address with a
 * fresh `lui`/`addiu` inside the argument setup. It also hoists the
 * `gp-0x7B70` load into the prologue where retail loads it inside the loop.
 * Unblocked by a source shape (or a declaration) that stops the address CSE;
 * the unit needs the game compiler with -G8 and the sda attributes, which this
 * file already has.
 */
