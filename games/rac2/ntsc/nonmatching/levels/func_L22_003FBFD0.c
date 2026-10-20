/* Not attempted in this run - decoded structure and the measured blockers.

   An eight-iteration loop (counter 7..0, `bgez`), each iteration advancing two
   pointers by 0x10 and making two per-level calls:

     a = 0x255820 (22) / 0x2561E0 (23)   the first and second argument of both
                                         calls, advanced at the end of the body
     b = 0x2558A0 (22) / 0x256260 (23)   advanced between the two calls
     call 1: (a, a, b) with f12 = the $gp float at 0x1C70 (22) / 0x1CC0 (23)
     call 2: (a, a)    with f12 = the $gp float at 0x1C6C (22) / 0x1CBC (23)

   The second call has no a2 set-up: whatever the first callee left in a2 is
   what the second receives, so its third parameter is either absent from the
   source or deliberately not passed.
   Callees, per level: 0x00325B50 (22) / 0x0032A268 (23) and
   0x002F3D60 (22) / 0x002F8440 (23).

   Blockers measured before writing C:
   - both f12 loads are the one-instruction `lwc1 $f12,%lo($gp)`, so the two float
     cells need their own -G8 unit with a size on each `sda` declaration (the
     route func_L00_002FD480.c in this directory describes);
   - the two pointer bases are materialised absolutely (lui + addiu, no $at):
     `nosda` declarations at the level-22 addresses above.
   Nothing in the body is VU0/MMI/`sync`, and there is no jump table. */
