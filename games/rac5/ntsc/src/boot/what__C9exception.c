struct exception {
    void **vptr;
};

char *what__C9exception(const struct exception *self) {
    typedef void *(*rtti_fn)(const void *);
    rtti_fn type_info_of = (rtti_fn)self->vptr[1];
    return *(char **)type_info_of(self);
}
