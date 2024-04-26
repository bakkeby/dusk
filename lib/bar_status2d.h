typedef struct {
	Picture icon;
	unsigned int icw;
	unsigned int ich;
	char iconpath[256];
} Image;

typedef struct {
	Image image;
	time_t atime;
} ImageBuffer;

static int size_status(Bar *bar, BarArg *a);
static int click_status(Bar *bar, Arg *arg, BarArg *a);
static int draw_status(Bar *bar, BarArg *a);
static int drw_2dtext(Drw *drw, int x, int y, unsigned int w, unsigned int h, unsigned int lpad, char *text, int invert, int drawbg, int defscheme);
static void setstatus(const Arg args[], int num_args);
static int status2dtextlength(char *stext);
static void statusclick(const Arg *arg);
static Image *loadimage(char *path, int use_cache);
static int loadimagefromfile(Image *image, char *path);
static void cleanup2dimagebuffer(void);
