/* functional: medium. Vendor screen: for each of 6 panels, builds the outer/inner screen rects from attachment points, draws the panel pass (switch), overlay, and a textured quad. */
typedef struct { float f[4]; } Vec_3AA38;
extern void xform_3AA38(void *, int, int *, Vec_3AA38 *) __asm__("func_0020DB98");
extern void vsub_3AA38(void *, void *, void *) __asm__("func_001F9BF0");
extern void vadd_3AA38(void *, void *, void *) __asm__("func_001F9BD8");
extern float vlen_3AA38(void *) __asm__("func_001F9CB8");
extern void vscale_3AA38(void *, void *, float) __asm__("func_001F9DC0");
extern void proj_3AA38(void *, void *, float *, float *, float *, float *) __asm__("func_00238F98");
extern float i2f_3AA38(int) __asm__("func_001FA888");
extern int f2i_3AA38(float) __asm__("func_001FA898");
extern void target_3AA38(int, int, float) __asm__("func_0023A948");
extern void bg_3AA38(int, int) __asm__("func_001FB908");
extern void greg_3AA38(int, long) __asm__("func_00234C98");
extern void pass0_3AA38(void *) __asm__("func_00239838");
extern void pass1_3AA38(void *) __asm__("func_00239CF8");
extern void pass2_3AA38(void *) __asm__("func_00239948");
extern void pass3_3AA38(void *, int, int) __asm__("func_0023A220");
extern void pass4_3AA38(void *) __asm__("func_0023A478");
extern void pass5_3AA38(void *) __asm__("func_00239A00");
extern void overlay_3AA38(int, float, float) __asm__("func_0023A5E0");
extern void restore_3AA38(void) __asm__("func_0023AA08");
extern void texref_3AA38(void) __asm__("func_00234E80");
extern void quad_3AA38(float, float, float, float, int, int, int, int, unsigned int, long) __asm__("func_001F5988");
extern float D_001E6518_3AA38[] __asm__("D_001E6518");
extern int D_00161398_3AA38 __asm__("D_00161398");
extern int D_001613A0_3AA38 __asm__("D_001613A0");
extern int D_0016139C_3AA38 __asm__("D_0016139C");
extern int D_00161394_3AA38 __asm__("D_00161394");
extern int D_0015EE80_3AA38 __asm__("D_0015EE80");
extern long D_0015EFD0_3AA38 __asm__("D_0015EFD0");

void func_0023AA38(void *ctx) {
    Vec_3AA38 first, second, origin, pts[3], off, corners[4], c0, c3;
    int idx[3];
    float ow, oh, ox, oy, iw, ih, ix, iy, len, t;
    long tex0;
    int pass;

    for (pass = 0; pass < 6; pass++) {
        float *adj = &D_001E6518_3AA38[pass * 4];
        idx[0] = pass << 2;
        idx[1] = (pass << 2) + 1;
        idx[2] = (pass << 2) + 2;
        xform_3AA38(ctx, 3, idx, pts);
        origin = pts[0];
        vsub_3AA38(&first, &pts[1], pts);
        vsub_3AA38(&second, &pts[2], pts);
        len = vlen_3AA38(&first);
        vscale_3AA38(&off, &first, adj[0]);
        vadd_3AA38(&origin, &origin, &off);
        vscale_3AA38(&first, &first, len - (adj[2] + adj[2]));
        len = vlen_3AA38(&second);
        vscale_3AA38(&off, &second, adj[1]);
        vadd_3AA38(&origin, &origin, &off);
        vscale_3AA38(&second, &second, len - (adj[3] + adj[3]));
        vadd_3AA38(&off, &origin, &first);
        vadd_3AA38(&off, &off, &second);
        proj_3AA38(&origin, &off, &ow, &oh, &ox, &oy);
        if (D_00161398_3AA38 != 0 || D_001613A0_3AA38 != 0) {
            t = 1.0f;
            if (D_001613A0_3AA38 != 0) {
                t = i2f_3AA38(8 - D_0016139C_3AA38) * 0.125f;
            }
            if (D_00161398_3AA38 != 0) {
                t = i2f_3AA38(D_00161394_3AA38) * 0.125f;
            }
            len = vlen_3AA38(&first);
            vscale_3AA38(&off, &first, (len * (1.0f - t)) * 0.5f);
            vscale_3AA38(&first, &first, len * t);
            vadd_3AA38(&origin, &origin, &off);
            len = vlen_3AA38(&second);
            vscale_3AA38(&off, &second, (len * (1.0f - t)) * 0.5f);
            vscale_3AA38(&second, &second, len * t);
            vadd_3AA38(&origin, &origin, &off);
        }
        corners[0] = origin;
        vadd_3AA38(&corners[1], &origin, &first);
        vadd_3AA38(&corners[2], &origin, &second);
        vadd_3AA38(&corners[3], &corners[2], &first);
        c0 = corners[0];
        c3 = corners[3];
        proj_3AA38(&c0, &c3, &iw, &ih, &ix, &iy);
        target_3AA38(9, 7, 1.0f);
        bg_3AA38(0x200, 0x200);
        greg_3AA38(0x42, 0x8000000064L);
        switch (pass) {
        case 0: pass0_3AA38(ctx); break;
        case 1: pass1_3AA38(ctx); break;
        case 2: pass2_3AA38(ctx); break;
        case 3: pass3_3AA38(ctx, (int)iw, (int)ih); break;
        case 4: pass4_3AA38(ctx); break;
        case 5: pass5_3AA38(ctx); break;
        }
        overlay_3AA38(pass, iw, ih);
        restore_3AA38();
        bg_3AA38(0x200, 0x200);
        greg_3AA38(0x42, 0x8000000064L);
        greg_3AA38(8, 5);
        texref_3AA38();
        tex0 = D_0015EFD0_3AA38;
        if (D_0015EE80_3AA38 != 0) {
            float h = oh * 0.92857146f - 1.0f;
            int hi;
            int wi = (int)(ow - 1.0f);
            hi = f2i_3AA38(h);
            quad_3AA38(ix, iy, iw, ih, 0, 0, wi, hi, 0x80808080, tex0);
        } else {
            quad_3AA38(ix, iy, iw, ih, 0, 0, (int)(ow - 1.0f), (int)(oh - 1.0f), 0x80808080, tex0);
        }
        greg_3AA38(8, 0);
    }
}
