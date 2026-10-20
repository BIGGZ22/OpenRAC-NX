/* func_002B0CE0 (124 bytes, all 27 levels): same body as the deposit of func_002B0C28 and the
   same first blocker - this cc1 splits the %hi/%lo pair of the globals' addresses where retail
   keeps them together:
     retail: lui $at,0x1B ; sw $s0,-0x7050($at) ; lui $at,0x1B ; sw $s2,-0x704C($at) ;
             lui $at,0x1B ; sw $s1,0x2088($at)      (each store with its own %hi, adjacent)
     this cc1: lui $v0,%hi ; sw $s0,0($v0) ...       (the %hi hoisted, the store at offset 0)
   Two more differences are left in this one, both about the layout of the `a1` test:
     - retail branches `beql $s1,$zero` with the $gp store in its slot and the two calls after
       it; this cc1 branches `bnez $s1` over the store and puts the store in the `b`'s slot.
     - retail materialises the return value 3 between the first and the second store, this cc1
       after all three.
   The control flow is otherwise retail's: the helper's own return value is kept when it is not
   zero (the `bnel` with the register restore in its slot), the two arms end on the same two
   stores and the value 3.
   The globals that are read and written need two names each, as in func_002B0C28's deposit,
   and the $gp one needs the sda attribute.
   What would unblock it: the unit's profile - `-mno-split-addresses` on a file of its own,
   `src/game/002B00B0-g8.c -O2 -G8 -ffunction-sections -mno-split-addresses` in
   config/cflags.txt, which this wave's assignment does not allow me to add. */
extern int D_001B8FB4;
extern int D_001B8FB4_b;
extern int D_001B8FB0;
extern int D_001B8FB0_b;
extern int D_001C2088;
extern int D_001C2088_b;
extern int D_001A8FB0 __attribute__((sda));
extern int func_00288FB8(int);
extern void func_L00_0030DDB0(int, int);
extern int func_002B0C28(int, int, int);

int func_002B0CE0(int a0, int a1, int a2)
{
    int result = func_002B0C28(a0, a1, a2);

    if (result != 0)
        return result;
    if (a1 == 0) {
        D_001A8FB0 = a2;
    } else {
        func_L00_0030DDB0(func_00288FB8(a1), a2);
        D_001B8FB0_b = a2;
    }
    D_001B8FB4_b = a0;
    D_001C2088_b = a1;
    return 3;
}
