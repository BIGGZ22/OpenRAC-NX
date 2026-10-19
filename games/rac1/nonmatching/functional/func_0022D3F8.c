/* functional: high. fcheck: unchecked. Sky triangle packet builder: for each of count triangles (4 bytes each: three vertex indices and a pad byte) whose three vertices are not all flagged (the byte at +0xF of each 16-byte vertex record in verts is nonzero for all three: culled), appends three 32-byte GIF A+D entries to the packet at D_00161000 + 0x20: the vertex colour word from colours[index], sign-extended to 64 bits, to RGBAQ (register 1), then the screen position packed from the low halves of the vertex record's first two words (X, Y; Z = 0) to XYZ2 (register 5), which draws the vertex. If anything was appended, the 32-byte header at the packet start gets a DMA cnt tag and VIF DIRECT for (quadwords - 1) and a GIF tag with NLOOP = quadwords - 2, EOP and one A+D register, and D_00161000 moves past the entries; otherwise nothing is written. Differential test: 5 cases (1-6 triangles with mixed culling flags, and all culled) agree with retail run in an EE emulator (unicorn with an R5900 MMI/VU0 instruction model). */
extern unsigned int *D_00161000;

void func_0022D3F8(int count, unsigned char *tris, int *colours, unsigned char *verts) {
    unsigned int *start = D_00161000;
    unsigned int *out = start + 8;
    unsigned int qw;
    while (count-- > 0) {
        unsigned char *t = tris;
        tris += 4;
        if (verts[t[0] * 16 + 0xF] & verts[t[1] * 16 + 0xF] & verts[t[2] * 16 + 0xF]) {
            continue;
        }
        {
            int k;
            for (k = 0; k < 3; k++) {
                unsigned int idx = t[k];
                unsigned int *v = (unsigned int *)(verts + idx * 16);
                int c = colours[idx];
                out[0] = (unsigned int)c;
                out[1] = (unsigned int)(c >> 31);
                out[2] = 1;
                out[3] = 0;
                out[4] = (v[0] & 0xFFFF) | ((v[1] & 0xFFFF) << 16);
                out[5] = 0;
                out[6] = 5;
                out[7] = 0;
                out += 8;
            }
        }
    }
    qw = (unsigned int)((unsigned char *)out - (unsigned char *)start) >> 4;
    if (qw - 2 == 0) {
        return;
    }
    start[0] = 0x10000000 | (qw - 1);
    start[1] = 0;
    start[2] = 0;
    start[3] = 0x50000000 | (qw - 1);
    start[4] = (qw - 2) | 0x8000;
    start[5] = 0x10000000;
    start[6] = 0xE;
    start[7] = 0;
    D_00161000 = out;
}
