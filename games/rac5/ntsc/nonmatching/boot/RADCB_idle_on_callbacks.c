/* RADCB_idle_on_callbacks 0x01EAB608..0x01EAB610 (8 bytes).
 * The retail symbol is a bare `jr $ra ; nop` stub: the entry takes no
 * arguments and does nothing.  The 5384-byte next_label interval that the
 * boot interval table attributes to it continues past the stub into unnamed
 * code at 0x01EAB610 (that block has no symbol and is not this target).
 */
void RADCB_idle_on_callbacks(void)
{
}
