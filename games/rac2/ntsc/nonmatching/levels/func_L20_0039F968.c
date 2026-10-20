/* Not matched (worker 06, level-20 wave of 2026-10-20).

   This body belongs in the `-G8` unit: it reads a float through `$gp`
   (`lwc1 $f12,-0x2B4($gp)`), which only comes out of a file built with
   `-G8`, so it cannot live in the -G0 unit
   (src/levels/20_yeedil/L20_0039C448.c).

   Its one 128-bit copy is `lq $v0,0x0($a0)` / `sq $v0,0x0($a1)` - both
   sides are parameters, so this compiler does reproduce that form; the
   `$gp` access is what keeps it out of reach here.

   The body is 996 bytes with a frame of 0x250. */
