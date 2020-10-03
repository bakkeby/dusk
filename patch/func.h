static void enable(const Arg *arg);
static int enabled(const long functionality);
static void disable(const Arg *arg);
static int disabled(const long functionality);
static void reload();

enum {
	SmartGaps = 1 << 0, // enables no gaps if there is only one visible window
	Systray = 1 << 1, // enables systray
	SwallowFloating = 1 << 2, // means swallow floating windows by default
	CenteredWindowName = 1 << 3,
	BarActiveGroupBorderColor = 1 << 4, // use border color of active group, otherwise color for master group is used
	SpawnCwd = 1 << 5, // spawn applications in the currently selected client's working directory

} func_options;


// differentiating between system functionality and per tag / workspace functionality?

