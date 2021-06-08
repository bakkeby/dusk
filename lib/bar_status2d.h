static int size_status(Bar *bar, BarArg *a);
static int click_status(Bar *bar, Arg *arg, BarArg *a);
static int draw_status(Bar *bar, BarArg *a);
static int drw_2dtext(Drw *drw, int x, int y, unsigned int w, unsigned int h, unsigned int lpad, const char *text, int invert, Bool ignored, int drawbg, int defscheme);
static void setstatus(const Arg args[], int num_args);
static int status2dtextlength(char *stext);
static void statusclick(const Arg *arg);