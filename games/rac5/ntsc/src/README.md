# Authored matching source

`boot/` holds the thirty-one accepted boot C definitions (1,064 bytes).
`boot/memcard_busy.c` was the first; thirty further reviewed bodies were promoted
from `nonmatching/boot/` in one documented review of their boundaries, symbols,
callers and behaviour. Their pinned compiler, byte extents and complete boot load
image are checked by `tools/boot_trial_gate.py` with a lawful local ELF. The
accepted set is enumerated in `tools/boot_c_campaign.py` and pinned in
`config/boot/compiler_trial.json`. See `docs/BOOT-MATCH-SCOPE.md` for the exact
acceptance scope and limitations.
Other candidates remain in `nonmatching/` until independently reviewed.
`levels/frontend/func_00002F80.c` reconstructs one 476-byte frontend New Game
initializer. The level C gate checks its complete SNR2 module and measured
relocated image; the source keeps unnamed structure fields as offsets where
their original names and layouts are not established.
`levels/shared/func_00000100.c` is one additional 8-byte function accepted
at all twenty numbered level placements by the
[shared level C gate](../docs/LEVEL-C-WITNESS.md).
