/* Not attempted in this run (budget). It belongs to the 0x123xxx record
   parser whose neighbours are already matched in src/core/001232F0.c:
   func_00123530/func_00123578 decode 7-bit varints and func_00123978 walks a
   `record[0]`-strided list collecting each record into an output array while
   tracking the minimum and maximum `record[2] + record[3]` addresses. This
   one runs the same walk to sum `func_00123930(record)` over one of the two
   lists (the other falls back to `func_00123930` on a single entry), stores
   the count at `arg0 + 0x10`, allocates two `count * 4` arrays with malloc,
   then re-walks the list calling func_00123978 and finally func_001236C8.
   No upstream source: it is game-format code, so it has to be decoded from
   the disassembly. Ordinary C, no barriers, no VU. */
