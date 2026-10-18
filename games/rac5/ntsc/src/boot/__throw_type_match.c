extern int strcmp(const char *a, const char *b);

void *__throw_type_match(const char *thrown, const char *caught, void *ptr) {
    void *result = ptr;
    if (strcmp(thrown, caught) != 0)
        result = 0;
    return result;
}
