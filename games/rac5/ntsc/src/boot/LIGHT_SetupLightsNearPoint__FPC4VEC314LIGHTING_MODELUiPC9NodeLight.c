/* Thin forwarder that appends the currently registered static light arrays. */
extern unsigned int g_nStaticDirectionalLights;                /* 0x01EDD9B0 */
extern const void *g_StaticDirectionalLights;                  /* 0x01EDD9B4 */
extern unsigned int g_nStaticPointLights;                      /* 0x01EDD9B8 */
extern const void *g_StaticPointLights;                        /* 0x01EDD9BC */

extern void
LIGHT_SetupLightsNearPoint__FPC4VEC314LIGHTING_MODELUiPC9NodeLightUiPC16DirectionalLightUiPC10PointLight(
    const void *position, int model, unsigned int count, const void *node_light,
    unsigned int n_directional, const void *directional,
    unsigned int n_point, const void *point);

void LIGHT_SetupLightsNearPoint__FPC4VEC314LIGHTING_MODELUiPC9NodeLight(
    const void *position, int model, unsigned int count, const void *node_light) {
    LIGHT_SetupLightsNearPoint__FPC4VEC314LIGHTING_MODELUiPC9NodeLightUiPC16DirectionalLightUiPC10PointLight(
        position, model, count, node_light, g_nStaticDirectionalLights,
        g_StaticDirectionalLights, g_nStaticPointLights, g_StaticPointLights);
}
