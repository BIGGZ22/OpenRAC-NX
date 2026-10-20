/* func_002A90C8 (36 bytes, boot + 22 levels): the call shape, the argument registers and the
   epilogue are exact; one instruction differs. Retail materialises the address of the level's
   own table as a pair and leaves the jal's delay slot empty:
       addiu $sp,$sp,-0x10 ; lui $t0,0x1B ; addiu $t0,$t0,-0x70E0 ; sd $ra,0($sp)
       jal func_002A8F70 ; nop ; ld $ra,0($sp) ; jr $ra ; addiu $sp,$sp,0x10
   This cc1 hoists the %hi into $t0, keeps the %lo for the jal's delay slot and so emits 32
   bytes with the pair split:
       addiu $sp,$sp,-0x10 ; lui $t0,%hi(D) ; sd $ra,0($sp) ; jal func_002A8F70
       addiu $t0,$t0,%lo(D) ; ld $ra,0($sp) ; jr $ra ; addiu $sp,$sp,0x10
   Measured (scratch only, never in src/): a separate local for the address, `&D[0]`, a cast
   through long, the pointer as the fifth argument with the float sixth, an eight-argument
   callee and the SDK compiler all give the same split pair. `-mno-split-addresses` keeps the
   pair together but then moves the `ld $ra` into the jal's delay slot, where retail has the
   `nop`. What would unblock it: a per-file flag that keeps the %hi/%lo pair together without
   filling the delay slot. The table address is per level (0x001B8F20 in level 00, 0x001B8F60 in
   level 01, 0x001A8BD0 in the boot program), so the name carries level 00's. */
extern void func_002A8F70(void *, void *, void *, void *, float, void *);

extern char D_L00_001B8F20[];

void func_002A90C8(void *a0, void *a1, void *a2, void *a3, float f12)
{
    func_002A8F70(a0, a1, a2, a3, f12, D_L00_001B8F20);
}
