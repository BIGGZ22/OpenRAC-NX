extern void FileLoadPump(void);
extern void func_001336D0(void (*pump)(void));

void InstallFileLoadPump(void)
{
    func_001336D0(FileLoadPump);
}
