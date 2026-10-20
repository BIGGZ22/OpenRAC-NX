/* Not C: R5900 MMI instructions.
   A word-at-a-time memcpy (8-byte alignment test, the 0x80808080... magic for
   a zero-byte scan, 16-byte and 8-byte copies, byte tail). The loop bodies are
   opcode 0x1C, the R5900's MMI set (pcpyld/por/pand...), which rabbitizer
   prints as `.word 0x... # INVALID`. No C accepted by this compiler produces
   them, so this is handwritten SDK assembly: ASM_FUNC on that evidence. */
