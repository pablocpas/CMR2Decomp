// Compare the actual startup tables with a dump from original + SilentPatch.
#include "port/types.h"
#include "FixedPoint.h"
#include <cstdio>
#include <cstdlib>

extern void Scene_InitFixedMathTables();
extern short g_acosTable[4096];

template <class T> static void Compare(FILE *file, const char *name, const T *table, unsigned count) {
    for (unsigned i = 0; i < count; ++i) {
        T expected;
        if (fread(&expected, sizeof(expected), 1, file) != 1) {
            fprintf(stderr, "math tables: truncated reference\n"); exit(1);
        }
        if (table[i] != expected) {
            fprintf(stderr, "%s[%u]: original %lld, port %lld\n", name, i,
                    (long long)expected, (long long)table[i]); exit(1);
        }
    }
}

int main(int argc, char **argv) {
    if (argc != 2) return 2;
    FILE *file = fopen(argv[1], "rb");
    if (!file) { perror(argv[1]); return 2; }
    Scene_InitFixedMathTables();
    Compare(file, "sqrt", g_sqrtTable, 4096);
    Compare(file, "sin", g_sinTable, 4096);
    Compare(file, "acos", g_acosTable, 4096);
    Compare(file, "atan", g_atanTable, 512);
    Compare(file, "tan", g_tanTable, 4096);
    if (fgetc(file) != EOF || ferror(file)) return 1;
    fclose(file);
    puts("math tables: all 16,896 entries identical to original + SilentPatch");
}
