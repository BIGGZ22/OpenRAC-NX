/* _calloc_r -- RAC5 boot body, w11/s1.
 *
 * Provenance: newlib libc/stdlib/mallocr.c (Doug Lea malloc, DEFINE_CALLOC),
 * EE newlib snapshot D:/RAC2/master/build/sdk-compiler-build/source/ee/newlib.
 * Body verbatim (minus the lock and MORECORE_CLEARS cases, which retail does
 * not emit); chunksize/mem2chunk/SIZE_SZ restated from the same file with
 * INTERNAL_SIZE_T = size_t = 32 bits, so SIZE_SZ = 4 and csz is read at
 * mem-4. MALLOC_ZERO is the 9*sizeof variant, byte-for-byte the macro of that
 * snapshot. Retail: 0x01ED0758, interval 264 bytes. Toolchain: SN ProDG 3.01
 * ee-gcc2953 (GCC 2.95.3) -O2 -G0, Ps2EeAs.exe, SN ld.exe.
 */
typedef unsigned int size_t;

struct _reent;

extern void *_malloc_r (struct _reent *, size_t);
extern void *memset (void *, int, size_t);

#define SIZE_SZ 4
#define PREV_INUSE 1
#define IS_MMAPPED 2

#define chunksize(p) ((p)[1] & ~(PREV_INUSE | IS_MMAPPED))

#define MALLOC_ZERO(charp, nbytes) \
do { \
  size_t mzsz = (nbytes); \
  if (mzsz <= 9*sizeof (mzsz)) { \
    size_t *mz = (size_t *) (charp); \
    if (mzsz >= 5*sizeof (mzsz)) { *mz++ = 0; \
                                   *mz++ = 0; \
      if (mzsz >= 7*sizeof (mzsz)) { *mz++ = 0; \
                                     *mz++ = 0; \
        if (mzsz >= 9*sizeof (mzsz)) { *mz++ = 0; \
                                       *mz++ = 0; }}} \
                                   *mz++ = 0; \
                                   *mz++ = 0; \
                                   *mz   = 0; \
  } else memset ((charp), 0, mzsz); \
} while (0)

void *
_calloc_r (struct _reent *ptr, size_t n_elements, size_t elem_size)
{
  size_t sz = n_elements * elem_size;
  size_t *p;
  void *mem;
  size_t csz;

  mem = _malloc_r (ptr, sz);
  if (mem == 0)
    return 0;
  p = (size_t *) ((char *) mem - 2 * SIZE_SZ);
  csz = chunksize (p);
  MALLOC_ZERO (mem, csz - SIZE_SZ);
  return mem;
}
