int CalcSaveSectionsSize(int *section)
{
    int size = 8;

    if (*section) {
        do {
            size += 8;
            size += section[1];
            size = (size + 3) & -4;
            section += 4;
        } while (*section);
    }
    return size + 8;
}
