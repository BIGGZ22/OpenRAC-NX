/* 64-byte near miss: 56-byte C body matches retail from +8; retail starts with addiu sp,+0x50; nop before the standard frame. */
extern int func_00282978(short *);
extern void func_002B9180(void *);
void func_002C38A0(char *self)
{
    if (func_00282978((short *)(self + 10)) != 0)
        func_002B9180(self);
}
