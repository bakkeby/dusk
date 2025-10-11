static Picture geticonprop(Window w, int iconsize, unsigned int *icw, unsigned int *ich);
static void freeicon(Client *c);
static void updateicon(Client *c);
static int load_icon_from_file(Client *c, const char *iconpath);
static int load_image_from_file(Image *image, const char *iconpath);
