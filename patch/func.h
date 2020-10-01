static void enable(const Arg *arg);
static int enabled(const long functionality);
static void disable(const Arg *arg);
static int disabled(const long functionality);
static void reload();

enum {
	SmartGaps = 1 << 0, // enables no gaps if there is only one visible window
	Systray = 1 << 1, // enables systray
	SwallowFloating = 1 << 2, // means swallow floating windows by default


} func_options;


// differentiating between system functionality and per tag / workspace functionality?

