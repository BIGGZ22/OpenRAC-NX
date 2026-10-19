/* newlib vfprintf, retail 0x01ECAD98 (208 bytes), exact under SN ProDG 3.01
 * (ee-gcc2953, -O2 -G0) with the switch table pooled at 0x01F26CE0.
 *
 * Sony's entry point scans the format for a conversion that needs the
 * floating-point formatter -- 'E', 'G', 'e', 'f', 'g', plus 'L' because the
 * long-double modifier implies a float conversion.  Any such conversion routes
 * the whole call to _vfprintf_r; otherwise the integer-only _vfiprintf_r runs.
 *
 * The scan stops at the first byte >= 'A' after '%', so a specifier such as
 * "%08x" skips its flags/digits.  `c' is a signed char on purpose: the retail
 * offsets the value by 'E' and sign-extends the byte before the range test,
 * which is what makes the six cases one 0x23-entry jump table.
 */
struct _reent;
typedef struct _FILE {
    unsigned char _pad0[0x54];
    struct _reent *_data;
} FILE;

extern int _vfprintf_r(struct _reent *, FILE *, const char *, void *);
extern int _vfiprintf_r(struct _reent *, FILE *, const char *, void *);

int vfprintf(FILE *fp, const char *fmt, void *ap) {
    const char *p = fmt;
    signed char c;

    while (*p != 0) {
        if (*p == '%' && p[1] != 0) {
            p++;
            while (*p < 'A' && p[1] != 0)
                p++;
            c = *(unsigned char *)p - 'E';
            switch (c) {
            case 'E' - 'E':
            case 'G' - 'E':
            case 'L' - 'E':
            case 'e' - 'E':
            case 'f' - 'E':
            case 'g' - 'E':
                return _vfprintf_r(fp->_data, fp, fmt, ap);
            }
        }
        p++;
    }
    return _vfiprintf_r(fp->_data, fp, fmt, ap);
}
