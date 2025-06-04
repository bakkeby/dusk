typedef struct FlexDim FlexDim;
struct FlexDim {
	int x;    /* x position */
	int y;    /* y position */
	int h;    /* height */
	int w;    /* width */
	int ih;   /* inner horizontal gap */
	int iv;   /* inner vertical gap */
	int n;    /* total number of tiled clients */
	int an;   /* number of desired clients for the given tiling area */
	int ai;   /* starting index for desired clients in the given tiling area */
	int grp;  /* tiling area; master, stack or secondary stack area */
};

static void customlayout(int ws_num, char *symbol, int nmaster, int nstack, int split, int master_axis, int stack_axis, int secondary_stack_axis);
static void flextile(Workspace *ws);
static void getfactsforrange(Client *f, int n, int size, int *rest, float *fact, int include_mina);
static void mirrorlayout(const Arg *arg);
static void rotatelayoutaxis(const Arg *arg);
static void setlayoutaxisex(const Arg *arg);
static void setwindowborders(Workspace *ws, Client *sel);
static void layoutconvert(const Arg *arg);
static int convert_arrange(int arrange);
static int convert_split(int split);

/* Symbol handlers */
static void updatelayoutsymbols(Workspace *ws, int n);
static void setflexsymbols(Workspace *ws, int n);
static void monoclesymbols(Workspace *ws, int n);
static void decksymbols(Workspace *ws, int n);

/* Layout split */
static void layout_no_split(Workspace *ws, FlexDim d);
static void layout_split_vertical(Workspace *ws, FlexDim d);
static void layout_split_horizontal(Workspace *ws, FlexDim d);
static void layout_split_vertical_dual_stack(Workspace *ws, FlexDim d);
static void layout_split_horizontal_dual_stack(Workspace *ws, FlexDim d);
static void layout_split_centered_vertical(Workspace *ws, FlexDim d);
static void layout_split_centered_horizontal(Workspace *ws, FlexDim d);
static void layout_floating_master(Workspace *ws, FlexDim d);
static void layout_split_vertical_fixed(Workspace *ws, FlexDim d);
static void layout_split_horizontal_fixed(Workspace *ws, FlexDim d);
static void layout_split_vertical_dual_stack_fixed(Workspace *ws, FlexDim d);
static void layout_split_horizontal_dual_stack_fixed(Workspace *ws, FlexDim d);
static void layout_split_centered_vertical_fixed(Workspace *ws, FlexDim d);
static void layout_split_centered_horizontal_fixed(Workspace *ws, FlexDim d);
static void layout_floating_master_fixed(Workspace *ws, FlexDim d);

/* Layout tile arrangements */
static void arrange_left_to_right(Workspace *ws, FlexDim d);
static void arrange_top_to_bottom(Workspace *ws, FlexDim d);
static void arrange_monocle(Workspace *ws, FlexDim d);
static void arrange_gaplessgrid(Workspace *ws, FlexDim d);
static void arrange_gaplessgrid_cfacts(Workspace *ws, FlexDim d);
static void arrange_gaplessgrid_alt1(Workspace *ws, FlexDim d);
static void arrange_gaplessgrid_alt2(Workspace *ws, FlexDim d);
static void arrange_gridmode(Workspace *ws, FlexDim d);
static void arrange_horizgrid(Workspace *ws, FlexDim d);
static void arrange_dwindle(Workspace *ws, FlexDim d);
static void arrange_dwindle_cfacts(Workspace *ws, FlexDim d);
static void arrange_spiral(Workspace *ws, FlexDim d);
static void arrange_spiral_cfacts(Workspace *ws, FlexDim d);
static void arrange_tatami(Workspace *ws, FlexDim d);
static void arrange_tatami_cfacts(Workspace *ws, FlexDim d);
static void arrange_aspectgrid(Workspace *ws, FlexDim d);
static void arrange_left_to_right_aspect(Workspace *ws, FlexDim d);
static void arrange_top_to_bottom_aspect(Workspace *ws, FlexDim d);
static void arrange_aspect_tiles(Workspace *ws, FlexDim d, int arrange);

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
	TOP_TO_BOTTOM = 0,       // clients are arranged vertically
	LEFT_TO_RIGHT = 1,       // clients are arranged horizontally
	MONOCLE = 2,             // clients are arranged in deck / monocle mode
	GAPLESSGRID = 3,         // clients are arranged in a gapless grid (original formula)
	GAPPLESSGRID = 3,        // ^ alias for backwards compatibility
	GAPLESSGRID_CFACTS = 4,  // clients are arranged in a gapless grid that takes cfacts into account
	GAPPLESSGRID_CFACTS = 4, // ^ alias for backwards compatibility
	GAPLESSGRID_ALT1 = 5,    // clients are arranged in a gapless grid (alt. 1, fills rows first, cfacts for rows)
	GAPPLESSGRID_ALT1 = 5,   // ^ alias for backwards compatibility
	GAPLESSGRID_ALT2 = 6,    // clients are arranged in a gapless grid (alt. 2, fills columns first, cfacts for columns)
	GAPPLESSGRID_ALT2 = 6,   // ^ alias for backwards compatibility
	GRIDMODE = 7,            // clients are arranged in a grid
	HORIZGRID = 8,           // clients are arranged in a horizontal grid
	DWINDLE = 9,             // clients are arranged in fibonacci dwindle mode
	DWINDLE_CFACTS = 10,     // clients are arranged in fibonacci dwindle mode and takes cfacts into account
	SPIRAL = 11,             // clients are arranged in fibonacci spiral mode
	SPIRAL_CFACTS = 12,      // clients are arranged in fibonacci spiral mode and takes cfacts into account
	TATAMI = 13,             // clients are arranged as tatami mats
	TATAMI_CFACTS = 14,      // clients are arranged as tatami mats that takes cfacts into account
	ASPECTGRID = 15,         // clients are arranged in a grid based on their aspect ratio
	TOP_TO_BOTTOM_AR = 16,   // clients are arranged vertically based on their aspect ratio
	LEFT_TO_RIGHT_AR = 17,   // clients are arranged horizontally based on their aspect ratio
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
	65,  // "A",
	61,  // "=",
	124, // "|",
};
