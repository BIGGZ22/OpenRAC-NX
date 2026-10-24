/* COL_IntersectCapsuleWithMesh__FPfT0P4VEC3PC4VEC3T3ffPC13CollisionMeshPC7MATRIX4T2_LEVEL19_0019DB00 -- LEVEL19.REL 0x0019DB00, 88 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Enveloppe publique de COL_IntersectCapsuleWithMeshInternal : elle
 *   prepare un bloc local de 16 octets, décale les arguments d'un cran
 *   (le premier parametre du wrapper devient le second de l'interne) et
 *   rend le résultat de l'appel sous forme de booleen.
 *   Les deux derniers arguments registres du wrapper (les "ff" du nom
 *   retail) sont transmis en recopie brute dans les registres t2/t3 de
 *   l'interne : ce sont donc des valeurs opaques de 32 bits, declarees
 *   ici comme telles (le compilateur EE-GCC 2.9 ne sait pas placer un
 *   flottant au-dela des quatre premiers registres et l'eliminerait).
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct
{
  float v[4];
} VEC4;

typedef struct
{
  float x, y, z;
} VEC3;

typedef struct CollisionMesh CollisionMesh;
typedef struct MATRIX4 MATRIX4;

extern int func_19D8C0 (VEC4 *, float *, float *, VEC3 *, const VEC3 *,
			const VEC3 *, unsigned int, unsigned int,
			const CollisionMesh *);

int
COL_IntersectCapsuleWithMesh__FPfT0P4VEC3PC4VEC3T3ffPC13CollisionMeshPC7MATRIX4T2
    (float *r0, float *r1, VEC3 *v0, const VEC3 *v1, const VEC3 *v2,
     unsigned int t0, unsigned int t1, const CollisionMesh *mesh,
     const MATRIX4 *mat, VEC3 *out)
{
  VEC4 tmp;

  (void) mat;
  (void) out;

  return func_19D8C0 (&tmp, r0, r1, v0, v1, v2, t0, t1, mesh) != 0;
}
