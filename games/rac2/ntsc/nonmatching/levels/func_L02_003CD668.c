/* func_L02_003CD668 (240 bytes, 1 place) - two MMI words, ASM_FUNC candidate unless they
 * turn out to be lq/sq.
 *
 * Classified by pattern, not decoded this wave: two `.word # INVALID` entries, no
 * max.s/min.s class word, five calls.  Given the id-list family it sits in (the same
 * L02_002ED7B0.c unit as func_L02_003CD5B0 and func_L02_003D18D8, both deposited with the
 * list-walk recipe), the two INVALID words are the thing to look at first: if they are
 * lq/sq pairs rather than real MMI, the recipe from 01_oozla/L01_003CAFE0-g8.c applies.
 */
