extern int g_mapCurrentLevel;
extern void func_00296520(void);

void MapSetCurrentLevel(int level)
{
    g_mapCurrentLevel = level;
    func_00296520();
}
