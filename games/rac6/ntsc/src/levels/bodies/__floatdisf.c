/* __floatdisf -- LEVEL19.REL 0x001CAD10, 224 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Routine de bibliotheque du compilateur EE-GCC 2.9 (libgcc2.c) :
 *   conversion d'un entier 64 bits en flottant.  Le R5900 n'a pas de double
 *   materiel : toute l'arithmetique double passe par litodp/dpmul/dpadd.
 *   Le resultat est rendu en v0/v1 (double), comme au retail.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (sdk991111-sdkas:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef float SFtype __attribute__ ((mode (SF)));
typedef float DFtype __attribute__ ((mode (DF)));
typedef int SItype __attribute__ ((mode (SI)));
typedef unsigned int USItype __attribute__ ((mode (SI)));
typedef int DItype __attribute__ ((mode (DI)));
typedef unsigned int UDItype __attribute__ ((mode (DI)));

/* BITS_PER_UNIT vient des en-tetes du SDK (octets), pas de la macro
 * interne de GCC : WORD_SIZE = 4 * 8 = 32 et DI_SIZE = 8 * 8 = 64, ce que
 * confirme le retail (HIGH_HALFWORD_COEFF = 2^16, HIGH_WORD_COEFF = 2^32,
 * REP_BIT = 2^11). */
#define BITS_PER_UNIT 8
#define WORD_SIZE (sizeof (SItype) * BITS_PER_UNIT)
#define HIGH_HALFWORD_COEFF (((UDItype) 1) << (WORD_SIZE / 2))
#define HIGH_WORD_COEFF (((UDItype) 1) << WORD_SIZE)
#define DI_SIZE (sizeof (DItype) * BITS_PER_UNIT)

#define DF_SIZE 53
#define SF_SIZE 24

#define REP_BIT ((USItype) 1 << (DI_SIZE - DF_SIZE))

SFtype
__floatdisf (DItype u)
{
  DFtype f;

  if (DF_SIZE < DI_SIZE
      && DF_SIZE > (DI_SIZE - DF_SIZE + SF_SIZE))
    {
      if (! (- ((DItype) 1 << DF_SIZE) < u
	     && u < ((DItype) 1 << DF_SIZE)))
	{
	  if ((USItype) u & (REP_BIT - 1))
	    u |= REP_BIT;
	}
    }
  f = (SItype) (u >> WORD_SIZE);
  f *= HIGH_HALFWORD_COEFF;
  f *= HIGH_HALFWORD_COEFF;
  f += (USItype) (u & (HIGH_WORD_COEFF - 1));

  return (SFtype) f;
}
