/* Not attempted in this run.
   396 bytes, the largest of the lot: a conditional field extraction with four
   `bgezl`/`bltzl` early exits, three calls to func_00121AB8, and a packed word
   built from six shifted fields (<<5, <<6, <<8, <<3, <<7) stored through two
   out-pointers. No C was compiled for it; the shapes are ordinary, so it is
   worth a real attempt. */
