/* 0 of N places match - no C can be placed.
   The whole extent is ddiu $sp,$sp,+N; nop pairs between two real functions
   (the previous one ends with jr $ra / addiu $sp,$sp,-N, the next starts with a
   real prologue). Catalogue defect: the splitter invented a function over padding
   or a dead-stripped stub tail that carries no body. Recorded for the maintainers;
   no source form reproduces it. */

/* words: 27BD0030 00000000 27BD0030 */