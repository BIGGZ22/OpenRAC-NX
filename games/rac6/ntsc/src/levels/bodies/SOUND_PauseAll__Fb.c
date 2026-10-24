/* SOUND_PauseAll__Fb -- FE.REL 0x011838, 128 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   ...                              (restauration puis jr $ra)
 *   Le systeme FMOD expose 64 canaux ; la fonction met en pause les 64 canaux
 *   existants puis memorise le drapeau global.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct FMOD_System FMOD_System;
typedef struct FMOD_Channel FMOD_Channel;

extern FMOD_System *SOUND_GetSystem__Fv(void);
extern void getChannel__Q24FMOD6SystemiPPQ24FMOD7Channel(FMOD_System *system,
                                                        int index,
                                                        FMOD_Channel **channel);
extern int setPaused__Q24FMOD7Channelb(FMOD_Channel *channel, int paused);

extern char g_sound_pause_all;

void SOUND_PauseAll__Fb(int pause)
{
    FMOD_System *system = SOUND_GetSystem__Fv();
    int index;

    if (system) {
        for (index = 0; index < 64; index++) {
            FMOD_Channel *channel;

            getChannel__Q24FMOD6SystemiPPQ24FMOD7Channel(system, index, &channel);
            setPaused__Q24FMOD7Channelb(channel, pause);
        }
    }

    g_sound_pause_all = (char)pause;
}
