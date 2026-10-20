/* 0 of 27 places match: exact size (284 B) but the live-range structure differs.
   Two symmetric blocks: state = func_31A768(*(gp-0x1B8), *(0x1AEE3C)) & 0xFF, then
   func_31A728(0xFF) & 0xFF, then func_33A2B0(a1, block_source, pair, state, float, x, 0)
   where pair = D_001AEE28[object->bBC].first / .second (an 8-byte pair table indexed by
   the byte at object+0xBC) and block_source is object->f68 (block 1) or a 16-byte stack
   vector built by func_31A948(local, A*(1.0f/60.0f), B*(1.0f/60.0f)) (block 2).
   The retail keeps the table base in $s4 as `addiu $s4,$gp,-0x1C8` (gp-relative), reads
   object->f68 into $s3 early, and saves s0..s5+ra in a 0x50 frame; this candidate comes
   out one register short (0x40 frame) with an absolute table base. Next lever: keep the
   table base and the f68 read as variables live across both blocks and let the sda table
   symbol resolve gp-relative. */
