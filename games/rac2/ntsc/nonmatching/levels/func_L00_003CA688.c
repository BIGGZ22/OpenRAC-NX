INCLUDE_ASM("asm/levels", func_L00_003CA650);


extern void func_002EE5B0(void *a0, void *a1, void *a2);
extern void func_002EE108(void *a0, void *a1, void *a2);
extern void func_002EC608(void *a0, unsigned int a1);
extern void func_002EC970(void *a0, void *a1, unsigned int a2, int a3, float f12, float f13);

/* Builds one 16-byte record (a default value with two angles overwritten) and walks it through three helpers. */
void func_L00_003CA688(char *owner)
{
    Rac2Native_9cdc323a4d0c2fbd_Quad local;
    TI128 blank;
    char *inner;

    blank = 0;
    local.q = blank;
    local.vector.vector_00 = -0.3f;
    local.vector.vector_0C = 1.0f;
    func_002EE5B0(&local, &local, owner + 0xC0);
    inner = *(char **)(owner + 0x68);
    func_002EE108(&local, &local, owner + 0x10);
    func_002EC608(&local, 0x103080);
    func_002EC970(&local, inner + 0x10, 0x103080, 4, *(float *)(inner + 0x64), 0.25f);
}
