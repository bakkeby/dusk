/* Key binding functions */
static void defaultgaps(const Arg *arg);
static void incrgaps(const Arg *arg);
static void incrigaps(const Arg *arg);
static void incrogaps(const Arg *arg);
static void incrohgaps(const Arg *arg);
static void incrovgaps(const Arg *arg);
static void incrihgaps(const Arg *arg);
static void incrivgaps(const Arg *arg);
static void togglegaps(const Arg *arg);

/* Internals */
static void getgaps(Workspace *ws, int *oh, int *ov, int *ih, int *iv, int *nc);
static void setgaps(int oh, int ov, int ih, int iv);
static void setgapsex(const Arg *arg);
