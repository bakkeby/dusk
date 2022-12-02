static void customlayout(const Arg args[], int num_args);
static void flextile(Workspace *ws);
static void getfactsforrange(Workspace *ws, int an, int ai, int size, int *rest, float *fact);
static void mirrorlayout(const Arg *arg);
static void rotatelayoutaxis(const Arg *arg);
static void setlayoutaxisex(const Arg *arg);
static void setwindowborders(Workspace *ws, Client *sel);
static void layoutmonconvert(Workspace *ws, Monitor *from, Monitor *to);
static void layoutconvert(const Arg *arg);
static int convert_arrange(int arrange);
static int convert_split(int split);

/* Symbol handlers */
static void setflexsymbols(Workspace *ws, unsigned int n);
static void monoclesymbols(Workspace *ws, unsigned int n);
static void decksymbols(Workspace *ws, unsigned int n);

/* Layout split */
static void layout_no_split(Workspace *ws, int x, int y, int h, int w, int ih, int iv, int n);
static void layout_split_vertical(Workspace *ws, int x, int y, int h, int w, int ih, int iv, int n);
static void layout_split_horizontal(Workspace *ws, int x, int y, int h, int w, int ih, int iv, int n);
static void layout_split_vertical_dual_stack(Workspace *ws, int x, int y, int h, int w, int ih, int iv, int n);
static void layout_split_horizontal_dual_stack(Workspace *ws, int x, int y, int h, int w, int ih, int iv, int n);
static void layout_split_centered_vertical(Workspace *ws, int x, int y, int h, int w, int ih, int iv, int n);
static void layout_split_centered_horizontal(Workspace *ws, int x, int y, int h, int w, int ih, int iv, int n);
static void layout_floating_master(Workspace *ws, int x, int y, int h, int w, int ih, int iv, int n);
static void layout_split_vertical_fixed(Workspace *ws, int x, int y, int h, int w, int ih, int iv, int n);
static void layout_split_horizontal_fixed(Workspace *ws, int x, int y, int h, int w, int ih, int iv, int n);
static void layout_split_vertical_dual_stack_fixed(Workspace *ws, int x, int y, int h, int w, int ih, int iv, int n);
static void layout_split_horizontal_dual_stack_fixed(Workspace *ws, int x, int y, int h, int w, int ih, int iv, int n);
static void layout_split_centered_vertical_fixed(Workspace *ws, int x, int y, int h, int w, int ih, int iv, int n);
static void layout_split_centered_horizontal_fixed(Workspace *ws, int x, int y, int h, int w, int ih, int iv, int n);
static void layout_floating_master_fixed(Workspace *ws, int x, int y, int h, int w, int ih, int iv, int n);

/* Layout tile arrangements */
static void arrange_left_to_right(Workspace *ws, int ax, int ay, int ah, int aw, int ih, int iv, int n, int an, int ai, int arr, int grp);
static void arrange_top_to_bottom(Workspace *ws, int ax, int ay, int ah, int aw, int ih, int iv, int n, int an, int ai, int arr, int grp);
static void arrange_monocle(Workspace *ws, int ax, int ay, int ah, int aw, int ih, int iv, int n, int an, int ai, int arr, int grp);
static void arrange_gapplessgrid(Workspace *ws, int ax, int ay, int ah, int aw, int ih, int iv, int n, int an, int ai, int arr, int grp);
static void arrange_gapplessgrid_cfacts(Workspace *ws, int ax, int ay, int ah, int aw, int ih, int iv, int n, int an, int ai, int arr, int grp);
static void arrange_gapplessgrid_alt1(Workspace *ws, int ax, int ay, int ah, int aw, int ih, int iv, int n, int an, int ai, int arr, int grp);
static void arrange_gapplessgrid_alt2(Workspace *ws, int ax, int ay, int ah, int aw, int ih, int iv, int n, int an, int ai, int arr, int grp);
static void arrange_gridmode(Workspace *ws, int ax, int ay, int ah, int aw, int ih, int iv, int n, int an, int ai, int arr, int grp);
static void arrange_horizgrid(Workspace *ws, int ax, int ay, int ah, int aw, int ih, int iv, int n, int an, int ai, int arr, int grp);
static void arrange_dwindle(Workspace *ws, int ax, int ay, int ah, int aw, int ih, int iv, int n, int an, int ai, int arr, int grp);
static void arrange_dwindle_cfacts(Workspace *ws, int ax, int ay, int ah, int aw, int ih, int iv, int n, int an, int ai, int arr, int grp);
static void arrange_spiral(Workspace *ws, int ax, int ay, int ah, int aw, int ih, int iv, int n, int an, int ai, int arr, int grp);
static void arrange_spiral_cfacts(Workspace *ws, int ax, int ay, int ah, int aw, int ih, int iv, int n, int an, int ai, int arr, int grp);
static void arrange_tatami(Workspace *ws, int ax, int ay, int ah, int aw, int ih, int iv, int n, int an, int ai, int arr, int grp);
static void arrange_tatami_cfacts(Workspace *ws, int ax, int ay, int ah, int aw, int ih, int iv, int n, int an, int ai, int arr, int grp);

