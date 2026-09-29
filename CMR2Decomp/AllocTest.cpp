void sink(void *);
void f(int x)
{
    char a[4];
    char b[4];
    int c;
    int d;
    char e[4];
    int h;
    int i;
    int j;
    int k;
    int l;
    int m;
    int n;
    int o;
    int p;
    int q;
    int r;
    int s;
    int t;
    sink(a); sink(b); sink(&c); sink(&d); sink(e);
    sink(&h); sink(&i); sink(&j); sink(&k); sink(&l); sink(&m);
    sink(&n); sink(&o); sink(&p); sink(&q); sink(&r); sink(&s); sink(&t);
    (void)x;
}
