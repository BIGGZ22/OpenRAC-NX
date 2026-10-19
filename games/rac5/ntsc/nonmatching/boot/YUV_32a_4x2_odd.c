/* YUV_32a_4x2_odd 0x01EAE078..0x01EAE080 (8 bytes).
 * The retail symbol is a bare `jr $ra ; nop` stub, like the four YUV_*_4x2_*
 * entries that precede it.  The 2872-byte next_label interval attributed to it
 * continues past the stub into the unnamed blitter at 0x01EAE080, which is not
 * this symbol's body.
 */
void YUV_32a_4x2_odd(void)
{
}
