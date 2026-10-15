/* functional: medium. VU0 vadd.xyz on two 16-byte vectors: dst.xyz += src.xyz, dst.w unchanged, dst written back. Plain float adds stand in for the VU pipeline, so rounding on denormals may differ; the first argument is unused as in the assembly. */
void func_L00_001FF240(int unused, float *dst, float *src)
{
    dst[0] = dst[0] + src[0];
    dst[1] = dst[1] + src[1];
    dst[2] = dst[2] + src[2];
}
