#define INC(X)                  ((X) + 2000)
#define MASTER(X)               ((X) + 4000)
#define STACK(X)                ((X) + 5000)
#define PREVSEL                 3000
#define LASTTILED               -1
#define ISINC(X)                ((X)->i > 1000 && (X)->i < 3000)
#define ISPREVSEL(X)            ((X)->i == 3000)
#define ISLAST(X)               ((X)->i < 0)
#define GETINC(X)               ((X)->i - 2000)
#define GETMASTER(X)            ((X)->i - 4000)
#define GETSTACK(X)             ((X)->i - 5000)
#define ISMASTER(X)             ((X)->i >= 4000 && (X)->i < 5000)
#define ISSTACK(X)              ((X)->i >= 5000 && (X)->i < 6000)
#define MOD(N,M)                ((N)%(M) < 0 ? (N)%(M) + (M) : (N)%(M))
#define TRUNC(X,A,B)            (MAX((A), MIN((X), (B))))

static void stackfocus(const Arg *arg);
static void stackpush(const Arg *arg);
static void stackswap(const Arg *arg);
static void stackposclient(const Arg *arg, Client **f);
