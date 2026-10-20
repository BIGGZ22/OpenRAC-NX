/* func_002B0C28 (184 bytes, all 27 levels): the body is right - same size, same block layout,
   same branches, same three stores, the same return values - and only the *address* form of
   the three globals differs, because this cc1 splits the %hi/%lo pair of a symbol address
   while retail keeps it in one piece:
     retail: lui $v0,0x1B ; lw $v0,-0x739C($v0)          (the load, right after the %hi)
             lui $at,0x1B ; sw $s0,0x1D48($at)           (the stores, each with its own %hi)
     this cc1 (default profile): lui $v0,%hi ; ... ; lw $v0,0($v0)   (the %hi hoisted to the
             top of the block, the load scheduled down to its use) and lui $v0,%hi ;
             sw $s0,0($v0) (the address built in a register, the store at offset 0).
   Two source facts are needed to get this far and both are measured:
     - the global that is read and written must be under TWO names (`D_001B8FB4` for the read,
       `D_001B8FB4_b` for the write, and the same for `D_001C2088`), because this cc1 CSEs a
       symbol address into a callee-saved register and keeps it across the call (the wall
       recorded in docs/COMPILER-NOTES.md); with one name the body is 204 bytes with the
       address in $s4 and two extra saves, with two names it is retail's 184 bytes and the
       only difference left is the %hi/%lo split above.
     - the fifth global is reached through $gp: `D_001A8FB0` is at 0x001A8FB0 = $gp-0x6040, so
       it is declared `extern int D_001A8FB0 __attribute__((sda));` and the store comes out
       `sw $v0,-0x6040($gp)` (the sda attribute alone is enough, the unit's -G0 profile keeps
       the gp-relative form).
   What would unblock it: the unit's profile. Compiled with `-mno-split-addresses` the body
   is retail's byte for byte (the compiler then emits `lw $2,D_001B8FB4` and `sw $16,D_001C2088`
   and the assembler expands them into retail's `lui %hi ; op %lo` pairs), so the two
   functions belong in a file of their own with
       src/game/002B00B0-g8.c	-O2 -G8 -ffunction-sections -mno-split-addresses
   in config/cflags.txt, which this wave's assignment does not allow me to add. */
extern int D_001B8FB4;
extern int D_001B8FB4_b;
extern int D_001B8FB0;
extern int D_001C2088;
extern int D_001C2088_b;
extern int D_001A8FB0 __attribute__((sda));
extern int func_00288FB8(int);
extern void func_L00_0030DDB0(int, int);

int func_002B0C28(int a0, int a1, int a2)
{
    int current = D_001B8FB4;

    if (current == a0) {
        if (a1 != 0)
            func_L00_0030DDB0(func_00288FB8(a1), a2);
        D_001C2088 = a1;
        D_001A8FB0 = 2;
    } else {
        if (current != 0)
            return 0;
        if (a1 != 0)
            func_L00_0030DDB0(func_00288FB8(a1), a2);
    }
    D_001B8FB4_b = a0;
    D_001B8FB0 = 2;
    D_001C2088_b = a1;
    return 1;
}
