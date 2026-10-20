/* Not attempted in this run - and the first thing the next worker needs is
 * the profile, not the C.
 *
 * src/core/00131780.c line 199. snd_BankLoadByLoc (symbol_addrs: "sync bank
 * load by CD loc, rpc#3"). Four of its five globals are reached
 * gp-relatively (`lw -0x7B28($gp)`, `-0x7AF8`, `-0x7E70`; `sw -0x7B68`,
 * `-0x7E30`), so this body belongs in a `src/core/00131780-g8.c` sibling
 * (the *-g8.c name alone selects -G8 via tools/build.py:210) with those
 * symbols declared `__attribute__((sda))` - the same split func_00133250
 * and func_00133640 already use. With the unit's current `sdk -O2 -G2`
 * line the prologue cannot come out.
 *
 * Everything it calls is already named or pinned: func_00133580(1)
 * (snd_CheckLoadInProgress, the sibling above), func_00115D48 (the
 * assertion/printf arm, three call sites), func_00132028 (snd_Pump),
 * func_0011AEA0, func_0011D810 (sceSifCheckStatRpc) and func_0011D620
 * (sceSifCallRpc, rpc number 3 - the `lui $a1` at +0xE0 and the
 * `jal func_11D620` at +0xFC with a0 = 0x137140, a1 = 3, a2 = 1,
 * a3 = 0x1371C0, t0 = 8, t1 = 0x137180, t2 = 4, t3 = 0).
 *
 * Why it was left: it needs a new sibling file and the -g8 declarations
 * before the first byte of C is meaningful, and this run's budget went to
 * the two functions that had upstream source (func_00123C28, func_00122B00).
 * It is ordinary C - no VU0, no MMI, no `sync` anywhere in the 384 bytes.
 */
