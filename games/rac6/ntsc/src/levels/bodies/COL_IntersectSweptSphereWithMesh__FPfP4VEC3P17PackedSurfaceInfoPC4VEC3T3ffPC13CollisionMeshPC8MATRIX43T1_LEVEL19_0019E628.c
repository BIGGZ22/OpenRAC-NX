/* COL_IntersectSweptSphereWithMesh__FPfP4VEC3P17PackedSurfaceInfoPC4VEC3T3ffPC13CollisionMeshPC8MATRIX43T1_LEVEL19_0019E628 -- LEVEL19.REL 0x0019E628, 88 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Meme enveloppe que 0x0019E2C0, mais la variante "MATRIX43" de
 *   l'interne (func_19E400).
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

typedef struct PackedSurfaceInfo PackedSurfaceInfo;
typedef struct CollisionMesh CollisionMesh;
typedef struct MATRIX43 MATRIX43;

extern int func_19E400 (VEC4 *, float *, VEC3 *, PackedSurfaceInfo *,
			const VEC3 *, const VEC3 *, unsigned int, unsigned int,
			const CollisionMesh *);

int
COL_IntersectSweptSphereWithMesh__FPfP4VEC3P17PackedSurfaceInfoPC4VEC3T3ffPC13CollisionMeshPC8MATRIX43T1
    (float *r0, VEC3 *v0, PackedSurfaceInfo *si, const VEC3 *v1,
     const VEC3 *v2, unsigned int t0, unsigned int t1,
     const CollisionMesh *mesh, const MATRIX43 *mat, VEC3 *out)
{
  VEC4 tmp;

  (void) mat;
  (void) out;

  return func_19E400 (&tmp, r0, v0, si, v1, v2, t0, t1, mesh) != 0;
}
