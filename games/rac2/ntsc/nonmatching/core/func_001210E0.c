/* from src/core/00120390.c: no C attempt - the shape was not identified in
 * this wave, so this is a routing note rather than an attempt.
 *
 * What the bytes say: a 0x700-byte frame that saves $fp, $ra, $s0-$s7 and the
 * twelve float registers $f20-$f31 (`swc1` at 0x6A0..0x6F8) - the frame size
 * and the float saves are the tell for a transform/geometry routine that
 * keeps a big local block at $fp+0x2F0 (0x600 bytes of it).
 *
 * It calls one function through the global pointer D_00135D38 (0x135D38),
 * the object factory its neighbours func_00120420/func_00120448/func_00120470
 * use, stores the single argument into the object (`obj+0xC = arg0`) and,
 * when `obj+0x8` is 0, calls func_00120368 first. It then calls
 * func_001240C8(0x12118C, &local) - a call whose first argument is a function
 * address in this same unit, i.e. a callback/thunk registration.
 *
 * What would unblock it: reading func_001240C8 and the object produced by
 * D_00135D38 first (the surrounding 00120390.c unit's matched functions only
 * cover the small accessors), then writing the body against that structure.
 * The float-register saves mean the body is worth doing once the object layout
 * is known - the prologue alone pins the -O2 -G0 profile (sd saves, no
 * 16-byte slots).
 */
