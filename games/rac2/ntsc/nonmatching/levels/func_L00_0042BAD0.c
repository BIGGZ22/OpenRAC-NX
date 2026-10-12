/* famille de quatre corps identiques : func_L00_0042BAD0 / _BCF8 / _BFF8 / _C290
   dernier check.py : "00_aranos_tutorial: instruction at +0x18 differs"
   Seul ecart : cc1 hisse ld $ra,0x8($sp) avant le magasin et place daddu $v0,$s0,$zero
   entre lui et addiu ; retail garde lui / addiu / daddu / sw / ld $s0 / ld $ra.
   Variantes essayees et refusees : char *self = a0; idem. Seule la constante differe
   entre les quatre (0x001ADD98 / 0x001ADD38 / 0x001ADD78 / 0x001ADD58). */
void *func_L00_0042BAD0(char *a0)
{
    func_L00_0042B8D8(a0);
    *(char **)(a0 + 0x30) = D_L00_001ADD98;
    return a0;
}
