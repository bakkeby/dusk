static void enable(const Arg *arg);
static int enabled(const long functionality);
static void disable(const Arg *arg);
static int disabled(const long functionality);
static void reload();

enum {
	SmartGaps = 1 << 0,
	Systray = 1 << 1,


} func_options;


// differentiating between system functionality and per tag / workspace functionality?

