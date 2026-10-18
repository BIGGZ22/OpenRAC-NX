extern void *gENG_InstanceResources[256]; /* 0x01F78DE0 */

void *GDEMESH_GetMesh__FUi(unsigned int index) {
    if (index < 256) return gENG_InstanceResources[index];
    return 0;
}
