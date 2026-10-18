/* newlib strtol, retail 0x01ECE0D0 (52 bytes + padding):
 *   return _strtol_r(_impure_ptr, str, endptr, base);
 */
struct _reent;

extern struct _reent *_impure_ptr;
extern long _strtol_r(struct _reent *ptr, const char *str, char **endptr, int base);

long strtol(const char *str, char **endptr, int base) {
    return _strtol_r(_impure_ptr, str, endptr, base);
}
