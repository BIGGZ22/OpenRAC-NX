/* newlib strtoul, retail 0x01ECA3C8 (52 bytes + padding):
 *   return _strtoul_r(_impure_ptr, str, endptr, base);
 */
struct _reent;

extern struct _reent *_impure_ptr;
extern unsigned long _strtoul_r(struct _reent *ptr, const char *str, char **endptr, int base);

unsigned long strtoul(const char *str, char **endptr, int base) {
    return _strtoul_r(_impure_ptr, str, endptr, base);
}
