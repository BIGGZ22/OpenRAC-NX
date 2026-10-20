/* Not attempted in this run (budget). Same 0x123xxx record parser as
   func_00123A00 (src/core/001232F0.c holds the matched neighbours
   func_00123530/func_00123578/func_00123978). A 0x3C0 frame holding a
   0x2F8-byte scratch record that is memset and then copied 0x20 bytes at a
   time (ld/sd quads) to the caller's output; it walks the varint stream with
   func_00123530/func_001236C8 and func_00123D30, uses -- with the byte
   comparison against 0x7A -- the same record-list walk, and finally assigns
   func_001236C8's result plus two stream fields into the object it was given
   (offsets 0x00, 0x04, 0x0C). It is 4 bytes over the 0x2F8 scratch, so the
   frame and the copy loop are the first things to pin down. Needs the game
   compiler's -O2 -G0 shape (the unit's marker file already carries
   `sdk -O2 -G2`); decode from the disassembly, no upstream source. */
