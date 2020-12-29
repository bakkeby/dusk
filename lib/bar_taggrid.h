static int width_taggrid(Bar *bar, BarArg *a);
static int draw_taggrid(Bar *bar, BarArg *a);
static int click_taggrid(Bar *bar, Arg *arg, BarArg *a);
static void taggridmovetag(const Arg *arg);

/* grid of tags */
#define TAGGRID_UP                1 << 0
#define TAGGRID_DOWN              1 << 1
#define TAGGRID_LEFT              1 << 2
#define TAGGRID_RIGHT             1 << 3
#define TAGGRID_TOGGLETAG         1 << 4
#define TAGGRID_TAG               1 << 5
#define TAGGRID_VIEW              1 << 6
#define TAGGRID_TOGGLEVIEW        1 << 7