extern void func_00282CF0(void *, void *, void *);
extern unsigned int func_0031AB08(unsigned int);
extern void func_0031AB18(void *, unsigned int, void *, int);

void func_0031AAB0(char *object)
{
    char *data = *(char **)(object + 0x68);
    char *other = *(char **)(data + 0x30);
    if (other != 0) {
        float vector[4];
        func_00282CF0(vector, (float *)(other + 0x10), (float *)(object + 0x10));
        func_0031AB18(object, func_0031AB08((unsigned int)object), vector, 0);
    }
}
