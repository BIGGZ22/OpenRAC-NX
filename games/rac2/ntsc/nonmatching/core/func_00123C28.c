/* from src/core/001232F0.c: extract_cie_info, exact at check.py but not linkable as C */
/*
 * The body below matches tools/check.py at 1 of 1, but tools/build.py refuses it with
 * `func_00123C28: needs .sdata at its place, which is not C` - the same wall as the switch
 * tables: the placement needs a data section the C cannot carry. Kept here with the
 * measurement; the marker is back in the unit.
 */
void *func_00123C28(fde *f, struct cie_info *c) {
    unsigned char *p;
    int i;

    c->augmentation = get_cie(f)->augmentation;

    if (func_00115544(c->augmentation, "") != 0
        && func_00115544(c->augmentation, "eh") != 0
        && c->augmentation[0] != 'z')
        return 0;

    p = (unsigned char *)c->augmentation + func_001157AC(c->augmentation) + 1;

    if (func_00115544(c->augmentation, "eh") == 0) {
        c->eh_ptr = read_pointer(p);
        p += sizeof(void *);
    } else
        c->eh_ptr = 0;

    p = func_00123530(p, (unsigned int *)&c->code_align);
    p = func_00123578(p, &c->data_align);
    c->ra_regno = *p++;

    if (c->augmentation[0] == 'z') {
        p = func_00123530(p, (unsigned int *)&i);
        p += i;
    }

    return p;
}
