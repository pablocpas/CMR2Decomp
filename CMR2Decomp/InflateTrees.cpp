typedef unsigned char Byte;
typedef unsigned int uInt;

struct inflate_huft_s {
    union {
        struct {
            Byte Exop;
            Byte Bits;
        } what;
        uInt pad;
    } word;
    uInt base;
};

typedef struct inflate_huft_s inflate_huft;

#define exop word.what.Exop
#define bits word.what.Bits

#define BMAX 15
#define MANY 1440
#define Z_OK 0
#define Z_DATA_ERROR (-3)
#define Z_MEM_ERROR (-4)
#define Z_BUF_ERROR (-5)

// Builds the multi-level Huffman lookup tables used by the inflate decoder.
// FUNCTION: CMR2 0x004c4000
int huft_build(uInt *b, uInt n, uInt s, const uInt *d, const uInt *e, inflate_huft **t, uInt *m, inflate_huft *hp, uInt *hn, uInt *v)
{
    uInt a;
    uInt c[BMAX + 1];
    uInt f;
    int g;
    int h;
    register uInt i;
    register uInt j;
    register int k;
    int l;
    uInt mask;
    register uInt *p;
    inflate_huft *q;
    struct inflate_huft_s r;
    inflate_huft *u[BMAX];
    register int w;
    uInt x[BMAX + 1];
    uInt *xp;
    int y;
    uInt z;

    p = c;
#define C0 *p++ = 0;
#define C2 C0 C0 C0 C0
#define C4 C2 C2 C2 C2
    C4
    p = b;
    i = n;
    do {
        c[*p++]++;
    } while (--i);
    if (c[0] == n) {
        *t = 0;
        *m = 0;
        return Z_OK;
    }

    l = *m;
    for (j = 1; j <= BMAX; j++)
        if (c[j])
            break;
    k = j;
    if ((uInt)l < j)
        l = j;
    for (i = BMAX; i; i--)
        if (c[i])
            break;
    g = i;
    if ((uInt)l > i)
        l = i;
    *m = l;

    for (y = 1 << j; j < i; j++, y <<= 1)
        if ((y -= c[j]) < 0)
            return Z_DATA_ERROR;
    if ((y -= c[i]) < 0)
        return Z_DATA_ERROR;
    c[i] += y;

    x[1] = j = 0;
    p = c + 1;
    xp = x + 2;
    while (--i) {
        *xp++ = (j += *p++);
    }

    p = b;
    i = 0;
    do {
        if ((j = *p++) != 0)
            v[x[j]++] = i;
    } while (++i < n);
    n = x[g];

    x[0] = i = 0;
    p = v;
    h = -1;
    w = -l;
    u[0] = 0;
    q = 0;
    z = 0;

    for (; k <= g; k++) {
        a = c[k];
        while (a--) {
            while (k > w + l) {
                h++;
                w += l;

                z = g - w;
                z = z > (uInt)l ? l : z;
                if ((f = 1 << (j = k - w)) > a + 1) {
                    f -= a + 1;
                    xp = c + k;
                    if (j < z)
                        while (++j < z) {
                            if ((f <<= 1) <= *++xp)
                                break;
                            f -= *xp;
                        }
                }
                z = 1 << j;

                if (*hn + z > MANY)
                    return Z_MEM_ERROR;
                u[h] = q = hp + *hn;
                *hn += z;

                if (h) {
                    x[h] = i;
                    r.bits = (Byte)l;
                    r.exop = (Byte)j;
                    j = i >> (w - l);
                    r.base = (uInt)(q - u[h - 1] - j);
                    u[h - 1][j] = r;
                } else {
                    *t = q;
                }
            }

            r.bits = (Byte)(k - w);
            if (p >= v + n) {
                r.exop = 128 + 64;
            } else if (*p < s) {
                r.exop = (Byte)(*p < 256 ? 0 : 32 + 64);
                r.base = *p++;
            } else {
                r.exop = (Byte)(e[*p - s] + 16 + 64);
                r.base = d[*p++ - s];
            }

            f = 1 << (k - w);
            for (j = i >> w; j < z; j += f)
                q[j] = r;

            for (j = 1 << (k - 1); i & j; j >>= 1)
                i ^= j;
            i ^= j;

            mask = (1 << w) - 1;
            while ((i & mask) != x[h]) {
                h--;
                w -= l;
                mask = (1 << w) - 1;
            }
        }
    }

    return y != 0 && g != 1 ? Z_BUF_ERROR : Z_OK;
}

