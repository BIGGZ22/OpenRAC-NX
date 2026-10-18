/* sceGsGetGParam 0x01E9C108..0x01E9C118 */
extern int sceGsGParamArea;
void *sceGsGetGParam(void) {
    return &sceGsGParamArea;
}
