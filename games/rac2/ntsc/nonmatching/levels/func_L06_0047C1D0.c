/* No attempt: the retail bytes are not code. Six words at 0x47C1D0: 0x03E00008, 0,
   0x03E00008, 0, 0x03E00008, 0 -- three plain `jr $ra; nop` stubs in a row, sitting
   between the epilogue of the function ending at 0x47C1CC and the next function at
   0x47C1E8. One 24-byte C function cannot emit three returns; this is either a
   catalogue split error (three empty functions) or padding. No C can be placed. */
