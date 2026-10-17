/* functional: medium. Corrected in place: a zero b vector gave 1/sqrtf(0) = inf and then NaN; retail's vrsqrt gives a finite Q there, so n = b*Q = 0 and the result is a. Otherwise reflects a about the plane with normal b: n = b/|b|, t = -(a.n); when t < 0 dst is a copied whole, otherwise dst.xyz = a.xyz + 2*t*n and dst.w = a.w (joined with func_L00_001FF670, the copy). VU0 vrsqrt is approximate; this uses an exact sqrt. */
void func_L00_001FF610(float *dst, float *a, float *b) {
    float len2 = b[0] * b[0] + b[1] * b[1] + b[2] * b[2];
    float inv = len2 > 0.0f ? 1.0f / __builtin_sqrtf(len2) : 0.0f;
    float nx = b[0] * inv;
    float ny = b[1] * inv;
    float nz = b[2] * inv;
    float t = -(a[0] * nx + a[1] * ny + a[2] * nz);
    if (t < 0) {
        dst[0] = a[0];
        dst[1] = a[1];
        dst[2] = a[2];
        dst[3] = a[3];
        return;
    }
    dst[0] = a[0] + 2 * t * nx;
    dst[1] = a[1] + 2 * t * ny;
    dst[2] = a[2] + 2 * t * nz;
    dst[3] = a[3];
}
