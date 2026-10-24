void memcard_update_state(void) {
    union {
        char name[0x40];
        struct {
            s32 seen[5];
            char name[0x40] __attribute__((aligned(16)));
        } directory;
    } workspace;
    McState_209E68 *finished_card;

    if (mc_data_0013D390.busy) {
        mc_data_0013D390.busy = mc_call_00123F30(1, &mc_data_0013D390.cmd, &mc_data_0013D390.result) == 0;
        return;
    }
    mc_data_0013D390.busy = 1;

    switch (mc_data_0013D390.state) {
    case 0:
        if (mc_data_0013D390.cur < 0) {
            mc_data_0013D390.cur = 0;
        }
        if (mc_call_00124068(mc_data_0013D390.card[mc_data_0013D390.cur].port, mc_data_0013D390.card[mc_data_0013D390.cur].slot, &mc_data_0013D390.card[mc_data_0013D390.cur].type, &mc_data_0013D390.card[mc_data_0013D390.cur].free, &mc_data_0013D390.card[mc_data_0013D390.cur].format) == 0) {
            mc_data_0013D390.state = 1;
        }
        break;

    case 1:
        if (mc_data_0013D390.result != 0) {
            mc_data_0013D390.card[mc_data_0013D390.cur].x14 = -4;
            mc_data_0013D390.card[mc_data_0013D390.cur].x1C = mc_data_0013D390.result;
            mc_data_0013D390.card[mc_data_0013D390.cur].xAC = -1;
            mc_data_0013D390.card[mc_data_0013D390.cur].used_blocks = 0;
            if (mc_data_0013D390.card[mc_data_0013D390.cur].request == 1) {
                if (mc_data_0013D390.result == -1) mc_data_0013D390.card[mc_data_0013D390.cur].request = 2;
                else mc_data_0013D390.card[mc_data_0013D390.cur].request = 0;
            }
        } else {
            if (mc_data_0013D390.card[mc_data_0013D390.cur].request == 1) mc_data_0013D390.card[mc_data_0013D390.cur].request = 0;
        }
        if (++mc_data_0013D390.cur < 1) {
            mc_data_0013D390.state = 0;
        } else {
            mc_data_0013D390.state = 2;
        }
        mc_data_0013D390.busy = 0;
        break;

    case 2:
        if (mc_data_0013D390.xDC >= 0 && mc_data_0013D390.card[0].request != 3) {
            if (mc_data_0013D390.card[0].x1C == 0) {
                if (mc_data_0013D390.xE0 >= 0) {
                    mc_data_0013D390.cur = mc_data_0013D390.xE0;
                    mc_data_0013D390.state = mc_data_0013D390.xDC;
                    mc_data_0013D390.err = 0;
                } else {
                    mc_data_0013D390.err = 0x271A;
                }
            } else if (mc_data_0013D390.xDC == 7) {
                mc_data_0013D390.card[0].request = 0;
            }
            mc_data_0013D390.xDC = -1;
            mc_data_0013D390.xE0 = -1;
        } else {
            mc_data_0013D390.cur++;
            if (mc_data_0013D390.card[0].request == 3) mc_data_0013D390.card[0].request = 1;
            if (mc_data_0013D390.cur >= 11 || mc_data_0013D390.card[0].request == 1) {
                mc_data_0013D390.cur = 0;
                mc_data_0013D390.state = 0;
            }
        }
        mc_data_0013D390.busy = 0;
        break;

    case 3:
        if (mc_data_0013D390.card[mc_data_0013D390.cur].format == 0) {
            if (mc_call_00124340(mc_data_0013D390.card[mc_data_0013D390.cur].port, mc_data_0013D390.card[mc_data_0013D390.cur].slot) == 0) {
                mc_data_0013D390.state = 4;
            }
            break;
        }
        mc_data_0013D390.busy = 0;
        mc_data_0013D390.state = 2;
        break;

    case 4:
        if (mc_data_0013D390.result != 0) {
            mc_data_0013D390.err = 1;
            mc_data_0013D390.errCard = mc_data_0013D390.cur;
        }
        mc_data_0013D390.state = 0;
        mc_data_0013D390.cur = 0;
        mc_data_0013D390.busy = 0;
        break;

    case 5:
        if (mc_data_0013D390.card[mc_data_0013D390.cur].format == 0) {
            if (mc_call_00124528(mc_data_0013D390.card[mc_data_0013D390.cur].port, mc_data_0013D390.card[mc_data_0013D390.cur].slot) == 0) {
                mc_data_0013D390.state = 6;
            }
            break;
        }
        mc_data_0013D390.busy = 0;
        mc_data_0013D390.state = 2;
        break;

    case 6:
        if (mc_data_0013D390.result != 0) {
            mc_data_0013D390.err = 2;
            mc_data_0013D390.errCard = mc_data_0013D390.cur;
        }
        mc_data_0013D390.state = 0;
        mc_data_0013D390.cur = 0;
        mc_data_0013D390.busy = 0;
        break;

    case 7:
        if (mc_call_001241F0(mc_data_0013D390.card[mc_data_0013D390.cur].port, mc_data_0013D390.card[mc_data_0013D390.cur].slot, mc_data_0013D2E8, 0, 11, mc_data_001A0980) == 0) {
            mc_data_0013D390.sub = 0;
            mc_data_0013D390.state = 8;
        }
        break;

    case 8:
        switch (mc_data_0013D390.sub) {
        case 0:
            if (mc_data_0013D390.result > 0) {
                s32 icon0;
                s32 icon1;
                s32 icon2;
                s32 i;
                s32 blocks;
                icon0 = 0;
                mc_memset(workspace.directory.seen, 0, 20);
                icon1 = 0;
                icon2 = 0;
                blocks = 0;
                for (i = 0; i < mc_data_0013D390.result; i++) {
                    McDirEntry_209E68 *entry = &mc_directory[i];
                    char *filename = (char *)((s32)mc_directory + i * 64 + 32);
                    s32 slot;
                    blocks += (entry->size + 1023U) >> 10;
                    if (mc_strcmp(filename, D_0013D315) == 0) icon0 = 1;
                    if (mc_strcmp(filename, D_0013D335) == 0) icon1 = 1;
                    if (mc_strcmp(filename, D_0013D355) == 0) icon2 = 1;
                    if (mc_strncmp(filename, D_0015FF58, 4) == 0 &&
                        mc_strncmp(filename + 5, D_0015FF60, 4) == 0) {
                        slot = (u8)entry->name[4] - '0';
                        if (slot < 0) continue;
                        if (slot < 5) workspace.directory.seen[slot] = 1;
                    }
                }
                blocks += (mc_data_0013D390.result + 2) / 2;
                if ((u32)(blocks - 351) < 2 && mc_data_0013D390.result == 10 && icon0 && icon1 && icon2 &&
                    workspace.directory.seen[0] && workspace.directory.seen[1] && workspace.directory.seen[2] && workspace.directory.seen[3] && workspace.directory.seen[4]) {
                    mc_data_0013D390.card[mc_data_0013D390.cur].used_blocks = 0;
                    mc_data_0013D390.card[mc_data_0013D390.cur].x14 = -1;
                    mc_data_0013D390.sub = 1;
                    mc_data_0013D390.busy = 0;
                } else {
                    mc_data_0013D390.card[mc_data_0013D390.cur].used_blocks = blocks;
                    mc_data_0013D390.card[mc_data_0013D390.cur].x14 = -2;
                    goto reset_request;
                }
            } else {
                mc_data_0013D390.card[mc_data_0013D390.cur].x14 = -2;
                if (mc_data_0013D390.result == -2) {
                    mc_data_0013D390.err = 3;
                    mc_data_0013D390.errCard = mc_data_0013D390.cur;
                } else if (mc_data_0013D390.result >= 0) {
                    mc_data_0013D390.card[mc_data_0013D390.cur].x14 = -2;
                } else if (mc_data_0013D390.result != -4) {
                    mc_data_0013D390.err = 4;
                    mc_data_0013D390.errCard = mc_data_0013D390.cur;
                }
                mc_data_0013D390.state = 0;
                mc_data_0013D390.cur = 0;
                mc_data_0013D390.busy = 0;
                if (mc_data_0013D390.card[0].request == 2) mc_data_0013D390.card[0].request = 0;
            }
            break;
        case 1:
            mc_call_00116248(workspace.directory.name, mc_data_0013D370, 0);
            if (mc_call_001238B0(mc_data_0013D390.card[mc_data_0013D390.cur].port, mc_data_0013D390.card[mc_data_0013D390.cur].slot, workspace.directory.name, 1) == 0) {
                mc_data_0013D390.sub++;
            }
            break;
        case 2:
            if (mc_data_0013D390.result >= 0) {
                mc_data_0013D390.fd = mc_data_0013D390.result;
                mc_data_0013D390.sub = 3;
            } else {
                mc_data_0013D390.card[mc_data_0013D390.cur].x14 = -2;
                mc_data_0013D390.errCard = mc_data_0013D390.cur;
                if (mc_data_0013D390.result == -7) {
                    mc_data_0013D390.err = 0x2710;
                } else if (mc_data_0013D390.result == -5) {
                    mc_data_0013D390.err = 0x2711;
                } else if (mc_data_0013D390.result == -4) {
                    mc_data_0013D390.err = 0x2712;
                } else if (mc_data_0013D390.result == -3) {
                    mc_data_0013D390.err = 0x2713;
                } else if (mc_data_0013D390.result == -2) {
                    mc_data_0013D390.err = 3;
                } else {
                    mc_data_0013D390.err = 4;
                }
                mc_data_0013D390.state = 0;
                mc_data_0013D390.cur = 0;
                if (mc_data_0013D390.card[0].request == 2) mc_data_0013D390.card[0].request = 0;
            }
            mc_data_0013D390.busy = 0;
            break;
        case 3:
            mc_data_0013D390.size = 8;
            if (mc_call_00123C30(mc_data_0013D390.fd, &mc_data_0013D390.card[mc_data_0013D390.cur].xB0, 8) == 0) {
                mc_data_0013D390.sub = 4;
            }
            break;
        case 4:
            if (mc_data_0013D390.result == mc_data_0013D390.size) {
                mc_data_0013D390.card[mc_data_0013D390.cur].xAC = 0;
                if (mc_data_0013D390.card[mc_data_0013D390.cur].xB0 != mc_call_0020BAD8(mc_data_001A05C0)) {
                    mc_data_0013D390.card[mc_data_0013D390.cur].xAC++;
                }
                if (mc_data_0013D390.card[mc_data_0013D390.cur].xB4 != mc_call_0020BAD8(mc_data_001A08C0)) {
                    mc_data_0013D390.card[mc_data_0013D390.cur].xAC++;
                }
                mc_data_0013D390.busy = 0;
                mc_data_0013D390.sub = 5;
            } else {
                mc_data_0013D390.card[mc_data_0013D390.cur].x14 = -2;
                mc_data_0013D390.errCard = mc_data_0013D390.cur;
                if (mc_data_0013D390.result >= 0) {
                    mc_data_0013D390.err = 0x2716;
                    if (mc_call_00123A10(mc_data_0013D390.fd) == 0) {
                        mc_data_0013D390.state = 0;
                        mc_data_0013D390.cur = 0;
                        mc_data_0013D390.busy = 0;
                        if (mc_data_0013D390.card[0].request == 2) mc_data_0013D390.card[0].request = 0;
                    }
                    break;
                }
                if (mc_data_0013D390.result == -5) {
                    mc_data_0013D390.err = 0x2711;
                } else if (mc_data_0013D390.result == -4) {
                    mc_data_0013D390.err = 0x2714;
                } else if (mc_data_0013D390.result == -3) {
                    mc_data_0013D390.err = 0x2715;
                } else if (mc_data_0013D390.result == -2) {
                    mc_data_0013D390.err = 3;
                } else {
                    mc_data_0013D390.err = 4;
                }
                mc_data_0013D390.state = 0;
                mc_data_0013D390.cur = 0;
                mc_data_0013D390.busy = 0;
                if (mc_data_0013D390.card[0].request == 2) mc_data_0013D390.card[0].request = 0;
            }
            break;
        case 5:
            if (mc_call_00123A10(mc_data_0013D390.fd) == 0) {
                mc_data_0013D390.busy = 0;
                mc_data_0013D390.state = 21;
            }
            break;
        }
        break;

    case 9:
        mc_data_0013D390.state = 10;
        mc_data_0013D390.sub = 0;
        mc_data_0013D390.card[mc_data_0013D390.cur].x14 = 0;
    case 10:
        switch (mc_data_0013D390.sub) {
        case 0:
            if (mc_data_0013D390.card[mc_data_0013D390.cur].free + mc_data_0013D390.card[mc_data_0013D390.cur].used_blocks >= 351) {
                mc_data_0013D390.sub = 1;
            } else {
                mc_data_0013D390.err = 7;
                mc_data_0013D390.errCard = mc_data_0013D390.cur;
                mc_data_0013D390.state = 0;
                mc_data_0013D390.cur = 0;
            }
            mc_data_0013D390.busy = 0;
            break;
        case 1:
            if (mc_call_001239D8(mc_data_0013D390.card[mc_data_0013D390.cur].port, mc_data_0013D390.card[mc_data_0013D390.cur].slot, mc_data_0013D2D0) == 0) {
                mc_data_0013D390.sub = 2;
            }
            break;
        case 2:
            if (mc_data_0013D390.result == 0 || mc_data_0013D390.result == -4) {
                mc_data_0013D390.sub = 3;
                break;
            }
            mc_data_0013D390.errCard = mc_data_0013D390.cur;
            if (mc_data_0013D390.result == -3) {
                mc_data_0013D390.err = 7;
            } else if (mc_data_0013D390.result == -2) {
                mc_data_0013D390.err = 6;
            } else {
                mc_data_0013D390.err = 0xD;
            }
            mc_data_0013D390.state = 0;
            mc_data_0013D390.cur = 0;
            mc_data_0013D390.busy = 0;
            break;
        case 3: {
            McChunk_209E68 *chunk;
            s32 size;

            mc_call_001FDF10(mc_data_00137C80.x14 << 11, &chunk, &size);
            mc_call_00217628(chunk, mc_data_00137C80.x10, mc_data_00137C80.x14);
            mc_data_0013D390.sub = 4;
            mc_data_0013D390.busy = 0;
            break;
        }
        case 4:
            if (mc_data_001517D8[0] == 0) {
                mc_data_0013D390.sub = 5;
            }
            mc_data_0013D390.busy = 0;
            break;
        case 5:
        case 9:
        case 13:
        case 18:
            switch (mc_data_0013D390.sub) {
            case 5:
                mc_call_001166FC(workspace.name, mc_data_0013D300);
                break;
            case 9:
                mc_call_001166FC(workspace.name, mc_data_0013D320);
                break;
            case 18:
                mc_call_001166FC(workspace.name, mc_data_0013D340);
                break;
            case 13:
                mc_call_00116248(workspace.name, mc_data_0013D370, mc_data_0013D390.card[mc_data_0013D390.cur].x14);
                break;
            }
            if (mc_call_001238B0(mc_data_0013D390.card[mc_data_0013D390.cur].port, mc_data_0013D390.card[mc_data_0013D390.cur].slot, workspace.name, 0x203) == 0) {
                mc_data_0013D390.fd = -1;
                mc_data_0013D390.sub++;
            }
            break;
        case 6:
        case 10:
        case 14:
        case 19: {
            McChunk_209E68 *chunk;
            s32 size;

            if (mc_data_0013D390.result >= 0) {
                if (mc_data_0013D390.fd < 0) {
                    mc_data_0013D390.fd = mc_data_0013D390.result;
                }
                mc_call_001FDF10(mc_data_00137C94[0] << 11, &chunk, &size);
                switch (mc_data_0013D390.sub) {
                case 6:
                    mc_data_0013D390.size = 0x3C4;
                    mc_data_0013D390.buf = (u8 *)chunk + chunk->x0;
                    break;
                case 10:
                    mc_data_0013D390.size = chunk->xC;
                    mc_data_0013D390.buf = (u8 *)chunk + chunk->x8;
                    break;
                case 19:
                    if (mc_data_0015EF90 != 0) {
                        mc_data_0013D390.size = 0x3C00;
                    } else {
                        mc_data_0013D390.size = 0x3C04;
                    }
                    mc_data_0013D390.buf = &mc_data_0015EE84;
                    break;
                case 14: {
                    s32 n = mc_call_0020BAD8(mc_data_001A05C0);
                    mc_data_0013D390.size = n + mc_call_0020BAD8(mc_data_001A08C0) * 20 + 8;
                    mc_data_0013D390.buf = (u8 *)chunk + chunk->x10;
                    break;
                }
                }
                if (mc_call_00123D48(mc_data_0013D390.fd, mc_data_0013D390.buf, mc_data_0013D390.size) == 0) {
                    mc_data_0013D390.sub++;
                }
            } else {
                mc_data_0013D390.err = 0xA;
                mc_data_0013D390.errCard = mc_data_0013D390.cur;
                mc_data_0013D390.state = 0;
                mc_data_0013D390.cur = 0;
                mc_data_0013D390.busy = 0;
            }
            break;
        }
        case 7:
        case 11:
        case 15:
        case 20:
            if (mc_data_0013D390.result == mc_data_0013D390.size) {
                if (mc_call_00123A10(mc_data_0013D390.fd) == 0) {
                    mc_data_0013D390.sub++;
                }
                break;
            }
            mc_data_0013D390.errCard = mc_data_0013D390.cur;
            if (mc_data_0013D390.result >= 0) {
                finished_card = &mc_data_0013D390;
                finished_card->err = 0xB;
                if (mc_call_00123A10(finished_card->fd) == 0) {
                    finished_card->busy = 0;
                    goto reset_state;
                }
                break;
            }
            if (mc_data_0013D390.result == -4) {
                mc_data_0013D390.err = 8;
            } else if (mc_data_0013D390.result == -3) {
                mc_data_0013D390.err = 7;
            } else if (mc_data_0013D390.result == -2) {
                mc_data_0013D390.err = 6;
            } else {
                mc_data_0013D390.err = 0xD;
            }
            mc_data_0013D390.state = 0;
            mc_data_0013D390.cur = 0;
            mc_data_0013D390.busy = 0;
            break;
        case 8:
        case 12:
        case 16:
            if (mc_data_0013D390.result == 0) {
                mc_data_0013D390.sub++;
            } else {
                mc_data_0013D390.err = 0xC;
                mc_data_0013D390.errCard = mc_data_0013D390.cur;
                mc_data_0013D390.state = 0;
                mc_data_0013D390.cur = 0;
            }
            mc_data_0013D390.busy = 0;
            break;
        case 17:
            if (++mc_data_0013D390.card[mc_data_0013D390.cur].x14 < 5) {
                mc_data_0013D390.sub = 13;
            } else {
                mc_data_0013D390.sub++;
            }
            mc_data_0013D390.busy = 0;
            break;
        case 21:
            mc_data_0013D390.card[0].x14 = -1;
            mc_data_0013D390.state = 0;
            mc_data_0013D390.cur = 0;
            mc_data_0013D390.card[0].xAC = 0;
            break;
        }
        break;

    case 11:
        mc_data_0013D390.state = 12;
        mc_data_0013D390.sub = 0;
        mc_data_0013D390.card[mc_data_0013D390.cur].x14 = -2;
    case 12:
        switch (mc_data_0013D390.sub) {
        case 0:
            if (mc_call_001241F0(mc_data_0013D390.card[mc_data_0013D390.cur].port, mc_data_0013D390.card[mc_data_0013D390.cur].slot, mc_data_0013D2E8, 0, 1, mc_data_001A0980) == 0) {
                mc_data_0013D390.sub = 1;
            }
            break;
        case 1:
            if (mc_data_0013D390.result == 1) {
                mc_call_00116248(workspace.name, mc_data_0015FF68, mc_data_0013D2D0, mc_data_001A09A0);
                mc_call_001E9730(mc_data_0015FF70, workspace.name);
                if (mc_call_00124410(mc_data_0013D390.card[mc_data_0013D390.cur].port, mc_data_0013D390.card[mc_data_0013D390.cur].slot, workspace.name) == 0) {
                    mc_data_0013D390.sub = 2;
                }
            } else {
                if (mc_data_0013D390.result == 0) {
                    mc_data_0013D390.sub = 3;
                } else {
                    mc_data_0013D390.errCard = mc_data_0013D390.cur;
                    if (mc_data_0013D390.result == -5) {
                        mc_data_0013D390.err = 0x10;
                    } else if (mc_data_0013D390.result == -4) {
                        mc_data_0013D390.err = 0xF;
                    } else if (mc_data_0013D390.result == -2) {
                        mc_data_0013D390.err = 0xE;
                    } else {
                        mc_data_0013D390.err = 0x12;
                    }
                    mc_data_0013D390.state = 0;
                    mc_data_0013D390.cur = 0;
                }
                mc_data_0013D390.busy = 0;
            }
            break;
        case 2:
            if (mc_call_001241F0(mc_data_0013D390.card[mc_data_0013D390.cur].port, mc_data_0013D390.card[mc_data_0013D390.cur].slot, mc_data_0013D2E8, 1, 1, mc_data_001A0980) == 0) {
                mc_data_0013D390.sub = 1;
            }
            break;
        case 3:
            if (mc_call_00124410(mc_data_0013D390.card[mc_data_0013D390.cur].port, mc_data_0013D390.card[mc_data_0013D390.cur].slot, mc_data_0013D2D0) == 0) {
                mc_call_001E9730(mc_data_001E8518, mc_data_0013D2D0);
                mc_data_0013D390.sub = 4;
            }
            break;
        case 4:
            if (mc_data_0013D390.result != 0) {
                mc_data_0013D390.errCard = mc_data_0013D390.cur;
                if (mc_data_0013D390.result == -6) {
                    mc_data_0013D390.err = 0x11;
                } else if (mc_data_0013D390.result == -5) {
                    mc_data_0013D390.err = 0x10;
                } else if (mc_data_0013D390.result == -4) {
                    mc_data_0013D390.err = 0xF;
                } else if (mc_data_0013D390.result == -2) {
                    mc_data_0013D390.err = 0xE;
                } else {
                    mc_data_0013D390.err = 0x12;
                }
            }
            mc_data_0013D390.state = 0;
            mc_data_0013D390.cur = 0;
            mc_data_0013D390.busy = 0;
            break;
        }
        break;

    case 21:
        mc_data_0013D390.state = 22;
        mc_data_0013D390.card[mc_data_0013D390.cur].x18 = -1;
        mc_data_0013D390.xC8 = 0;
        break;

    case 22:
        mc_data_0013D390.card[mc_data_0013D390.cur].x18++;
        if (mc_data_0013D390.card[mc_data_0013D390.cur].x18 < 5) {
            mc_data_0013D390.sub = 0;
            mc_data_0013D390.state = 23;
        } else {
            mc_data_0013D390.state = 0;
            mc_data_0013D390.cur = 0;
            mc_data_0013D390.busy = 0;
            mc_data_0013D390.card[0].request = 0;
        }
        break;
    case 13:
        if (mc_data_0013D390.card[mc_data_0013D390.cur].x14 < 0) {
            mc_data_0013D390.err = 0x13;
            mc_data_0013D390.state = 0;
            mc_data_0013D390.cur = 0;
            mc_data_0013D390.busy = 0;
            if (mc_data_0013D390.card[0].request == 2) mc_data_0013D390.card[0].request = 0;
            break;
        }
        mc_data_0013D390.xC8 = 0;
        mc_data_0013D390.sub = 0;
        mc_data_0013D390.state = 14;
    case 14:
    case 23:
        switch (mc_data_0013D390.sub) {
        case 0:
            if (mc_data_0013D390.state == 23) {
                mc_call_00116248(workspace.name, mc_data_0013D370, mc_data_0013D390.card[mc_data_0013D390.cur].x18);
            } else {
                mc_call_00116248(workspace.name, mc_data_0013D370, mc_data_0013D390.card[mc_data_0013D390.cur].x14);
            }
            if (mc_call_001238B0(mc_data_0013D390.card[mc_data_0013D390.cur].port, mc_data_0013D390.card[mc_data_0013D390.cur].slot, workspace.name, 1) == 0) {
                mc_data_0013D390.sub++;
            }
            break;
        case 1:
            if (mc_data_0013D390.result >= 0) {
                mc_data_0013D390.fd = mc_data_0013D390.result;
                mc_data_0013D390.sub = 2;
            } else {
                mc_data_0013D390.errCard = mc_data_0013D390.cur;
                if (mc_data_0013D390.result == -7) {
                    mc_data_0013D390.err = 0x14;
                } else if (mc_data_0013D390.result == -5) {
                    mc_data_0013D390.err = 0x15;
                } else if (mc_data_0013D390.result == -4) {
                    mc_data_0013D390.err = 0x16;
                } else if (mc_data_0013D390.result == -3) {
                    mc_data_0013D390.err = 0x17;
                } else if (mc_data_0013D390.result == -2) {
                    mc_data_0013D390.err = 0x18;
                } else {
                    mc_data_0013D390.err = 0x1C;
                }
                mc_data_0013D390.state = 0;
                mc_data_0013D390.cur = 0;
                if (mc_data_0013D390.card[0].request == 2) mc_data_0013D390.card[0].request = 0;
            }
            mc_data_0013D390.busy = 0;
            break;
        case 2:
            mc_data_0013D390.size = 8;
            if (mc_call_00123C30(mc_data_0013D390.fd, &mc_data_0013D390.card[mc_data_0013D390.cur].xB0, 8) == 0) {
                mc_data_0013D390.sub = 3;
            }
            break;
        case 3:
            if (mc_data_0013D390.result == mc_data_0013D390.size) {
                mc_data_0013D390.busy = 0;
                mc_data_0013D390.sub = 4;
                break;
            }
            mc_data_0013D390.errCard = mc_data_0013D390.cur;
            if (mc_data_0013D390.result >= 0) {
                mc_data_0013D390.err = 0x1B;
                if (mc_call_00123A10(mc_data_0013D390.fd) == 0) {
                    mc_data_0013D390.state = 0;
                    mc_data_0013D390.cur = 0;
                    mc_data_0013D390.busy = 0;
                    if (mc_data_0013D390.card[0].request == 2) mc_data_0013D390.card[0].request = 0;
                }
                break;
            }
            if (mc_data_0013D390.result == -5) {
                mc_data_0013D390.err = 0x15;
            } else if (mc_data_0013D390.result == -4) {
                mc_data_0013D390.err = 0x19;
            } else if (mc_data_0013D390.result == -3) {
                mc_data_0013D390.err = 0x1A;
            } else if (mc_data_0013D390.result == -2) {
                mc_data_0013D390.err = 0x18;
            } else {
                mc_data_0013D390.err = 0x1C;
            }
            mc_data_0013D390.state = 0;
            mc_data_0013D390.cur = 0;
            mc_data_0013D390.busy = 0;
            if (mc_data_0013D390.card[0].request == 2) mc_data_0013D390.card[0].request = 0;
            break;
        case 4:
            mc_data_0013D390.size = mc_data_0013D390.card[mc_data_0013D390.cur].xB0;
            if (mc_data_0013D390.size > 0x1800) {
                mc_call_001F9978();
            }
            if (mc_call_00123C30(mc_data_0013D390.fd, mc_data_0014EFD0, mc_data_0013D390.size) == 0) {
                mc_data_0013D390.sub = 5;
            }
            break;
        case 5:
            if (mc_data_0013D390.result == mc_data_0013D390.size) {
                if (mc_data_0013D390.state == 23) {
                    mc_call_0020BCB0(mc_data_0014EFD0, mc_data_0013D390.cur, mc_data_0013D390.card[mc_data_0013D390.cur].x18);
                    mc_data_0013D390.sub = 8;
                } else {
                    mc_data_0013D390.card[mc_data_0013D390.cur].xAC = mc_call_0020BD70(mc_data_0014EFD0, 0, mc_data_001A05C0);
                    mc_data_0013D390.sub = 6;
                }
                mc_data_0013D390.busy = 0;
                break;
            }
            mc_data_0013D390.errCard = mc_data_0013D390.cur;
            if (mc_data_0013D390.result >= 0) {
                mc_data_0013D390.err = 0x1B;
                if (mc_call_00123A10(mc_data_0013D390.fd) == 0) {
                    mc_data_0013D390.state = 0;
                    mc_data_0013D390.cur = 0;
                    mc_data_0013D390.busy = 0;
                    if (mc_data_0013D390.card[0].request == 2) mc_data_0013D390.card[0].request = 0;
                }
                break;
            }
            if (mc_data_0013D390.result == -5) {
                mc_data_0013D390.err = 0x15;
            } else if (mc_data_0013D390.result == -4) {
                mc_data_0013D390.err = 0x19;
            } else if (mc_data_0013D390.result == -3) {
                mc_data_0013D390.err = 0x1A;
            } else if (mc_data_0013D390.result == -2) {
                mc_data_0013D390.err = 0x18;
            } else {
                mc_data_0013D390.err = 0x1C;
            }
            mc_data_0013D390.state = 0;
            mc_data_0013D390.cur = 0;
            mc_data_0013D390.busy = 0;
            if (mc_data_0013D390.card[0].request == 2) mc_data_0013D390.card[0].request = 0;
            break;
        case 6:
            mc_data_0013D390.size = mc_data_0013D390.card[mc_data_0013D390.cur].xB4;
            if (mc_data_0013D390.size > 0x1000) {
                mc_call_001F9978();
            }
            if (mc_call_00123C30(mc_data_0013D390.fd, mc_data_001507D0, mc_data_0013D390.size) == 0) {
                mc_data_0013D390.sub = 7;
            }
            break;
        case 7:
            if (mc_data_0013D390.result == mc_data_0013D390.size) {
                mc_data_0013D390.card[mc_data_0013D390.cur].xAC += mc_call_0020BD70(mc_data_001507D0, mc_data_0013D390.xC8, mc_data_001A08C0);
                if (++mc_data_0013D390.xC8 < 20) {
                    mc_data_0013D390.sub = 6;
                } else {
                    mc_data_0013D390.sub = 8;
                }
                mc_data_0013D390.busy = 0;
                break;
            }
            mc_data_0013D390.errCard = mc_data_0013D390.cur;
            if (mc_data_0013D390.result >= 0) {
                mc_data_0013D390.err = 0x1B;
                if (mc_call_00123A10(mc_data_0013D390.fd) == 0) {
                    mc_data_0013D390.state = 0;
                    mc_data_0013D390.cur = 0;
                    mc_data_0013D390.busy = 0;
                    if (mc_data_0013D390.card[0].request == 2) mc_data_0013D390.card[0].request = 0;
                }
                break;
            }
            if (mc_data_0013D390.result == -5) {
                mc_data_0013D390.err = 0x15;
            } else if (mc_data_0013D390.result == -4) {
                mc_data_0013D390.err = 0x19;
            } else if (mc_data_0013D390.result == -3) {
                mc_data_0013D390.err = 0x1A;
            } else if (mc_data_0013D390.result == -2) {
                mc_data_0013D390.err = 0x18;
            } else {
                mc_data_0013D390.err = 0x1C;
            }
        reset_request:
            mc_data_0013D390.state = 0;
            mc_data_0013D390.cur = 0;
            mc_data_0013D390.busy = 0;
            if (mc_data_0013D390.card[0].request == 2) mc_data_0013D390.card[0].request = 0;
            break;
        case 8:
            if (mc_call_00123A10(mc_data_0013D390.fd) == 0) {
                if (mc_data_0013D390.state == 23) {
                    mc_data_0013D390.busy = 0;
                    mc_data_0013D390.state = 22;
                } else {
                    finished_card = &mc_data_0013D390;
                    finished_card->card[0].request = 0;
                    goto reset_state;
                }
            }
            break;
        }
        break;

    case 15:
        if (mc_data_0013D390.card[mc_data_0013D390.cur].xAC != 0) {
            mc_data_0013D390.busy = 0;
            mc_data_0013D390.state = 0;
            mc_data_0013D390.cur = 0;
            mc_data_0013D390.err = 0x2717;
            break;
        }
        if (mc_data_0013D390.card[mc_data_0013D390.cur].x14 < 0) {
            mc_data_0013D390.busy = 0;
            mc_data_0013D390.state = 0;
            mc_data_0013D390.cur = 0;
            mc_data_0013D390.err = 0x1D;
            break;
        }
        mc_data_0013D390.sub = 0;
        mc_data_0013D390.state = 16;
    case 16:
        switch (mc_data_0013D390.sub) {
        case 0:
            mc_call_00116248(workspace.name, mc_data_0013D370, mc_data_0013D390.card[mc_data_0013D390.cur].x14);
            if (mc_call_001238B0(mc_data_0013D390.card[mc_data_0013D390.cur].port, mc_data_0013D390.card[mc_data_0013D390.cur].slot, workspace.name, 2) == 0) {
                mc_data_0013D390.sub = 1;
            }
            break;
        case 1:
            if (mc_data_0013D390.result >= 0) {
                mc_data_0013D390.fd = mc_data_0013D390.result;
                mc_data_0013D390.sub = 2;
            } else {
                mc_data_0013D390.errCard = mc_data_0013D390.cur;
                if (mc_data_0013D390.result == -7) {
                    mc_data_0013D390.err = 0x1E;
                } else if (mc_data_0013D390.result == -5) {
                    mc_data_0013D390.err = 0x1F;
                } else if (mc_data_0013D390.result == -4) {
                    mc_data_0013D390.err = 0x20;
                } else if (mc_data_0013D390.result == -3) {
                    mc_data_0013D390.err = 0x21;
                } else if (mc_data_0013D390.result == -2) {
                    mc_data_0013D390.err = 0x22;
                } else {
                    mc_data_0013D390.err = 0x26;
                }
                mc_data_0013D390.state = 0;
                mc_data_0013D390.cur = 0;
            }
            mc_data_0013D390.busy = 0;
            break;
        case 2:
            if (mc_call_00123AC8(mc_data_0013D390.fd, 8, 0) == 0) {
                mc_data_0013D390.sub = 3;
            }
            break;
        case 3:
            if (mc_data_0013D390.result >= 0) {
                mc_data_0013D390.sub = 4;
            } else {
                mc_data_0013D390.state = 0;
                mc_data_0013D390.cur = 0;
                mc_data_0013D390.busy = 0;
            }
            break;
        case 4:
            if (mc_call_00123D48(mc_data_0013D390.fd, mc_data_0014EFD0, mc_call_0020BAD8(mc_data_001A05C0)) == 0) {
                mc_data_0013D390.sub = 5;
            }
            break;
        case 5:
            if (mc_data_0013D390.result == mc_call_0020BAD8(mc_data_001A05C0)) {
                mc_data_0013D390.sub = 6;
                break;
            }
            mc_data_0013D390.errCard = mc_data_0013D390.cur;
            if (mc_data_0013D390.result >= 0) {
                finished_card = &mc_data_0013D390;
                finished_card->err = 0x25;
                if (mc_call_00123A10(finished_card->fd) == 0) {
                    finished_card->busy = 0;
                    goto reset_state;
                }
                break;
            }
            if (mc_data_0013D390.result == -5) {
                mc_data_0013D390.err = 0x1F;
            } else if (mc_data_0013D390.result == -4) {
                mc_data_0013D390.err = 0x23;
            } else if (mc_data_0013D390.result == -3) {
                mc_data_0013D390.err = 0x24;
            } else if (mc_data_0013D390.result == -2) {
                mc_data_0013D390.err = 0x22;
            } else {
                mc_data_0013D390.err = 0x26;
            }
            mc_data_0013D390.state = 0;
            mc_data_0013D390.cur = 0;
            mc_data_0013D390.busy = 0;
            break;
        case 6:
            if (mc_data_0013D390.xC8 == 0) {
                mc_data_0013D390.sub = 8;
                break;
            }
            if (mc_call_00123AC8(mc_data_0013D390.fd, mc_data_0013D390.xC8 * mc_call_0020BAD8(mc_data_001A08C0), 1) == 0) {
                mc_data_0013D390.sub = 7;
            }
            break;
        case 7:
            if (mc_data_0013D390.result >= 0) {
                mc_data_0013D390.sub = 8;
            } else {
                mc_data_0013D390.state = 0;
                mc_data_0013D390.cur = 0;
                mc_data_0013D390.busy = 0;
            }
            break;
        case 8:
            if (mc_call_00123D48(mc_data_0013D390.fd, mc_data_001507D0, mc_call_0020BAD8(mc_data_001A08C0)) == 0) {
                mc_data_0013D390.sub = 9;
            }
            break;
        case 9:
            if (mc_data_0013D390.result == mc_call_0020BAD8(mc_data_001A08C0)) {
                mc_data_0013D390.sub = 10;
                break;
            }
            mc_data_0013D390.errCard = mc_data_0013D390.cur;
            if (mc_data_0013D390.result >= 0) {
                finished_card = &mc_data_0013D390;
                finished_card->err = 0x25;
                if (mc_call_00123A10(finished_card->fd) == 0) {
                    finished_card->busy = 0;
                reset_state:
                    finished_card->cur = 0;
                    finished_card->state = 0;
                }
                break;
            }
            if (mc_data_0013D390.result == -5) {
                mc_data_0013D390.err = 0x1F;
            } else if (mc_data_0013D390.result == -4) {
                mc_data_0013D390.err = 0x23;
            } else if (mc_data_0013D390.result == -3) {
                mc_data_0013D390.err = 0x24;
            } else if (mc_data_0013D390.result == -2) {
                mc_data_0013D390.err = 0x18;
            } else {
                mc_data_0013D390.err = 0x1C;
            }
            mc_data_0013D390.state = 0;
            mc_data_0013D390.cur = 0;
            mc_data_0013D390.busy = 0;
            break;
        case 10:
            finished_card = &mc_data_0013D390;
            if (mc_call_00123A10(finished_card->fd) == 0) {
                finished_card->state = 0;
                finished_card->cur = 0;
            }
            break;
        }
        break;

    case 17: {
        McChunk_209E68 *chunk;
        s32 size;

        mc_call_001FDF10(mc_data_00137C80.x14 << 11, &chunk, &size);
        mc_call_00217628(chunk, mc_data_00137C80.x10, mc_data_00137C80.x14);
        mc_data_0013D390.sub = 18;
        mc_data_0013D390.busy = 0;
        break;
    }

    case 18:
        if (mc_data_001517D8[0] == 0) {
            McChunk_209E68 *chunk;
            s32 size;

            mc_call_001FDF10(mc_data_00137C94[0] << 11, &chunk, &size);
            mc_data_0013D390.sub = 19;
            mc_data_0013D390.buf = (u8 *)chunk + chunk->x10;
        }
        mc_data_0013D390.busy = 0;
        break;

    case 19:
        if (mc_data_0013D390.card[mc_data_0013D390.cur].xAC != 0) {
            mc_data_0013D390.busy = 0;
            mc_data_0013D390.state = 0;
            mc_data_0013D390.cur = 0;
            mc_data_0013D390.err = 0x2718;
            break;
        }
        if (mc_data_0013D390.card[mc_data_0013D390.cur].x14 < 0) {
            mc_data_0013D390.busy = 0;
            mc_data_0013D390.state = 0;
            mc_data_0013D390.cur = 0;
            mc_data_0013D390.err = 0x27;
            break;
        }
        mc_data_0013D390.xC8 = 0;
        mc_data_0013D390.sub = 0;
        mc_data_0013D390.state = 20;
    case 20:
        switch (mc_data_0013D390.sub) {
        case 0:
            mc_call_00116248(workspace.name, mc_data_0013D370, mc_data_0013D390.card[mc_data_0013D390.cur].x14);
            if (mc_call_001238B0(mc_data_0013D390.card[mc_data_0013D390.cur].port, mc_data_0013D390.card[mc_data_0013D390.cur].slot, workspace.name, 2) == 0) {
                mc_data_0013D390.sub = 1;
            }
            break;
        case 1:
            if (mc_data_0013D390.result >= 0) {
                s32 n;

                mc_data_0013D390.fd = mc_data_0013D390.result;
                n = mc_call_0020BAD8(mc_data_001A05C0);
                mc_data_0013D390.size = n + mc_call_0020BAD8(mc_data_001A08C0) * 20 + 8;
                if (mc_call_00123D48(mc_data_0013D390.fd, mc_data_0013D390.buf, mc_data_0013D390.size) == 0) {
                    mc_data_0013D390.sub = 2;
                }
                break;
            }
            mc_data_0013D390.err = 0x2B;
            mc_data_0013D390.errCard = mc_data_0013D390.cur;
            mc_data_0013D390.state = 0;
            mc_data_0013D390.cur = 0;
            mc_data_0013D390.busy = 0;
            break;
        case 2:
            if (mc_data_0013D390.result == mc_data_0013D390.size) {
                if (mc_call_00123A10(mc_data_0013D390.fd) == 0) {
                    mc_data_0013D390.sub = 3;
                }
                break;
            }
            mc_data_0013D390.errCard = mc_data_0013D390.cur;
            if (mc_data_0013D390.result >= 0) {
                mc_data_0013D390.err = 0xB;
                if (mc_call_00123A10(mc_data_0013D390.fd) == 0) {
                    mc_data_0013D390.state = 0;
                    mc_data_0013D390.cur = 0;
                    mc_data_0013D390.busy = 0;
                }
                break;
            }
            if (mc_data_0013D390.result == -4) {
                mc_data_0013D390.err = 0x28;
            } else if (mc_data_0013D390.result == -3) {
                mc_data_0013D390.err = 0x29;
            } else if (mc_data_0013D390.result == -2) {
                mc_data_0013D390.err = 0x2A;
            } else {
                mc_data_0013D390.err = 0x2D;
            }
            mc_data_0013D390.state = 0;
            mc_data_0013D390.cur = 0;
            mc_data_0013D390.busy = 0;
            break;
        case 3:
            if (mc_data_0013D390.result != 0) {
                mc_data_0013D390.err = 0x2C;
                mc_data_0013D390.errCard = mc_data_0013D390.cur;
            }
            mc_data_0013D390.state = 0;
            mc_data_0013D390.cur = 0;
            mc_data_0013D390.busy = 0;
            break;
        }
        break;
    }
}
