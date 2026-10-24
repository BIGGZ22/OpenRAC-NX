void AdvanceCardMenu(void) {
 int old=D_0015EFB4;
 int intermediate=old&~4;
 int flags=intermediate&~2;
 D_0015EFB4=flags;
 if(memory_card_state[0xF4/4]==0) D_0015EFB0=3;
 else if(old&0x80) { D_0015EFB0=21; D_0015EFB4=(flags^0x80)|0x40; }
 else if(old&0x100) { D_0015EFB0=20; D_0015EFB4=(flags^0x100)|0x40; }
 else if(memory_card_state[7]!=0) { memory_card_state[0xF4/4]=0; D_0015EFB4=flags|1; D_0015EFB0=2; }
 else if(old&0x200) { D_0015EFB4=flags^0x200; D_0015EFB0=22; }
}
