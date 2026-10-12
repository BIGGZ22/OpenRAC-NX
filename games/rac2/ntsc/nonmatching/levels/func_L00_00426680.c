/* meilleur essai -- dernier check.py : "00_aranos_tutorial: %hi at +0x38 differs"
   Les sept magasins sont exacts (l'ordre source doit etre 0,2,1,3,4,5,6 : cc1 applique une
   rotation qui envoie le dernier magasin en tete et l'avant-dernier dans le creneau de jal).
   Il ne reste que la paire %hi/%lo de D_L00_001B1844. */
void func_L00_00426680(char *a0)
{
    int *p = *(int **)(a0 + 0x70);
    char *e;

    p[0] = 0;
    p[2] = 0;
    p[1] = 0;
    p[3] = 0;
    p[4] = 0;
    p[5] = 0;
    p[6] = 0;
    func_L00_00426560(a0);
    e = *(char **)(D_L00_001B1844 + *(short *)(a0 + 0x84) * 32 + 0x1C);
    *(short *)(e + 0x26) = 0;
    *(short *)(a0 + 0x7E) = 0;
}
