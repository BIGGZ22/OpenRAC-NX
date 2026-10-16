/* Carried from the PAL decompilation's src/overlays/shared/vendor_002B90A8.c (matched C there) (games/rac1) through the version
 * map (../../README.md): functional C, not matched C. Names and addresses are this version's. */
#include "types.h"
#include "qcopy.h"
#include "qzero.h"
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
/* What the PAL file this was written in declares before it (its unit, and its own file). */
extern float func_001FA580(float, float);
extern char D_L01_00167240[];
extern float D_0015ED6C MACRO_ADDR;
extern float func_001F9B48(void *, void *);
extern float func_001F9B80(void *, void *);
extern float func_001F99C0(float);
extern int func_001F9688(int);
extern float func_L00_001FF8B0(float, float);
extern float func_002132A8(float, float);
extern float D_0015ED70 MACRO_ADDR;
extern void func_001F99F8(void *);
extern void func_0020C828(void *);
extern f32 func_001F9DC8(f32);
extern f32 func_001F9DE0(f32);
extern void func_L00_00263AC8(float, void *, int, void *);
extern void func_L00_0025A120(void *);
extern void func_L01_002EEC68(char *);
extern void func_L01_002EF560(unsigned char *);
extern void func_L01_002EF770(unsigned char *, float *);
extern int func_L01_00276FE8(int *, int, int, float, void *, void *, void *);
extern void func_L01_0026E8E0_EF078(void *) __asm__("func_L01_0026D930");
extern void func_L00_00258B50(void *, void *, void *, void *, float);
extern float func_L00_0025BE00(float *, float *, float, float, float, float);
extern int func_L00_0025C698(void *, void *);
extern int func_L00_00261B48(int, void *, void *, void *);
extern void func_L00_0025AC00_EF078(void *, void *, int, void *, void *, float) __asm__("func_L00_00259BC8");
extern int func_L00_002DC9F0(void *, void *);
extern int func_L00_00257470(void *, int, int);
extern int func_00214CC8(float, void *);
extern int func_00214720(void *, int);
extern void func_0022DA68(int, int, char *);
extern void func_00212F90(void *, int, int, int);
extern int func_00213260(int);
extern float func_00213308(void);
extern int func_001FA6D0(float);
extern float func_0020C9E0(void *);
extern float D_0013F420[4];
extern float D_L01_0015F580[] MACRO_ADDR;
extern char D_L01_00161A40;
extern char D_L01_00161A50;
extern short D_L01_001619AC;



