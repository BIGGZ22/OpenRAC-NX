/* Quaternion product out = A * B, with A = (A.xyz, A.w) and B = (B.xyz, B.w):
   xyz = A.w*B.xyz + B.w*A.xyz + A x B, w = A.w*B.w - dot(A.xyz, B.xyz). */
void func_001FA588(void *out, void *a, void *b)
{
    float *o = (float *)out;
    float *A = (float *)a;
    float *B = (float *)b;
    float dot;
    float cx, cy, cz;

    dot = (B[0] * A[0] + B[1] * A[1]) + B[2] * A[2];
    cx = A[1] * B[2] - A[2] * B[1];
    cy = A[2] * B[0] - A[0] * B[2];
    cz = A[0] * B[1] - A[1] * B[0];

    o[0] = (B[0] * A[3] + A[0] * B[3]) + cx;
    o[1] = (B[1] * A[3] + A[1] * B[3]) + cy;
    o[2] = (B[2] * A[3] + A[2] * B[3]) + cz;
    o[3] = B[3] * A[3] - dot;
}
