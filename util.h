/* See LICENSE file for copyright and license details. */

#ifndef MAX
#define MAX(A, B)               ((A) > (B) ? (A) : (B))
#endif
#ifndef MIN
#define MIN(A, B)               ((A) < (B) ? (A) : (B))
#endif
#define BETWEEN(X, A, B)        ((A) <= (X) && (X) <= (B))

#ifdef _DEBUG
#define DEBUG(...) fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG(...)
#endif

enum {
	SmartGaps = 1 << 0, // enables no gaps if there is only one visible window
	SmartGapsMonocle = 1 << 0, // enforces no (outer) gaps in monocle layout
	Systray = 1 << 2, // enables systray
	SwallowFloating = 1 << 3, // means swallow floating windows by default
	CenteredWindowName = 1 << 4,
	BarActiveGroupBorderColor = 1 << 5, // use border color of active group, otherwise color for master group is used
	SpawnCwd = 1 << 6, // spawn applications in the currently selected client's working directory
	ColorEmoji = 1 << 7,
	Status2DNoAlpha = 1 << 8, // option to not use alpha when drawing status2d status
	HideVacantTags = 1 << 9, // hides vacant tags
	BarBorder = 1 << 10, // draw a border around the bar
	NoBorder = 1 << 11, // as per the noborder patch, show no border when only one client in tiled mode
	Warp = 1 << 12, // warp patch
};

void die(const char *fmt, ...);
void *ecalloc(size_t nmemb, size_t size);
int enabled(const long functionality);
int disabled(const long functionality);
void enablefunc(const long functionality);
void disablefunc(const long functionality);
