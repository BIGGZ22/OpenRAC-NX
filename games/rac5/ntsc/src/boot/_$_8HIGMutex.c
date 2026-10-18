typedef struct {
    int sema;
} HIGMutex;

extern int DeleteSema(int);
extern void __builtin_delete(void *);

void hig_mutex_dtor(HIGMutex *self, int flags) asm("_$_8HIGMutex");

void hig_mutex_dtor(HIGMutex *self, int flags) {
    if (self->sema >= 0)
        DeleteSema(self->sema);
    if (flags & 1)
        __builtin_delete(self);
}