/* Layout arrangements */
enum {
	NO_SPLIT,
	SPLIT_VERTICAL,                    // master stack vertical split
	SPLIT_HORIZONTAL,                  // master stack horizontal split
	SPLIT_CENTERED_VERTICAL,           // centered master vertical split
	SPLIT_CENTERED_HORIZONTAL,         // centered master horizontal split
	SPLIT_VERTICAL_DUAL_STACK,         // master stack vertical split with dual stack
	SPLIT_HORIZONTAL_DUAL_STACK,       // master stack vertical split with dual stack
	FLOATING_MASTER,                   // (fake) floating master
	SPLIT_VERTICAL_FIXED,              // master stack vertical fixed split
	SPLIT_HORIZONTAL_FIXED,            // master stack horizontal fixed split
	SPLIT_CENTERED_VERTICAL_FIXED,     // centered master vertical fixed split
	SPLIT_CENTERED_HORIZONTAL_FIXED,   // centered master horizontal fixed split
	SPLIT_VERTICAL_DUAL_STACK_FIXED,   // master stack vertical split with fixed dual stack
	SPLIT_HORIZONTAL_DUAL_STACK_FIXED, // master stack vertical split with fixed dual stack
	FLOATING_MASTER_FIXED,             // (fake) fixed floating master
	LAYOUT_LAST,
};

static char layoutsymb[] = {
	32,  // " ",
	124, // "|",
	61,  // "=",
	94,  // "^",
	126, // "~",
	58,  // ":",
	59,  // ";",
	43,  // "+",
	124, // "￤",
	61,  // "=",
	94,  // "^",
	126, // "~",
	58,  // ":",
	59,  // ";",
	43,  // "+",
};

/* Tile arrangements */
enum {
	TOP_TO_BOTTOM,       // clients are arranged vertically
	LEFT_TO_RIGHT,       // clients are arranged horizontally
	MONOCLE,             // clients are arranged in deck / monocle mode
	GAPPLESSGRID,        // clients are arranged in a gappless grid (original formula)
	GAPPLESSGRID_CFACTS, // clients are arranged in a gappless grid that takes cfacts into account
	GAPPLESSGRID_ALT1,   // clients are arranged in a gappless grid (alt. 1, fills rows first, cfacts for rows)
	GAPPLESSGRID_ALT2,   // clients are arranged in a gappless grid (alt. 2, fills columns first, cfacts for columns)
	GRIDMODE,            // clients are arranged in a grid
	HORIZGRID,           // clients are arranged in a horizontal grid
	DWINDLE,             // clients are arranged in fibonacci dwindle mode
	DWINDLE_CFACTS,      // clients are arranged in fibonacci dwindle mode and takes cfacts into account
	SPIRAL,              // clients are arranged in fibonacci spiral mode
	SPIRAL_CFACTS,       // clients are arranged in fibonacci spiral mode and takes cfacts into account
	TATAMI,              // clients are arranged as tatami mats
	TATAMI_CFACTS,       // clients are arranged as tatami mats that takes cfacts into account
	AXIS_LAST,
};

static char tilesymb[] = {
	61,  // "=",
	124, // "|",
	68,  // "D",
	71,  // "G",
	71,  // "G",
	49,  // "1",
	50,  // "2"
	35,  // "#",
	126, // "~",
	92,  // "\\",
	92,  // "\\",
	64,  // "@",
	64,  // "@",
	84,  // "T",
	84,  // "T",
};
