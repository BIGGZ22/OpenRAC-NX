/* fcheck: unchecked (never called in fcheck's capture run of the title, menu and level 0; OpenRAC tools/fcheck_gate.py, 2026-10-18). */
/* strchr: the first byte of the string a0 equal to a1 (taken as an
   unsigned char), or 0 if there is none. A1 of 0 finds the terminator.
   Retail scans 8 or 16 bytes at a time with MMI; the result is the same. */
char *func_00116428(char *s, int c) {
    unsigned char uc = (unsigned char)c;
    for (;; s++) {
        if ((unsigned char)*s == uc) {
            return s;
        }
        if (*s == 0) {
            return 0;
        }
    }
}
