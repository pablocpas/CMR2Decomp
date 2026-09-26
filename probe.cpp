extern unsigned char g_e[16];
int ext2(int, short *, int);
unsigned char ext_b(void);
unsigned char *ext_p(int);
int ext_i(void);
void t8(int i){ if (ext_b() == 0xff || (char)ext_b() != i) ext2(1,0,0); }
void t9(int i){ if (ext_b() == 0xff || ext_b() != i) ext2(1,0,0); }
void t10(void){ ext2(1,0, ext_b() ? 1 : 0); }
void t11(void){ ext2(1,0, ext_b() != 0); }
void t12(int i){ if (ext_b()) ext2(i,(short*)ext_p(i),1); else ext2(i,(short*)ext_p(i),0); }
