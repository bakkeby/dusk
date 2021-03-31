static int width_status(Bar *bar, BarArg *a);
static int click_status(Bar *bar, Arg *arg, BarArg *a);
static int draw_status(Bar *bar, BarArg *a);
static int drawstatusbar(BarArg *a, char *text);
static void setstatus(const Arg args[], int num_args);
static int status2dtextlength(char *stext);
static void statusclick(const Arg *arg);