/* No attempt: the retail bytes are not one function. 24 bytes at 0x43E8C0:
   `jr $ra` / nop, three times over (0x03E00008, 0, repeated three times), i.e. three
   merged empty bodies. The extents were built without splitting them, so no single C
   body can be placed; a tools/levels.py split is what unblocks it. */
