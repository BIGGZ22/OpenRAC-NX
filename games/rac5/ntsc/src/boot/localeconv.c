/* newlib localeconv, retail 0x01ECE140 (40 bytes):
 *   lw $a0,-0x18D4($v0) ; jal _localeconv_r ; epilogue
 */
struct lconv;
struct _reent;

extern struct _reent *_impure_ptr;
extern struct lconv *_localeconv_r(struct _reent *ptr);

struct lconv *localeconv(void) {
    return _localeconv_r(_impure_ptr);
}
