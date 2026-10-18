/* newlib strtod, retail 0x01ECA180 (40 bytes + padding):
 *   return _strtod_r(_impure_ptr, str, endptr);
 */
struct _reent;

extern struct _reent *_impure_ptr;
extern double _strtod_r(struct _reent *ptr, const char *str, char **endptr);

double strtod(const char *str, char **endptr) {
    return _strtod_r(_impure_ptr, str, endptr);
}
