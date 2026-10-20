/* from src/core/00115200.c: strlen. symbol_addrs/eu: "SDK strlen(s) hand-written r5900
   strlen.S - scans for the zero byte 16 then 8 then 1 byte at a time"; the file it sits in
   is one of the two 16-byte-slot units of docs/COMPILER-NOTES.md. Handwritten assembly:
   no C is written. */
