/* from src/core/0011B1E8.c: not a C candidate - the retail body is a cache
 * routine, and symbol_addrs/usa/symbol_addrs.txt already names it:
 * `_sceIDC = 0x0011B458; // type:func size:0xA4 - name (v2.00 Promises or SDK
 * library match), digest match`.
 *
 * Evidence from the bytes: the body is built out of instructions cc1 never
 * emits from C - `sync` (0x0000000F), `cache 0x10/0x16` (0xBCD00000 /
 * 0xBCD60000) and `mfc0 $v0,TagLo` (0x4002E000) - over a loop that walks a
 * 0x40-byte stride up to 0x1000, invalidating and writeback-invalidating each
 * line, with two range tests against the caller's (a0,a1) before each cache
 * op. AGENTS.md lists "instructions the compiler never emits" as the
 * handwritten criterion, and the inline-assembly rule leaves no C form.
 *
 * Recommendation (maintainer decision): ASM_FUNC. Same family as the
 * func_0011B3D0 / func_0011B500 stubs already marked around it.
 */
