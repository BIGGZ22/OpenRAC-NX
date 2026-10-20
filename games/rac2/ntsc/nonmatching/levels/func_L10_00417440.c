/* 0 of 1 places match - last line: 10_hrugis_cloud: 140 bytes, expected 136.
   The head is byte-exact: `lui %hi(D_L10_00265BA0+0x2FE0)` / the two `sw $zero,%gprel` on
   D_001B0F44/D_001B0F48 / the 128-iteration clear loop with its three nops and `bgez`.
   Two differences, both compiler-side, both in the trailing list walk:
   (a) retail materialises the arena base in THREE words (`lui $v0,0x26` / `addiu $v0,$v0,0x5BA0`
       / `addiu $v0,$v0,0x2FE0`, so the symbol's own %lo comes first and the 0x2FE0 offset after);
       here the cc1 folds the offset into the reloc and emits two (`lui %hi(sym+0x2FE0)` /
       `addiu %lo`), one word short;
   (b) retail keeps the `D_001B251C` address build INSIDE the loop (`lui $v1,0x1B` next to
       `lw $v1,0x251C($v1)`, one register reused), here the cc1 hoists the `lui` out and sinks
       it into the `beqz` delay slot, and it also drops retail's `daddu $a0,$v0,$zero` copy.
   What would unblock it: a spelling that stops LICM from hoisting the absolute address halves
   of a loop-invariant pointer load, and one that keeps `sym + const` unfolded. */
