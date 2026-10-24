/* LEVEL15_000676A0 -- LEVEL15.REL 0x000676A0, 76 bytes.
 *
 * Placement: module-relative offset of the disc module; the
 * other placements of the same body are listed in
 * config/levels/bodies.tsv.
 * Proof: the pinned chain compiles this source to the retail
 * bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 */


typedef struct G5Plane {
    long long x;
    long long y;
} __attribute__((packed)) G5Plane;

typedef struct G6Sphere {
    long long x;
    long long y;
} __attribute__((packed)) G6Sphere;

void TIE_DrawTiesForMirror__FG5PlaneG6SphereUl(G5Plane *plane,
                                                G6Sphere *sphere,
                                                unsigned long flags)
{
    G5Plane localPlane = *plane;
    G6Sphere localSphere = *sphere;

    (void)localPlane;
    (void)localSphere;
    (void)flags;
}
