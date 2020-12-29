enum {
	IconsDefault,
	IconsVacant,
	IconsOccupied,
	IconsLast
};

static char * geticon(Monitor *m, int tag, int iconset);
static char * tagicon(Monitor *m, int tag);
static void cycleiconset(const Arg *arg);
static void seticonset(const Arg *arg);
