/* FastVecDot: the dot product of the xyz parts of two vectors, as VU0 does it:
   (a.x*b.x + a.y*b.y), then + a.z*b.z (the z term times 1 from vf3). */
float func_001F9C78(void *a, void *b)
{
    float *pa = (float *)a;
    float *pb = (float *)b;
    float xy = pa[0] * pb[0] + pa[1] * pb[1];
    return xy + pa[2] * pb[2];
}
