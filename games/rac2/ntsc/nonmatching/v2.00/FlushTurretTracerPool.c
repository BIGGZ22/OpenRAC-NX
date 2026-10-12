extern char g_turretTracerPool[];
extern void func_003011C0(char *pool);

void FlushTurretTracerPool(void)
{
    func_003011C0(g_turretTracerPool);
}
