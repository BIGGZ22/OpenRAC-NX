/* from src/core/0011BF18.c: a 144-byte 64-bit integer/double helper. It takes the value
   in $a0 (copied to $a1), takes the exponent field with `dsll $v0,$a1,1; dsrl32 $a2,$v0,21`,
   subtracts 0x433, returns 0 below -53 and 9999 at or above 13 - a saturating fixed-point
   conversion. Below the range it rebuilds the mantissa (clearing the top 12 bits, setting
   bit 52) and shifts, with a round-to-nearest step through `bnel` and `movn`. No C attempt:
   the first instruction is a copy of the argument register that no plain-C body produces,
   which suggests the original was built from a 32-bit-argument prototype this catalogue row
   does not record; that is where a next attempt should start. */