/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/overlays/shared/gameplay/entities/002b8c08.c, FUN_L01_002edca0. */
void FUN_L01_002edca0(unsigned char *moby) {
    char *data = *(char **)(moby + 0x78);
    char *target;
    char *target_position;
    float move[4];
    float hit[4];
    float angle;
    float distance;
    int path_hit;
    int blocked;
    int result;

    if (moby[0x20] == 12) {
        *(u16 *)(moby + 0x34) |= 1;
        return;
    }
    func_L00_00263AC8(2.9f, moby, 0, data + 0x270);
    if (moby[0x31] != 0 && func_001F9B48(moby + 0x10, D_L01_00167240) < 28.0f) {
        func_L00_0025A120(moby);
        moby[0x7f] = 0x16;
    }
    func_L01_002EEC68((char *)moby);
    target = *(char **)(data + 0x200);
    switch (moby[0x20]) {
    case 0:
        qcopy(data + 0x210, moby + 0x10);
        switch (*(short *)(moby + 0xa6)) {
        case 0x23c:
            data[0x58] = func_001FA6D0(14.0f);
            data[0x5a] = func_001FA6D0(10.4f);
            *(float *)(data + 0x250) = 1.0f;
            break;
        case 0x361:
            data[0x58] = func_001FA6D0(14.0f);
            data[0x5a] = func_001FA6D0(5.6f);
            *(float *)(data + 0x250) = 0.7f;
            break;
        case 0x362:
            data[0x58] = func_001FA6D0(14.0f);
            data[0x5a] = func_001FA6D0(4.0f);
            *(float *)(data + 0x250) = 0.49f;
            break;
        }
        *(float *)(data + 0x30) = *(float *)(data + 0x250);
        *(float *)(data + 0x254) = func_00213308();
        if (*(short *)(moby + 0xa6) == 0x362)
            *(void **)(data + 0x130) = &D_L01_00161A50;
        else
            *(void **)(data + 0x130) = &D_L01_00161A40;
        data[0x29] = 1;
        if (*(int *)(data + 0x258) != 0) {
            *(float *)(data + 0x20) = 1.0f;
            *(int *)(data + 0x230) = 1;
        } else {
            *(float *)(data + 0x20) = 3.0f;
        }
        if (*(float *)(data + 0x224) == 0.0f)
            *(float *)(data + 0x224) = 15.0f;
        func_L01_0026E8E0_EF078(data + 0x170);
        *(float *)(data + 0x17c) = 2.0f;
        *(float *)(data + 0x178) = 2.0f;
        result = func_001FA6D0(*(float *)(data + 0x250) * 0.8f * 1024.0f);
        angle = *(float *)(data + 0x23c) * D_0015ED6C;
        *(int *)(data + 0x170) = result;
        *(float *)(data + 0x194) = angle;
        if (*(int *)(data + 0x230) != 0) {
            moby[0x20] = 1;
            *(u16 *)(moby + 0x34) |= 0x1000;
        } else {
            if (*(int *)(data + 0x25c) == -1) moby[0x20] = 12;
            else moby[0x20] = 13;
            *(int *)(moby + 0x94) = 0;
            *(int *)(moby + 0x58) = 0;
            moby[0x31] = 0;
            distance = *(float *)(moby + 0x18) - 20.0f;
            *(u16 *)(moby + 0x34) = (*(u16 *)(moby + 0x34) & 0xefff) | 1;
            *(float *)(moby + 0x18) = distance;
        }
        break;
    case 1:
        if (*(int *)(data + 0x204) != 2) {
            if (func_00213260(4) == 0) {
                path_hit = func_L01_00276FE8((int *)(data + 0x240), 1, *(int *)(data + 0x244),
                                            *(float *)(data + 0x250) * 0.8f,
                                            moby + 0x10, target + 0x10, move);
                if (path_hit != 0) {
                    moby[0x20] = 4;
                    if (moby[0x53] != 1)
                        func_00212F90(moby, 1, 0, func_001F9688(10));
                    func_001F99F8(data + 0x40);
                    *(float *)(data + 0x234) = func_002132A8(-0.34906584f, 0.34906584f);
                } else {
                    *(int *)(data + 0x204) = 2;
                }
            }
        }
        if (*(int *)(data + 0x22c) != 0 && *(int *)(data + 0x204) == 2) {
            moby[0x20] = 15;
            if (moby[0x53] != 1)
                func_00212F90(moby, 1, 0, 5);
            return;
        }
        *(float *)(data + 0x194) = *(float *)(data + 0x23c) * 0.5f * D_0015ED6C;
        if (moby[0x53] != 1)
            func_00212F90(moby, 1, 0, 5);
        moby[0x20] = 2;
        break;
    case 2:
        angle = func_L00_001FF8B0(*(float *)(data + 0x210) - *(float *)(moby + 0x10),
                              *(float *)(data + 0x214) - *(float *)(moby + 0x14));
        func_L00_0025BE00((float *)(moby + 0x48), (float *)(data + 0x220), angle,
                          D_0015ED70 * 1.0471976f, D_0015ED70 * 1.0471976f,
                          D_0015ED6C * 3.1415927f);
        move[0] = func_001F9DC8(*(float *)(moby + 0x48));
        move[1] = func_001F9DE0(*(float *)(moby + 0x48));
        move[2] = 0.0f;
        func_L00_00258B50(moby, data + 0x170, move, data + 0x40, 1.0f);
        result = *(int *)(data + 0x204);
        if (result != 2 && func_00213260(4) == 0) {
            path_hit = func_L01_00276FE8((int *)(data + 0x240), 1, *(int *)(data + 0x244),
                                        *(float *)(data + 0x250) * 0.8f,
                                        moby + 0x10, target + 0x10, hit);
            if (path_hit != 0) {
                moby[0x20] = 3;
                if (moby[0x53] != 0)
                    func_00212F90(moby, 0, 0, func_001F9688(10));
                func_001F99F8(data + 0x40);
                angle = func_002132A8(-0.34906584f, 0.34906584f);
                distance = *(float *)(data + 0x23c) * D_0015ED6C;
                *(float *)(data + 0x234) = angle;
                *(float *)(data + 0x194) = distance;
            } else {
                *(int *)(data + 0x204) = 2;
            }
        }
        if (*(int *)(data + 0x22c) != 0 && *(int *)(data + 0x204) == 2) {
            moby[0x20] = 15;
            if (moby[0x53] != 1)
                func_00212F90(moby, 1, 0, 5);
        }
        func_L01_002EF560(moby);
        return;
    case 15:
        blocked = 0;
        angle = func_L00_001FF8B0(*(float *)(target + 0x10) - *(float *)(moby + 0x10),
                              *(float *)(target + 0x14) - *(float *)(moby + 0x14));
        func_L00_0025BE00((float *)(moby + 0x48), (float *)(data + 0x220), angle,
                          D_0015ED70 * 1.0471976f, D_0015ED70 * 1.0471976f,
                          D_0015ED6C * 3.1415927f);
        move[0] = func_001F9DC8(*(float *)(moby + 0x48));
        move[1] = func_001F9DE0(*(float *)(moby + 0x48));
        move[2] = 0.0f;
        func_L00_00258B50(moby, data + 0x170, move, data + 0x40, 1.0f);
        if (func_L00_00261B48(*(int *)(data + 0x240), moby + 0x10, target + 0x10, hit) != 0 &&
            func_001F9B48(moby + 0x10, hit) < 1.5f) blocked = 1;
        if (*(int *)(data + 0x22c) == 0) {
            moby[0x20] = 5;
        } else if (blocked) {
            moby[0x20] = 16;
            if (moby[0x53] != 0)
                func_00212F90(moby, 0, 0, func_001F9688(10));
        } else if (func_001F9B80(moby + 0x10, target + 0x10) < 2.0f) {
            moby[0x20] = 6;
            if (moby[0x53] != 2)
                func_00212F90(moby, 2, 5, func_001F9688(10));
            return;
        }
        break;
    case 16:
        if (!(moby[0x70] & 2) || *(int *)(data + 0x22c) != 0) return;
        moby[0x20] = 5;
        if (moby[0x53] != 1)
            func_00212F90(moby, 1, 0, func_001F9688(20));
        return;
    case 4: {
        char *target_position;
        unsigned char path_hit;
        target_position = target + 0x10;
        distance = func_001F9B80(moby + 0x10, target_position);
        if (distance < (*(float *)&D_L01_001619AC)) {
            moby[0x20] = 6;
            *(float *)(data + 0x194) = (*(float *)(data + 0x23c) + *(float *)(data + 0x23c)) * D_0015ED6C;
            if (moby[0x53] != 2)
                func_00212F90(moby, 2, 0, func_001F9688(10));
            return;
        } else {
            if (func_L01_00276FE8((int *)(data + 0x240), 1, *(int *)(data + 0x244),
                                        *(float *)(data + 0x250) * 0.8f, moby + 0x10, target_position, hit) != 0)
                path_hit = 1;
            else
                path_hit = func_L01_00276FE8((int *)(data + 0x240), 1, *(int *)(data + 0x244),
                                            0.0f, moby + 0x10, target_position, hit) != 0;
            if (path_hit != 0) {
                angle = func_L00_001FF8B0(hit[0] - *(float *)(moby + 0x10), hit[1] - *(float *)(moby + 0x14));
                angle = func_001FA580(angle, *(float *)(data + 0x234));
                func_L00_0025BE00((float *)(moby + 0x48), (float *)(data + 0x220), angle,
                                  D_0015ED70 * 12.566371f, D_0015ED70 * 12.566371f,
                                  D_0015ED6C * 25.132742f);
                move[0] = func_001F9DC8(*(float *)(moby + 0x48));
                move[0] += move[0];
                move[1] = func_001F9DE0(*(float *)(moby + 0x48));
                move[1] += move[1];
                move[2] = 0.0f;
                func_L00_00258B50(moby, data + 0x170, move, data + 0x40, 1.0f);
            } else {
                *(int *)(data + 0x204) = 2;
            }
            if (*(int *)(data + 0x204) == 2) moby[0x20] = 5;
            func_L01_002EF560(moby);
            return;
        }
    }
    case 6: {
        char *target_position;
        float *rotation = (float *)(moby + 0x48);
        angle = func_L00_001FF8B0(*(float *)(target + 0x10) - *(float *)(moby + 0x10),
                              *(float *)(target + 0x14) - *(float *)(moby + 0x14));
        func_L00_0025BE00(rotation, (float *)(data + 0x220), angle,
                          D_0015ED70 * 12.566371f, D_0015ED70 * 12.566371f,
                          D_0015ED6C * 25.132742f);
        distance = func_0020C9E0(moby);
        if (moby[0x52] == moby[0x53] && 10.0f <= distance && distance <= 16.0f) {
            move[0] = func_001F9DC8(*rotation);
            move[0] += move[0];
            move[1] = func_001F9DE0(*rotation);
            move[1] += move[1];
            move[2] = 0.0f;
            func_L00_00258B50(moby, data + 0x170, move, data + 0x40, 1.0f);
        }
        if (func_00214CC8(15.5f, moby) != 0 && func_001F9B80(moby + 0x10, (target_position = target + 0x10)) < 2.0f &&
            func_001F99C0(*(float *)(target + 0x18) - *(float *)(moby + 0x18)) <
                *(float *)(data + 0x250) + *(float *)(data + 0x250)) {
            move[0] = func_001F9DC8(*(float *)(moby + 0x48));
            move[1] = func_001F9DE0(*(float *)(moby + 0x48));
            move[2] = 0.0f;
            move[3] = 5627.9248f;
            qcopy(hit, target_position);
            hit[2] += 0.75f;
            func_L00_0025AC00_EF078(target, moby, 0x10001, hit, move, 1.0f);
            return;
        }
        if (moby[0x70] & 2) {
            moby[0x20] = 7;
            if (moby[0x53] != 3)
                func_00212F90(moby, 3, 0, func_001F9688(10));
            *(float *)(data + 0x194) = *(float *)(data + 0x23c) * D_0015ED6C;
        }
        break;
    }
    case 3:
    case 7:
        if (!(moby[0x70] & 2)) return;
        moby[0x20] = 4;
        if (moby[0x53] != 1)
            func_00212F90(moby, 1, 0, func_001F9688(10));
        return;
    case 5: {
        int blocked = 0;
        char *target_position;
        distance = func_001F9B80(moby + 0x10, (target_position = data + 0x210));
        if (distance < 1.0f) {
            moby[0x20] = 1;
        } else {
            if (func_L01_00276FE8((int *)(data + 0x240), 1, *(int *)(data + 0x244),
                                        *(float *)(data + 0x250) * 0.8f, moby + 0x10, target_position, hit) != 0)
                blocked = 1;
            else
                blocked = func_L01_00276FE8((int *)(data + 0x240), 1, *(int *)(data + 0x244),
                                            0.0f, target_position, target_position, hit) != 0;
        }
        if (blocked != 0) {
            angle = func_L00_001FF8B0(hit[0] - *(float *)(moby + 0x10), hit[1] - *(float *)(moby + 0x14));
            angle = func_001FA580(angle, *(float *)(data + 0x234));
            func_L00_0025BE00((float *)(moby + 0x48), (float *)(data + 0x220), angle,
                              D_0015ED70 * 12.566371f, D_0015ED70 * 12.566371f,
                              D_0015ED6C * 25.132742f);
            move[0] = func_001F9DC8(*(float *)(moby + 0x48));
            move[0] += move[0];
            move[1] = func_001F9DE0(*(float *)(moby + 0x48));
            move[1] += move[1];
            move[2] = 0.0f;
            func_L00_00258B50(moby, data + 0x170, move, data + 0x40, 1.0f);
        }
        if (*(int *)(data + 0x204) != 2 && (moby[0x20] = 4, moby[0x53] != 1))
            func_00212F90(moby, 1, 0, func_001F9688(3));
        func_L01_002EF560(moby);
        return;
    }
    case 8:
        result = func_L00_0025C698(moby, data + 0x60);
        if (result & 1) {
            path_hit = func_L01_00276FE8((int *)(data + 0x240), 1, *(int *)(data + 0x244),
                                        *(float *)(data + 0x250) * 0.8f,
                                        moby + 0x10, data + 0x210, move);
            if (path_hit != 0) {
                func_0022DA68(4, 0, (char *)moby);
                *(float *)(data + 0x194) = *(float *)(data + 0x23c) * D_0015ED6C;
                moby[0x20] = 9;
                if (moby[0x53] != 5)
                    func_00212F90(moby, 5, 4, func_001F9688(10));
                qcopy(data + 0x210, moby + 0x10);
                return;
            }
            func_L01_002EF770(moby, D_L01_0015F580);
            func_L00_00257470(moby, 0, -1);
        } else if (!(result & 0x120)) {
            return;
        }
        func_0020C828(moby);
        return;
    case 9:
        if (moby[0x70] & 2) {
            moby[0x20] = 4;
            if (moby[0x53] != 1)
                func_00212F90(moby, 1, 0, func_001F9688(10));
            func_001F99F8(data + 0x40);
            angle = func_002132A8(-0.34906584f, 0.34906584f);
            distance = *(float *)(data + 0x23c) * D_0015ED6C;
            *(float *)(data + 0x234) = angle;
            *(float *)(data + 0x194) = distance;
        }
        break;
    case 13:
        *(u16 *)(moby + 0x34) |= 1;
        if (func_00214720(D_0013F420, *(int *)(data + 0x25c)) != 0) {
            u16 flags;
            float distance;
            int result;
            distance = *(float *)(moby + 0x18) + 20.0f;
            flags = *(u16 *)(moby + 0x34) | 0x1000;
            moby[0x20] = 1;
            *(u16 *)(moby + 0x34) = flags;
            *(float *)(moby + 0x58) = 1.0f;
            *(float *)(moby + 0x18) = distance;
            result = *(int *)(*(char **)(moby + 0x24) + 0x10);
            *(int *)(moby + 0x94) = result;
            *(u16 *)(moby + 0x34) = flags & 0xfffe;
            moby[0x31] = 1;
        }
        return;
    case 14:
        if (func_L00_002DC9F0(moby, data + 0x60)) {
            moby[0x20] = 1;
            *(u16 *)(data + 0x128) = 0;
        }
        break;
    }
}
