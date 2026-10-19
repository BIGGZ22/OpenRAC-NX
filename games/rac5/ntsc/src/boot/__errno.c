/* newlib __errno, retail 0x01EC60A8 (16 bytes).  _impure_ptr (0x01EFE72C) points
 * at struct _reent whose first member is _errno, so __errno just returns it.
 */
struct _reent;

extern struct _reent *_impure_ptr;

struct _reent *__errno(void) {
    return _impure_ptr;
}
