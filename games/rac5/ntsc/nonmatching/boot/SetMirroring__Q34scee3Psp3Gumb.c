/* scee::Psp3Gu::SetMirroring(bool) -> gu state byte at 0x01F77900. */
extern unsigned char gu_state_01F77900; /* 0x01F77900 */

void SetMirroring__Q34scee3Psp3Gumb(unsigned char value) {
    gu_state_01F77900 = value;
}
