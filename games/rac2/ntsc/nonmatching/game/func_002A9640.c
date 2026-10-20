/* func_002A9640 - near miss (wave 11, agent G4).

   The 64-byte slot table at 0x001BF200 (the boot's copy; every level has its own at
   another address): slot +0x24 is the value masked with the second argument, slot
   +0x38 the object whose byte 0xA8 belongs to.  Retail:

     lbu  $v0,0xA8($a0)
     beq  $v0,$t0,exitA          ; index == 255
     daddu $v1,$v0,$zero         ; delay slot: a copy of the index into $v1
     lui  $v0,%hi(D_001BF200)
     sll  $v1,$v1,6
     addiu $v0,$v0,%lo(D_001BF200)
     addu  $v1,$v1,$v0
     lw   $a3,0x38($v1)
     beql $a3,$a0,body           ; the object matches
     lw   $v0,0x24($v1)          ; delay slot: the load is conditional
     exitA: jr $ra ; daddu $v0,$zero,$zero
     body: and $v0,$v0,$a1
     bnez $v0,exitB
     daddu $v0,$v1,$zero         ; delay slot: the returned slot pointer
     beql $a2,$zero,exitC
     sb   $t0,0xA8($a3)
     exitC: jr $ra ; daddu $v0,$zero,$zero
     exitB: jr $ra ; nop

   What is left: retail loads the index byte once into $v0 and copies it into $v1
   for the table address, while every spelling measured here keeps it in one
   register ($v1) and scales it in place - the copy only appears when the byte is
   read twice, which then costs a second load.  And retail keeps the "return 0"
   after the store in a block of its own, while this cc1 cross-jumps it into the
   shared exit and needs one word more (92 against 88 bytes).

   tools/check.py func_002A9640: 0 of 28 places match (src/game/002A7750.c). */

typedef unsigned char u8;

typedef struct {
    unsigned char pad0[0x24];
    int value;
    unsigned char pad1[0x10];
    void *object;
    unsigned char pad2[0x04];
} Slot_2A9640;

typedef struct {
    unsigned char pad0[0xA8];
    unsigned char flag;
} Obj_2A9640;

extern Slot_2A9640 D_001BF200[];

Slot_2A9640 *func_002A9640(void *a0, int a1, void *a2) {
    int index = *(unsigned char *)((char *)a0 + 0xA8);
    Slot_2A9640 *slot;
    if (index != 255) {
        slot = &D_001BF200[index];
        if (slot->object != a0)
            return 0;
        if (slot->value & a1)
            return slot;
        if (a2 == 0)
            ((Obj_2A9640 *)slot->object)->flag = 255;
        return 0;
    }
    return 0;
}
