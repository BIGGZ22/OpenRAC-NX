extern char *D_L00_001B1E9C;
extern char *D_L00_001B1EA0;

/* Scans the record table from its start to its end pointer in 0x100-byte steps and returns the entry whose key follows the first record whose 16-bit key at +0xB2 equals the second parameter. */
char *func_L00_00358D50(int unused, int key)
{
    char *p = D_L00_001B1E9C;
    char *e = D_L00_001B1EA0;

    if (p != e) {
        do {
            if (*(short *)(p + 0xB2) == key)
                return p;
            p += 0x100;
        } while (p != e);
    }
    return 0;
}
