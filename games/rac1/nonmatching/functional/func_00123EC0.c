/* functional: high. fcheck: unchecked (never called in fcheck's capture run of the title, menu and level 0; OpenRAC tools/fcheck_gate.py, 2026-10-18). Calls func_00119678 with the third argument (a2, the thread id) and returns. The retail sync and ei (enable interrupts) are not modelled: the host has no interrupt state to change, and a1 and a0 are never read by the assembly. */
extern void func_00119678(int thread_id);

void func_00123EC0(int a0, unsigned short a1, void *a2)
{
    (void)a0;
    (void)a1;
    func_00119678((int)a2);
}
