struct Preview {
	Window win;
	int show;
};

static void createpreview(Monitor *m);
static void hidepreview(Monitor *m);
static void showpreview(Workspace *w, int x, int y);
static void storepreview(Workspace *ws);