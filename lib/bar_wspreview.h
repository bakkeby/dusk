struct Preview {
	Window win;
	int show;
};

static void hidewspreview(Monitor *m);
static void showwspreview(Workspace *w, int x, int y);
static void storepreview(Workspace *ws);