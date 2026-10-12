#ifndef INCLUDE_ASM_H
#define INCLUDE_ASM_H

/* The same source markers as rac1-decomp's include/include_asm.h, so a file moves between the
   OpenRAC decompilations unchanged. In this repository they expand to nothing: tools/build.py
   takes every function without C from the retail program's own disassembly at link time, in
   its place, so a marker only records where in the file the function sits. */
#define INCLUDE_ASM(FOLDER, NAME)
#define INCLUDE_RODATA(FOLDER, NAME)
#define ASM_FUNC(FOLDER, NAME) INCLUDE_ASM(FOLDER, NAME)
#define LINKER_REMNANT(FOLDER, NAME) INCLUDE_ASM(FOLDER, NAME)

#endif /* INCLUDE_ASM_H */
