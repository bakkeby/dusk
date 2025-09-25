#include <libconfig.h>

const char *progname = "dusk";
static char *cfg_filename = "dusk.cfg";

static char *_cfg_slopspawnstyle = NULL;
static char *_cfg_slopresizestyle = NULL;
static char *_cfg_toggle_float_pos = NULL;
static char *_cfg_custom_2d_indicator_1 = NULL;
static char *_cfg_custom_2d_indicator_2 = NULL;
static char *_cfg_custom_2d_indicator_3 = NULL;
static char *_cfg_custom_2d_indicator_4 = NULL;
static char *_cfg_custom_2d_indicator_5 = NULL;
static char *_cfg_custom_2d_indicator_6 = NULL;
static char *_cfg_occupied_workspace_label_format = NULL;
static char *_cfg_vacant_workspace_label_format = NULL;
static char *_cfg_cached_strings_array[50] = {0};
static char ***_cfg_colors = NULL;
static char ***_cfg_autostart = NULL;
static char ***_cfg_autorestart = NULL;
static Rule *_cfg_clientrules = NULL;
static BarDef *_cfg_bars = NULL;
static BarRule *_cfg_barrules = NULL;
static WorkspaceRule *_cfg_wsrules = NULL;
static Layout *_cfg_layouts = NULL;
static Button *_cfg_buttons = NULL;
static Command *_cfg_commands = NULL;
static Key *_cfg_keys = NULL;
static StackerIcon *_cfg_stackericons = NULL;
static int persist_client_states_across_restarts = 1;
static int persist_workstate_states_across_restarts = 1;

static int global_hz = 60;
static int dragcfact_hz = 60;
static int dragfact_hz = 60;
static int dragmfact_hz = 60;
static int dragwfact_hz = 60;
static int movemouse_hz = 60;
static int placemouse_hz = 60;
static int resizemouse_hz = 60;
static int swallowmouse_hz = 60;
static int markmouse_hz = 60;

static int num_wsrules = 0;
static int num_autostart = 0;
static int num_autorestart = 0;
static int num_cached_strings = 0;
static int num_client_rules = 0;
static int num_stackericons = 0;
static int num_layouts = 0;
static int num_button_bindings = 0;
static int num_key_bindings = 0;
static int num_commands = 0;

/* libconfig helper functions */
static int setting_length(const config_setting_t *cfg);
const char *setting_get_string_elem(const config_setting_t *cfg, int i);
static int setting_get_int_elem(const config_setting_t *cfg, int i);
const config_setting_t *setting_get_elem(const config_setting_t *cfg, int i);

static int config_lookup_sloppy_bool(const config_t *cfg, const char *name, int *ptr);
static int config_setting_lookup_sloppy_bool(const config_setting_t *cfg, const char *name, int *ptr);
static int _config_setting_get_sloppy_bool(const config_setting_t *cfg, int *ptr);

static int config_lookup_simple_float(const config_t *cfg, const char *name, float *floatptr);
static int config_setting_lookup_simple_float(const config_setting_t *cfg, const char *name, float *floatptr);
static int _config_setting_get_simple_float(const config_setting_t *cfg_item, float *floatptr);

static int config_lookup_strdup(const config_t *cfg, const char *name, char **strptr);
static int config_setting_lookup_strdup(const config_setting_t *cfg, const char *name, char **strptr);
static int _config_setting_strdup_string(const config_setting_t *cfg_item, char **strptr);

static int config_lookup_unsigned_int(const config_t *cfg, const char *name, unsigned int *ptr);
static int config_setting_lookup_unsigned_int(const config_setting_t *cfg, const char *name, unsigned int *ptr);
static int _config_setting_get_unsigned_int(const config_setting_t *cfg_item, unsigned int *ptr);

static void set_config_path(const char* filename, char *config_path, char *config_file);
static void load_config(void);
static void load_fallback_config(void);
static void load_autostart(config_t *cfg);
static void load_bar(config_t *cfg);
static void load_button_bindings(config_t *cfg);
static void load_clientrules(config_t *cfg);
static void load_colors(config_t *cfg);
static void load_commands(config_t *cfg);
static void load_fonts(config_t *cfg);
static void load_functionality(config_t *cfg);
static void load_indicators(config_t *cfg);
static void load_keybindings(config_t *cfg);
static void load_layouts(config_t *cfg);
static void load_workspace(config_t *cfg);
static void load_refresh_rates(config_t *cfg);
static void load_singles(config_t *cfg);
static void cleanup_config(void);
static int parse_align(const char *string);
static int parse_click(const char *string);
static ArgFunc parse_arg_function(const char *string);
static WsFunc parse_layout_function(const char *string);
static SymbolFunc parse_symbol_function(const char *string);
static int parse_indicator(const char *indicator);
static int parse_arg_function_int_constant(const char *string, ArgFunc func, int *ptr);
static int parse_key_type(const char *string);
static int parse_layout(const char *string);
static int parse_layout_split(const char *string);
static int parse_layout_arrangement(const char *string);
static unsigned int parse_modifier(const char *string);
static int parse_scheme(const char *string);
static int parse_stacker_placement(const char *string);
static int parse_bar_rule_value(const char *string);
static void *parse_void_reference(const char *string);
static char ***execv_dup(const char *const *flat, int *count);
static void execv_free(char ***argvv);
const char *parse_window_type(const char *string);
static void parse_module(const char *string, BarRule *rule);
static void *cfg_get_command(const char *string);

static void add_button_binding(unsigned int click, unsigned int mask, unsigned int button, ArgFunc function, int argument, void *void_argument, float float_argument);
#if USE_KEYCODES
static void add_key_binding(int type, unsigned int mod, KeyCode keycode, ArgFunc function, int argument, void *void_argument, float float_argument);
#else
static void add_key_binding(int type, unsigned int mod, KeySym keysym, ArgFunc function, int argument, void *void_argument, float float_argument);
#endif // USE_KEYCODES
static void add_stacker_icon(config_t *cfg, const char *string, int value);

int
config_lookup_strdup(const config_t *cfg, const char *name, char **strptr)
{
	return _config_setting_strdup_string(config_lookup(cfg, name), strptr);
}

int
config_setting_lookup_strdup(const config_setting_t *cfg, const char *name, char **strptr)
{
	return _config_setting_strdup_string(config_setting_lookup(cfg, name), strptr);
}

int
_config_setting_strdup_string(const config_setting_t *cfg_item, char **strptr)
{
	if (!cfg_item)
		return 0;

	const char *string = config_setting_get_string(cfg_item);

	if (!string)
		return 0;

	free(*strptr);
	*strptr = strdup(string);
	return 1;
}

int
config_lookup_simple_float(const config_t *cfg, const char *name, float *floatptr)
{
	return _config_setting_get_simple_float(config_lookup(cfg, name), floatptr);
}

int
config_setting_lookup_simple_float(const config_setting_t *cfg, const char *name, float *floatptr)
{
	return _config_setting_get_simple_float(config_setting_lookup(cfg, name), floatptr);
}

int
_config_setting_get_simple_float(const config_setting_t *cfg_item, float *floatptr)
{
	if (!cfg_item)
		return 0;

	double value = config_setting_get_float(cfg_item);

	*floatptr = (float)value;
	return 1;
}

int
config_lookup_sloppy_bool(const config_t *cfg, const char *name, int *ptr)
{
	return _config_setting_get_sloppy_bool(config_lookup(cfg, name), ptr);
}

int
config_setting_lookup_sloppy_bool(const config_setting_t *cfg, const char *name, int *ptr)
{
	return _config_setting_get_sloppy_bool(config_setting_lookup(cfg, name), ptr);
}

int
_config_setting_get_sloppy_bool(const config_setting_t *cfg_item, int *ptr)
{
	const char *string;

	if (!cfg_item)
		return 0;

	switch (config_setting_type(cfg_item)) {
	case CONFIG_TYPE_INT:
		*ptr = config_setting_get_int(cfg_item);
		return 1;
	case CONFIG_TYPE_STRING:
		string = config_setting_get_string(cfg_item);

		if (string && strlen(string)) {
			char a = tolower(string[0]);
			/* Match for positives like "true", "yes" and "on" */
			*ptr = (a == 't' || a == 'y' || !strcasecmp(string, "on"));
			return 1;
		}
		break;
	case CONFIG_TYPE_BOOL:
		*ptr = config_setting_get_bool(cfg_item);
		return 1;
	}

	return 0;
}

int
config_lookup_unsigned_int(const config_t *cfg, const char *name, unsigned int *ptr)
{
	return _config_setting_get_unsigned_int(config_lookup(cfg, name), ptr);
}

int
config_setting_lookup_unsigned_int(const config_setting_t *cfg, const char *name, unsigned int *ptr)
{
	return _config_setting_get_unsigned_int(config_setting_lookup(cfg, name), ptr);
}

int
_config_setting_get_unsigned_int(const config_setting_t *cfg_item, unsigned int *ptr)
{
	if (!cfg_item)
		return 0;

	int integer = config_setting_get_int(cfg_item);

	if (integer < 0)
		return 0;

	*ptr = (unsigned int)integer;
	return 1;
}

int
setting_length(const config_setting_t *cfg)
{
	if (!cfg)
		return 0;

	switch (config_setting_type(cfg)) {
	case CONFIG_TYPE_GROUP:
	case CONFIG_TYPE_LIST:
	case CONFIG_TYPE_ARRAY:
		return config_setting_length(cfg);
	}

	return 1;
}

const char *
setting_get_string_elem(const config_setting_t *cfg, int i)
{
	if (!cfg)
		return NULL;

	switch (config_setting_type(cfg)) {
	case CONFIG_TYPE_GROUP:
	case CONFIG_TYPE_LIST:
	case CONFIG_TYPE_ARRAY:
		return config_setting_get_string_elem(cfg, i);
	}

	return config_setting_get_string(cfg);
}

int
setting_get_int_elem(const config_setting_t *cfg, int i)
{
	if (!cfg)
		return 0;

	switch (config_setting_type(cfg)) {
	case CONFIG_TYPE_GROUP:
	case CONFIG_TYPE_LIST:
	case CONFIG_TYPE_ARRAY:
		return config_setting_get_int_elem(cfg, i);
	}

	return config_setting_get_int(cfg);
}

const config_setting_t *
setting_get_elem(const config_setting_t *cfg, int i)
{
	if (!cfg)
		return NULL;

	switch (config_setting_type(cfg)) {
	case CONFIG_TYPE_GROUP:
	case CONFIG_TYPE_LIST:
	case CONFIG_TYPE_ARRAY:
		return config_setting_get_elem(cfg, i);
	}

	return cfg;
}

void
set_config_path(const char* filename, char *config_path, char *config_file)
{
	const char *xdg_config_home = getenv("XDG_CONFIG_HOME");
	const char *home = getenv("HOME");

	if (startswith("/", filename)) {
		char *dname = strdup(filename);
		snprintf(config_path, PATH_MAX, "%s", dirname(dname));
		snprintf(config_file, PATH_MAX, "%s", filename);
		free(dname);
		return;
	}

	if (xdg_config_home && xdg_config_home[0] != '\0') {
		snprintf(config_path, PATH_MAX, "%s/%s", xdg_config_home, progname);
		snprintf(config_file, PATH_MAX, "%s/%s", config_path, filename);
	} else if (home) {
		snprintf(config_path, PATH_MAX, "%s/.config/%s", home, progname);
		snprintf(config_file, PATH_MAX, "%s/%s", config_path, filename);
	}
}

char ***
execv_dup(const char *const *flat, int *count)
{
	size_t progcount = 0;

	/* Count number of programs */
	for (size_t i = 0; flat[i]; ) {
		progcount++;
		while (flat[i]) i++;  /* skip program args */
		i++;                  /* skip NULL separator */
	}

	char ***all = malloc((progcount + 1) * sizeof *all);
	if (!all)
		return NULL;

	size_t p = 0;
	for (size_t i = 0; flat[i]; p++) {
		/* count args for this program */
		size_t argc = 0;
		while (flat[i + argc]) argc++;

		all[p] = malloc((argc + 1) * sizeof *all[p]);
		if (!all[p]) {
			execv_free(all);
			return NULL;
		}

		for (size_t j = 0; j < argc; j++) {
			all[p][j] = strdup(flat[i + j]);
			if (!all[p][j]) {
				execv_free(all);
				return NULL;
			}
		}
		all[p][argc] = NULL;

		i += argc + 1; /* skip argv + NULL */
	}
	all[p] = NULL;

	if (count)
		*count = progcount;

	return all;
}

void
execv_free(char ***argvv)
{
	if (!argvv)
		return;

	for (size_t i = 0; argvv[i]; i++) {
		char **argv = argvv[i];
		for (size_t j = 0; argv[j]; j++) {
			free(argv[j]);
		}
		free(argv);
	}
	free(argvv);
}

void
load_config(void)
{
	config_t cfg;
	char config_path[PATH_MAX] = {0};
	char config_file[PATH_MAX] = {0};

	set_config_path(cfg_filename, config_path, config_file);
	config_init(&cfg);
	config_set_include_dir(&cfg, config_path);
	if (config_read_file(&cfg, config_file)) {
		load_singles(&cfg);
		load_commands(&cfg);
		load_layouts(&cfg);
		load_autostart(&cfg);
		load_bar(&cfg);
		load_clientrules(&cfg);
		load_colors(&cfg);
		load_fonts(&cfg);
		load_functionality(&cfg);
		load_indicators(&cfg);
		load_workspace(&cfg);
		load_refresh_rates(&cfg);
		load_button_bindings(&cfg);
		load_keybindings(&cfg);
	} else if (strcmp(config_error_text(&cfg), "file I/O error")) {
		config_error = ecalloc(PATH_MAX + 255, sizeof(char));
		snprintf(config_error, PATH_MAX + 255,
			"Config %s: %s:%d",
			config_error_text(&cfg),
			config_file,
			config_error_line(&cfg));

		fprintf(stderr, "Error reading config at %s\n", config_file);
		fprintf(stderr, "%s:%d - %s\n",
				config_error_file(&cfg),
				config_error_line(&cfg),
				config_error_text(&cfg));
	}

	load_fallback_config();
	config_destroy(&cfg);
}

#define STRDUPIFNULL(X) if (!CFG(X)) CFG(X) = strdup(X)

void
load_fallback_config(void)
{
	int i, j;

	STRDUPIFNULL(slopspawnstyle);
	STRDUPIFNULL(slopresizestyle);
	STRDUPIFNULL(toggle_float_pos);
	STRDUPIFNULL(custom_2d_indicator_1);
	STRDUPIFNULL(custom_2d_indicator_2);
	STRDUPIFNULL(custom_2d_indicator_3);
	STRDUPIFNULL(custom_2d_indicator_4);
	STRDUPIFNULL(custom_2d_indicator_5);
	STRDUPIFNULL(custom_2d_indicator_6);
	STRDUPIFNULL(occupied_workspace_label_format);
	STRDUPIFNULL(vacant_workspace_label_format);

	if (!_cfg_colors) {
		/* Initialise the colors array */
		_cfg_colors = ecalloc(SchemeLast, sizeof(char **));
		for (i = 0; i < SchemeLast; i++) {
			_cfg_colors[i] = ecalloc((ColCount+1), sizeof(char *));
			for (j = 0; j <= ColCount; j++) {
				_cfg_colors[i][j] = NULL;
			}
		}
	}

	/* Fall back to default values if not set via config */
	for (i = 0; i < SchemeLast; i++) {
		for (j = 0; j <= ColCount; j++) {
			if (_cfg_colors[i][j] == NULL && colors[i][j] != NULL) {
				_cfg_colors[i][j] = strdup(colors[i][j]);
			}
		}
	}

	if (!_cfg_autostart) {
		_cfg_autostart = execv_dup(autostart, &num_autostart);
	}

	if (!_cfg_autorestart) {
		_cfg_autorestart = execv_dup(autorestart, &num_autorestart);
	}

	if (!_cfg_clientrules) {
		_cfg_clientrules = clientrules;
		num_client_rules = LENGTH(clientrules);
	}

	if (!_cfg_bars) {
		_cfg_bars = bars;
		num_bars = LENGTH(bars);
	}

	if (!_cfg_barrules) {
		_cfg_barrules = barrules;
		num_barrules = LENGTH(barrules);
	}

	if (!_cfg_wsrules) {
		_cfg_wsrules = wsrules;
		num_wsrules = LENGTH(wsrules);
	}

	if (!_cfg_layouts) {
		_cfg_layouts = layouts;
		num_layouts = LENGTH(layouts);
	}

	if (!_cfg_keys) {
		_cfg_keys = keys;
		num_key_bindings = LENGTH(keys);
	}

	if (!_cfg_buttons) {
		_cfg_buttons = buttons;
		num_button_bindings = LENGTH(buttons);
	}

	if (!_cfg_stackericons) {
		_cfg_stackericons = stackericons;
		num_stackericons = LENGTH(stackericons);
	}
}

#undef STRDUPIFNULL

void
cleanup_config(void)
{
	int i, j;
	Rule *rule;
	BarDef *bar;

	free(_cfg_slopspawnstyle);
	free(_cfg_slopresizestyle);
	free(_cfg_toggle_float_pos);
	free(_cfg_custom_2d_indicator_1);
	free(_cfg_custom_2d_indicator_2);
	free(_cfg_custom_2d_indicator_3);
	free(_cfg_custom_2d_indicator_4);
	free(_cfg_custom_2d_indicator_5);
	free(_cfg_custom_2d_indicator_6);
	free(_cfg_occupied_workspace_label_format);
	free(_cfg_vacant_workspace_label_format);

	/* Cleanup colors */
	for (i = 0; i < SchemeLast; i++) {
		for (j = 0; j < ColCount; j++) {
			free(_cfg_colors[i][j]);
		}
	}
	free(_cfg_colors);

	/* Cleanup client rules */
	if (_cfg_clientrules != clientrules) {
		for (i = 0; i < num_client_rules; i++) {
			rule = &_cfg_clientrules[i];
			free(rule->class);
			free(rule->role);
			free(rule->instance);
			free(rule->title);
			free(rule->wintype);
			free(rule->floatpos);
			free(rule->workspace);
			free(rule->label);
			free(rule->iconpath);
			free(rule->alttitle);
		}
		free(_cfg_clientrules);
	}

	/* Cleanup bar definitions */
	if (_cfg_bars != bars) {
		for (i = 0; i < num_bars; i++) {
			bar = &_cfg_bars[i];
			free(bar->barpos);
			free(bar->name);
			free(bar->extclass);
			free(bar->extinstance);
			free(bar->extname);
		}
		free(_cfg_bars);
	}

	/* Cleanup barrules */
	if (_cfg_barrules != barrules) {
		for (i = 0; i < num_barrules; i++) {
			free(_cfg_barrules[i].name);
		}
		free(_cfg_barrules);
	}

	/* Cleanup workspace rules */
	if (_cfg_wsrules != wsrules) {
		for (i = 0; i < num_wsrules; i++) {
			free(_cfg_wsrules[i].icondef);
			free(_cfg_wsrules[i].iconvac);
			free(_cfg_wsrules[i].iconocc);
		}
		free(_cfg_wsrules);
	}

	/* Cleanup layouts */
	if (_cfg_layouts != layouts) {
		for (i = 0; i < num_layouts; i++) {
			free(_cfg_layouts[i].name);
			free(_cfg_layouts[i].symbol);
		}
		free(_cfg_layouts);
	}

	/* Cleanup keybindings */
	if (_cfg_keys != keys) {
		free(_cfg_keys);
	}

	/* Cleanup buttons */
	if (_cfg_buttons != buttons) {
		free(_cfg_buttons);
	}

	/* Cleanup stacker icons */
	if (_cfg_stackericons != stackericons) {
		for (i = 0; i < num_stackericons; i++) {
			free(_cfg_stackericons[i].icon);
		}
		free(_cfg_stackericons);
	}

	/* Cleanup commands */
	if (_cfg_commands != NULL) {
		for (i = 0; i < num_commands; i++) {
			free(_cfg_commands[i].name);
			for (j = 0; _cfg_commands[i].argv[j] != NULL; j++) {
				free(_cfg_commands[i].argv[j]);
			}
		}
		free(_cfg_commands);
	}

	execv_free(_cfg_autostart);
	execv_free(_cfg_autorestart);

	/* Cleanup stray strings from config */
	for (i = 0; i < num_cached_strings; i++) {
		free(_cfg_cached_strings_array[i]);
	}
}

void
load_autostart(config_t *cfg)
{
	int i;
	const config_setting_t *auto_cfg;

	auto_cfg = config_lookup(cfg, "autostart");
	if (auto_cfg && config_setting_is_array(auto_cfg)) {
		num_autostart = config_setting_length(auto_cfg);
		if (num_autostart) {
			_cfg_autostart = ecalloc(num_autostart + 1, sizeof(char **));
			for (i = 0; i < num_autostart; i++) {
				_cfg_autostart[i] = parse_void_reference(config_setting_get_string_elem(auto_cfg, i));
			}
		}
	}

	auto_cfg = config_lookup(cfg, "autorestart");
	if (auto_cfg && config_setting_is_array(auto_cfg)) {
		num_autorestart = config_setting_length(auto_cfg);
		if (num_autorestart) {
			_cfg_autorestart = ecalloc(num_autorestart + 1, sizeof(char **));
			for (i = 0; i < num_autorestart; i++) {
				_cfg_autorestart[i] = parse_void_reference(config_setting_get_string_elem(auto_cfg, i));
			}
		}
	}
}

void
load_bar(config_t *cfg)
{
	int i;
	unsigned int alpha;
	const char *string;
	BarDef *bar;
	BarRule *rule;
	const config_setting_t *barconfig, *bar_t, *rules, *rule_t, *monitor, *scheme, *value, *align;

	config_lookup_sloppy_bool(cfg, "bar.showbar", &initshowbar);

	config_lookup_int(cfg, "bar.height", &bar_height);
	if (!config_lookup_int(cfg, "bar.vertpad", &vertpad))
		vertpad = borderpx;
	if (!config_lookup_int(cfg, "bar.sidepad", &sidepad))
		sidepad = borderpx;
	config_lookup_int(cfg, "bar.text_padding", &horizpadbar);
	config_lookup_int(cfg, "bar.height_padding", &vertpadbar);
	config_lookup_unsigned_int(cfg, "bar.systray_spacing", &systrayspacing);

	if (config_lookup_unsigned_int(cfg, "bar.alpha_fg", &alpha))
		default_alphas[ColFg] = alpha;
	if (config_lookup_unsigned_int(cfg, "bar.alpha_bg", &alpha))
		default_alphas[ColBg] = alpha;
	if (config_lookup_unsigned_int(cfg, "bar.alpha_border", &alpha))
		default_alphas[ColBorder] = alpha;

	/* Bars */
	barconfig = config_lookup(cfg, "bar.bars");
	if (barconfig && config_setting_is_list(barconfig)) {
		if ((num_bars = config_setting_length(barconfig))) {
			_cfg_bars = ecalloc(num_bars, sizeof(BarDef));
			for (i = 0; i < num_bars; i++) {
				bar = &_cfg_bars[i];
				bar->barpos = NULL;
				bar->extclass = NULL;
				bar->extinstance = NULL;
				bar->extname = NULL;
				bar->name = NULL;

				bar_t = config_setting_get_elem(barconfig, i);
				config_setting_lookup_int(bar_t, "monitor", &bar->monitor);
				config_setting_lookup_int(bar_t, "bar", &bar->idx);
				config_setting_lookup_int(bar_t, "vert", &bar->vert);
				config_setting_lookup_strdup(bar_t, "pos", &bar->barpos);
				config_setting_lookup_strdup(bar_t, "extclass", &bar->extclass);
				config_setting_lookup_strdup(bar_t, "extinst", &bar->extinstance);
				config_setting_lookup_strdup(bar_t, "extname", &bar->extname);
				config_setting_lookup_strdup(bar_t, "name", &bar->name);
			}
		}
	}

	/* Bar rules */
	rules = config_lookup(cfg, "bar.rules");
	if (!rules || !config_setting_is_list(rules))
		return;

	num_barrules = config_setting_length(rules);
	if (!num_barrules)
		return;

	_cfg_barrules = ecalloc(num_barrules, sizeof(BarRule));
	for (i = 0; i < num_barrules; i++) {
		rule = &_cfg_barrules[i];
		/* Default values */
		rule->monitor = -1;
		rule->bar = 0;
		rule->scheme = SchemeNorm;
		rule->lpad = 0;
		rule->rpad = 0;
		rule->value = 0;
		rule->alignment = 0;
		rule->name = NULL;

		rule_t = config_setting_get_elem(rules, i);
		if (!rule_t)
			continue;

		if ((monitor = config_setting_lookup(rule_t, "monitor"))) {
			switch (config_setting_type(monitor)) {
			case CONFIG_TYPE_INT:
				rule->monitor = config_setting_get_int(monitor);
				break;
			case CONFIG_TYPE_STRING:
				string = config_setting_get_string(monitor);
				if (!strcasecmp("A", string)) {
					rule->scheme = 'A';
				} else {
					rule->scheme = atoi(string);
				}
				break;
			}
		}

		config_setting_lookup_int(rule_t, "bar", &rule->bar);

		if ((scheme = config_setting_lookup(rule_t, "scheme"))) {
			switch (config_setting_type(scheme)) {
			case CONFIG_TYPE_INT:
				rule->scheme = config_setting_get_int(scheme);
				break;
			case CONFIG_TYPE_STRING:
				rule->scheme = parse_scheme(config_setting_get_string(scheme));
				break;
			}
		}

		config_setting_lookup_int(rule_t, "padding", &rule->lpad);
		config_setting_lookup_int(rule_t, "padding", &rule->rpad);
		config_setting_lookup_int(rule_t, "lpad", &rule->lpad);
		config_setting_lookup_int(rule_t, "rpad", &rule->rpad);

		if ((value = config_setting_lookup(rule_t, "value"))) {
			switch (config_setting_type(value)) {
			case CONFIG_TYPE_INT:
				rule->value = config_setting_get_int(value);
				break;
			case CONFIG_TYPE_STRING:
				rule->value = parse_bar_rule_value(config_setting_get_string(value));
				break;
			}
		}

		if ((align = config_setting_lookup(rule_t, "align"))) {
			switch (config_setting_type(align)) {
			case CONFIG_TYPE_INT:
				rule->alignment = config_setting_get_int(align);
				break;
			case CONFIG_TYPE_STRING:
				rule->alignment = parse_align(config_setting_get_string(align));
				break;
			}
		}

		config_setting_lookup_string(rule_t, "module", &string);
		parse_module(string, rule);

		config_setting_lookup_strdup(rule_t, "name", &rule->name);
	}
}

void
load_button_bindings(config_t *cfg)
{
	int i, j, k, length, num_clicks, num_modifiers, num_buttons, num_functions, num_arguments;
	int num_bindings, num_expanded_bindings, value;
	int click_arr[20] = {0};
	unsigned int modifier_arr[20] = {0};
	int button_arr[20] = {0};
	ArgFunc function_arr[20] = {0};
	const char *string;
	int argument_arr[20] = {0};
	void *void_argument_arr[20] = {0};
	float float_argument_arr[20] = {0};

	const config_setting_t *bindings, *binding, *click, *modifier, *button, *function, *argument, *arg;

	bindings = config_lookup(cfg, "button_bindings");
	if (!bindings || !config_setting_is_list(bindings))
		return;

	num_bindings = config_setting_length(bindings);
	if (!num_bindings)
		return;

	_cfg_buttons = ecalloc(MAX(num_bindings * 2, 60), sizeof(Button));

	/* Parse and set the button bindings based on config */
	for (i = 0; i < num_bindings; i++) {
		binding = config_setting_get_elem(bindings, i);
		click = config_setting_lookup(binding, "click");
		modifier = config_setting_lookup(binding, "modifier");
		button = config_setting_lookup(binding, "button");
		function = config_setting_lookup(binding, "function");
		argument = config_setting_lookup(binding, "argument");

		click_arr[0] = 0;
		modifier_arr[0] = 0;
		button_arr[0] = 1;
		function_arr[0] = NULL;
		argument_arr[0] = 0;
		float_argument_arr[0] = 0;
		void_argument_arr[0] = NULL;

		length = setting_length(click);
		num_clicks = MAX(length, 1);
		for (j = 0; j < length; j++) {
			click_arr[j] = parse_click(setting_get_string_elem(click, j));
		}

		length = setting_length(modifier);
		num_modifiers = MAX(length, 1);
		for (j = 0; j < length; j++) {
			modifier_arr[j] = parse_modifier(setting_get_string_elem(modifier, j));
		}

		length = setting_length(button);
		num_buttons = MAX(length, 1);
		for (j = 0; j < length; j++) {
			button_arr[j] = setting_get_int_elem(button, j);
		}

		length = setting_length(function);
		num_functions = MAX(length, 1);
		for (j = 0; j < length; j++) {
			function_arr[j] = parse_arg_function(setting_get_string_elem(function, j));
		}

		length = setting_length(argument);
		num_arguments = MAX(length, 1);
		for (j = 0; j < length; j++) {
			arg = setting_get_elem(argument, j);
			argument_arr[j] = 0;
			float_argument_arr[j] = 0;
			void_argument_arr[j] = NULL;
			switch (config_setting_type(arg)) {
			case CONFIG_TYPE_INT:
				argument_arr[j] = config_setting_get_int(arg);
				break;
			case CONFIG_TYPE_STRING:
				string = config_setting_get_string(arg);
				if (parse_arg_function_int_constant(string, function_arr[j % num_functions], &value)) {
					argument_arr[j] = value;
				} else {
					void_argument_arr[j] = parse_void_reference(string);
				}
				break;
			case CONFIG_TYPE_FLOAT:
				float_argument_arr[j] = config_setting_get_float(arg);
				break;
			}
		}

		/* Figure out the maximum number of expanded button bindings */
		int counts[5] = {num_clicks, num_modifiers, num_buttons, num_functions, num_arguments};
		num_expanded_bindings = counts[0];
		for (k = 1; k < 5; k++)
			if (counts[k] > num_expanded_bindings)
				num_expanded_bindings = counts[k];

		/* Finally add each button binding */
		for (j = 0; j < num_expanded_bindings; j++) {
			add_button_binding(
				click_arr[j % num_clicks],
				modifier_arr[j % num_modifiers],
				button_arr[j % num_buttons],
				function_arr[j % num_functions],
				argument_arr[j % num_arguments],
				void_argument_arr[j % num_arguments],
				float_argument_arr[j % num_arguments]
			);
		}
	}
}

void
add_button_binding(
	unsigned int click,
	unsigned int mask,
	unsigned int button,
	ArgFunc function,
	int argument,
	void *void_argument,
	float float_argument
) {
	_cfg_buttons[num_button_bindings].click = click;
	_cfg_buttons[num_button_bindings].mask = mask;
	_cfg_buttons[num_button_bindings].button = button;
	_cfg_buttons[num_button_bindings].func = function;
	if (void_argument != NULL) {
		_cfg_buttons[num_button_bindings].arg.v = void_argument;
	} else if (float_argument != 0) {
		_cfg_buttons[num_button_bindings].arg.f = float_argument;
	} else {
		_cfg_buttons[num_button_bindings].arg.i = argument;
	}

	num_button_bindings++;
}

void
load_clientrules(config_t *cfg)
{
	int i, f, num_flags;
	Rule *r;
	const char *string;
	const config_setting_t *rules, *rule, *flags;

	rules = config_lookup(cfg, "client_rules");
	if (!rules || !config_setting_is_list(rules))
		return;

	num_client_rules = config_setting_length(rules);
	if (!num_client_rules)
		return;

	_cfg_clientrules = ecalloc(num_client_rules, sizeof(Rule));

	for (i = 0; i < num_client_rules; i++) {

		r = &_cfg_clientrules[i];

		rule = config_setting_get_elem(rules, i);

		config_setting_lookup_strdup(rule, "class", &r->class);
		config_setting_lookup_strdup(rule, "role", &r->role);
		config_setting_lookup_strdup(rule, "instance", &r->instance);
		config_setting_lookup_strdup(rule, "title", &r->title);

		if (config_setting_lookup_string(rule, "wintype", &string))
			r->wintype = strdup(parse_window_type(string));

		config_setting_lookup_strdup(rule, "floatpos", &r->floatpos);
		config_setting_lookup_strdup(rule, "label", &r->label);
		config_setting_lookup_strdup(rule, "iconpath", &r->iconpath);
		config_setting_lookup_strdup(rule, "alttitle", &r->alttitle);
		config_setting_lookup_strdup(rule, "workspace", &r->workspace);

		if (config_setting_lookup_string(rule, "scratchkey", &string))
			r->scratchkey = string[0];
		if (config_setting_lookup_string(rule, "swallowedby", &string))
			r->swallowedby = string[0];
		if (config_setting_lookup_string(rule, "swallowkey", &string))
			r->swallowkey = string[0];

		config_setting_lookup_int(rule, "transient", &r->transient);
		config_setting_lookup_int(rule, "resume", &r->resume);
		config_setting_lookup_float(rule, "opacity", &r->opacity);

		flags = config_setting_lookup(rule, "flags");
		num_flags = setting_length(flags);
		for (f = 0; f < num_flags; f++) {
			r->flags |= getflagbyname(setting_get_string_elem(flags, f));
		}
	}
}

void
load_colors(config_t *cfg)
{
	int i, j, num_cols, scheme;
	const config_setting_t *cols, *col;

	cols = config_lookup(cfg, "colors");
	if (!cols || !config_setting_is_group(cols))
		return;

	num_cols = config_setting_length(cols);
	if (!num_cols)
		return;

	/* Initialise the colors array */
	_cfg_colors = ecalloc(SchemeLast, sizeof(char **));
	for (i = 0; i < SchemeLast; i++) {
		_cfg_colors[i] = ecalloc((ColCount+1), sizeof(char *));
		for (j = 0; j <= ColCount; j++) {
			_cfg_colors[i][j] = NULL;
		}
	}

	/* Parse and set the colors based on config */
	for (i = 0; i < num_cols; i++) {
		col = config_setting_get_elem(cols, i);
		scheme = parse_scheme(config_setting_name(col));

		config_setting_lookup_strdup(col, "fg", &_cfg_colors[scheme][ColFg]);
		config_setting_lookup_strdup(col, "bg", &_cfg_colors[scheme][ColBg]);
		config_setting_lookup_strdup(col, "border", &_cfg_colors[scheme][ColBorder]);
		config_setting_lookup_strdup(col, "resource", &_cfg_colors[scheme][ColCount]);
	}
}

void
load_commands(config_t *cfg)
{
	int i, j, num_cmd_elements;
	const config_setting_t *commands_list, *command_entry, *command_t;
	Command *command;

	commands_list = config_lookup(cfg, "commands");
	if (!commands_list || !config_setting_is_list(commands_list))
		return;

	num_commands = config_setting_length(commands_list);
	if (!num_commands)
		return;

	_cfg_commands = ecalloc(num_commands, sizeof(Command));
	for (i = 0; i < num_commands; i++) {
		command = &_cfg_commands[i];
		command->name = NULL;
		command->argv = NULL;

		command_entry = config_setting_get_elem(commands_list, i);
		config_setting_lookup_strdup(command_entry, "name", &command->name);

		command_t = config_setting_lookup(command_entry, "command");
		num_cmd_elements = config_setting_length(command_t);
		command->argv = ecalloc(num_cmd_elements + 2, sizeof(char*));

		if (!config_setting_lookup_strdup(command_entry, "scratchkey", &command->argv[0])) {
			command->argv[0] = NULL;
		}

		for (j = 0; j < num_cmd_elements; j++) {
			command->argv[j + 1] = strdup(config_setting_get_string_elem(command_t, j));
		}
		command->argv[j + 1] = NULL;

		if (command->name == NULL || command->argv == NULL) {
			fprintf(stderr, "Warning: config found incomplete command at line %d\n", config_setting_source_line(command_entry));
		}
	}
}

void
load_keybindings(config_t *cfg)
{
	int i, j, k, length, keytype, num_modifiers, num_keys, num_functions, num_arguments;
	int num_bindings, num_expanded_bindings, value;
	unsigned int modifier_arr[20] = {0};
	#if USE_KEYCODES
	KeyCode key_arr[20] = {0};
	#else
	KeySym key_arr[20] = {0};
	#endif // USE_KEYCODES
	ArgFunc function_arr[20] = {0};
	const char *string;
	int argument_arr[20] = {0};
	void *void_argument_arr[20] = {0};
	float float_argument_arr[20] = {0};

	const config_setting_t *bindings, *binding, *type, *modifier, *key, *function, *argument, *arg;

	bindings = config_lookup(cfg, "keybindings");
	if (!bindings || !config_setting_is_list(bindings))
		return;

	num_bindings = config_setting_length(bindings);
	if (!num_bindings)
		return;

	_cfg_keys = ecalloc(MAX(num_bindings * 2, 200), sizeof(Key));

	/* Parse and set the key bindings based on config */
	for (i = 0; i < num_bindings; i++) {
		binding = config_setting_get_elem(bindings, i);
		type = config_setting_lookup(binding, "type");
		modifier = config_setting_lookup(binding, "modifier");
		key = config_setting_lookup(binding, "key");
		function = config_setting_lookup(binding, "function");
		argument = config_setting_lookup(binding, "argument");

		keytype = (type ? parse_key_type(config_setting_get_string(type)) : KeyPress);

		modifier_arr[0] = 0;
		function_arr[0] = NULL;
		argument_arr[0] = 0;
		float_argument_arr[0] = 0;
		void_argument_arr[0] = NULL;
		key_arr[0] = 0;

		length = setting_length(modifier);
		num_modifiers = MAX(length, 1);
		for (j = 0; j < length; j++) {
			modifier_arr[j] = parse_modifier(setting_get_string_elem(modifier, j));
		}

		length = setting_length(function);
		num_functions = MAX(length, 1);
		for (j = 0; j < length; j++) {
			function_arr[j] = parse_arg_function(setting_get_string_elem(function, j));
		}

		length = setting_length(argument);
		num_arguments = MAX(length, 1);
		for (j = 0; j < length; j++) {
			arg = setting_get_elem(argument, j);
			argument_arr[j] = 0;
			float_argument_arr[j] = 0;
			void_argument_arr[j] = NULL;
			switch (config_setting_type(arg)) {
			case CONFIG_TYPE_INT:
				argument_arr[j] = config_setting_get_int(arg);
				break;
			case CONFIG_TYPE_STRING:
				string = config_setting_get_string(arg);
				if (parse_arg_function_int_constant(string, function_arr[j % num_functions], &value)) {
					argument_arr[j] = value;
				} else {
					void_argument_arr[j] = parse_void_reference(string);
				}
				break;
			case CONFIG_TYPE_FLOAT:
				float_argument_arr[j] = config_setting_get_float(arg);
				break;
			}
		}

		length = setting_length(key);
		num_keys = MAX(length, 1);
		for (j = 0; j < length; j++) {
			string = setting_get_string_elem(key, j);

			if (function_arr[j % num_functions] == stackfocus) {
				add_stacker_icon(cfg, string, argument_arr[j % num_arguments]);
			}

			#if USE_KEYCODES
			KeySym ks = XStringToKeysym(string);
			if (key_arr[j] == NoSymbol)
				fprintf(stderr, "Warning: config could not look up keycode for key %s\n", string);
			key_arr[j] = XKeysymToKeycode(dpy, ks);
			#else
			key_arr[j] = XStringToKeysym(string);
			if (key_arr[j] == NoSymbol)
				fprintf(stderr, "Warning: config could not look up keysym for key %s\n", string);
			#endif // USE_KEYCODES
		}

		/* Figure out the maximum number of expanded keybindings */
		int counts[4] = {num_modifiers, num_functions, num_arguments, num_keys};
		num_expanded_bindings = counts[0];
		for (k = 1; k < 4; k++)
			if (counts[k] > num_expanded_bindings)
				num_expanded_bindings = counts[k];

		/* Finally add each keybinding */
		for (j = 0; j < num_expanded_bindings; j++) {
			add_key_binding(
				keytype,
				modifier_arr[j % num_modifiers],
				key_arr[j % num_keys],
				function_arr[j % num_functions],
				argument_arr[j % num_arguments],
				void_argument_arr[j % num_arguments],
				float_argument_arr[j % num_arguments]
			);
		}
	}
}

void
add_key_binding(
	int type,
	unsigned int mod,
	#if USE_KEYCODES
	KeyCode keycode,
	#else
	KeySym keysym,
	#endif // USE_KEYCODES
	ArgFunc function,
	int argument,
	void *void_argument,
	float float_argument
) {
	_cfg_keys[num_key_bindings].type = type;
	_cfg_keys[num_key_bindings].mod = mod;
	#if USE_KEYCODES
	_cfg_keys[num_key_bindings].keycode = keycode;
	#else
	_cfg_keys[num_key_bindings].keysym = keysym;
	#endif // USE_KEYCODES
	_cfg_keys[num_key_bindings].func = function;
	if (void_argument != NULL) {
		_cfg_keys[num_key_bindings].arg.v = void_argument;
	} else if (float_argument != 0) {
		_cfg_keys[num_key_bindings].arg.f = float_argument;
	} else {
		_cfg_keys[num_key_bindings].arg.i = argument;
	}

	num_key_bindings++;
}

void
add_stacker_icon(config_t *cfg, const char *string, int value)
{
	const char *prefix, *suffix, *pos_string, *icon_char, *replace_str;
	int i, num_overrides, overridden = 0;
	int position = StackerTitlePrefix;
	const config_setting_t *stacker_cfg, *overrides, *override;

	if (_cfg_stackericons == NULL) {
		_cfg_stackericons = ecalloc(30, sizeof(StackerIcon));
	}

	stacker_cfg = config_lookup(cfg, "stacker_icons");
	if (stacker_cfg) {
		if (!config_setting_lookup_string(stacker_cfg, "prefix", &prefix))
			prefix = "";
		if (!config_setting_lookup_string(stacker_cfg, "suffix", &suffix))
			suffix = "";
		if (config_setting_lookup_string(stacker_cfg, "position", &pos_string))
			position = parse_stacker_placement(pos_string);

		overrides = config_setting_lookup(stacker_cfg, "overrides");
		if (overrides && config_setting_is_list(overrides)) {
			num_overrides = config_setting_length(overrides);
			for (i = 0; i < num_overrides; i++) {
				override = config_setting_get_elem(overrides, i);
				if (!config_setting_lookup_string(override, "replace", &replace_str))
					continue;

				if (strcmp(replace_str, string))
					continue;

				if (!config_setting_lookup_string(override, "with", &replace_str))
					continue;

				icon_char = replace_str;
				overridden = 1;
				break;
			}
		}

		if (!overridden) {
			icon_char = string;
		}
	} else {
		prefix = "";
		suffix = "";
		icon_char = string;
	}

	char *icon = NULL;
	freesprintf(&icon, "%s%s%s", prefix, icon_char, suffix);

	_cfg_stackericons[num_stackericons].icon = icon;
	_cfg_stackericons[num_stackericons].arg.i = value;
	_cfg_stackericons[num_stackericons].pos = position;

	num_stackericons++;
}

void
load_fonts(config_t *cfg)
{
	int i, num_fonts;
	const config_setting_t *fonts;

	fonts = config_lookup(cfg, "fonts");
	if (!fonts)
		return;

	num_fonts = config_setting_length(fonts);

	for (i = 0; i < num_fonts; i++) {
		drw_font_add(drw, config_setting_get_string_elem(fonts, i));
	}
}

void
load_singles(config_t *cfg)
{
	const char *string;

	config_lookup_unsigned_int(cfg, "borderpx", &borderpx);
	config_lookup_unsigned_int(cfg, "gaps.ih", &gappih);
	config_lookup_unsigned_int(cfg, "gaps.iv", &gappiv);
	config_lookup_unsigned_int(cfg, "gaps.oh", &gappoh);
	config_lookup_unsigned_int(cfg, "gaps.ov", &gappov);
	config_lookup_unsigned_int(cfg, "gaps.fl", &gappfl);
	config_lookup_sloppy_bool(cfg, "gaps.enabled", &enablegaps);
	config_lookup_unsigned_int(cfg, "gaps.smartgaps_fact", &smartgaps_fact);

	config_lookup_int(cfg, "nmaster", &nmaster);
	config_lookup_int(cfg, "nstack", &nstack);
	config_lookup_simple_float(cfg, "mfact", &mfact);

	if (config_lookup_string(cfg, "attach_method", &string)) {
		uint64_t flag = getflagbyname(string) & AttachFlag;

		if (flag) {
			attachdefault = flag;
		}
	}

	config_lookup_strdup(cfg, "slop.spawn_style", &_cfg_slopspawnstyle);
	config_lookup_strdup(cfg, "slop.resize_style", &_cfg_slopresizestyle);

	/* floatpos settings */
	config_lookup_int(cfg, "floatpos.grid_x", &floatposgrid_x);
	config_lookup_int(cfg, "floatpos.grid_y", &floatposgrid_y);
	config_lookup_strdup(cfg, "floatpos.toggle_position", &_cfg_toggle_float_pos);

	config_lookup_float(cfg, "opacity.default", &defaultopacity);
	config_lookup_float(cfg, "opacity.move", &moveopacity);
	config_lookup_float(cfg, "opacity.resize", &resizeopacity);
	config_lookup_float(cfg, "opacity.place", &placeopacity);

	/* flexwintitle settings */
	config_lookup_int(cfg, "flexwintitle.masterweight", &flexwintitle_masterweight);
	config_lookup_int(cfg, "flexwintitle.stackweight", &flexwintitle_stackweight);
	config_lookup_int(cfg, "flexwintitle.hiddenweight", &flexwintitle_hiddenweight);
	config_lookup_int(cfg, "flexwintitle.floatweight", &flexwintitle_floatweight);
	config_lookup_int(cfg, "flexwintitle.separator", &flexwintitle_separator);
	config_lookup_int(cfg, "flexwintitle.icon.size", &iconsize);
	config_lookup_int(cfg, "flexwintitle.icon.spacing", &iconspacing);

	/* restart settings */
	config_lookup_sloppy_bool(cfg, "restart.persist_client_states_across_restarts", &persist_client_states_across_restarts);
	config_lookup_sloppy_bool(cfg, "restart.persist_workstate_states_across_restarts", &persist_workstate_states_across_restarts);
}

void
load_refresh_rates(config_t *cfg)
{
	config_lookup_int(cfg, "refresh_rates.global_hz", &global_hz);
	if (!config_lookup_int(cfg, "refresh_rates.dragcfact_hz", &dragcfact_hz))
		dragcfact_hz = global_hz;
	if (!config_lookup_int(cfg, "refresh_rates.dragfact_hz", &dragfact_hz))
		dragfact_hz = global_hz;
	if (!config_lookup_int(cfg, "refresh_rates.dragmfact_hz", &dragmfact_hz))
		dragmfact_hz = global_hz;
	if (!config_lookup_int(cfg, "refresh_rates.dragwfact_hz", &dragwfact_hz))
		dragwfact_hz = global_hz;
	if (!config_lookup_int(cfg, "refresh_rates.movemouse_hz", &movemouse_hz))
		movemouse_hz = global_hz;
	if (!config_lookup_int(cfg, "refresh_rates.placemouse_hz", &placemouse_hz))
		placemouse_hz = global_hz;
	if (!config_lookup_int(cfg, "refresh_rates.resizemouse_hz", &resizemouse_hz))
		resizemouse_hz = global_hz;
	if (!config_lookup_int(cfg, "refresh_rates.swallowmouse_hz", &swallowmouse_hz))
		swallowmouse_hz = global_hz;
	if (!config_lookup_int(cfg, "refresh_rates.markmouse_hz", &markmouse_hz))
		markmouse_hz = global_hz;
}

void
load_workspace(config_t *cfg)
{
	const config_setting_t *rules, *rule_t, *icons, *layout;
	WorkspaceRule *rule;
	int i;

	config_lookup_simple_float(cfg, "workspace.preview_factor", &pfact);
	config_lookup_strdup(cfg, "workspace.labels.occupied_format", &_cfg_occupied_workspace_label_format);
	config_lookup_strdup(cfg, "workspace.labels.vacant_format", &_cfg_vacant_workspace_label_format);
	config_lookup_sloppy_bool(cfg, "workspace.labels.lower_case", &lowercase_workspace_labels);
	config_lookup_sloppy_bool(cfg, "workspace.labels.prefer_window_icons", &prefer_window_icons_over_workspace_labels);
	config_lookup_sloppy_bool(cfg, "workspace.labels.swap_occupied_format", &swap_occupied_workspace_label_format_strings);
	config_lookup_sloppy_bool(cfg, "workspace.per_monitor", &workspaces_per_mon);

	/* Workspace rules */
	rules = config_lookup(cfg, "workspace.rules");
	if (!rules || !config_setting_is_list(rules))
		return;

	num_wsrules = config_setting_length(rules);
	if (!num_wsrules)
		return;

	_cfg_wsrules = ecalloc(num_wsrules, sizeof(WorkspaceRule));
	for (i = 0; i < num_wsrules; i++) {
		rule = &_cfg_wsrules[i];

		/* Default values */
		rule->norm_scheme = SchemeWsNorm;
		rule->vis_scheme = SchemeWsVisible;
		rule->sel_scheme = SchemeWsSel;
		rule->occ_scheme = SchemeWsOcc;
		rule->monitor = -1;
		rule->pinned = 0;
		rule->layout = 0;
		rule->mfact = -1;
		rule->nmaster = -1;
		rule->nstack = -1;
		rule->enablegaps = -1;
		rule->name = NULL;
		rule->icondef = NULL;
		rule->iconvac = NULL;
		rule->iconocc = NULL;

		rule_t = config_setting_get_elem(rules, i);
		if (!rule_t)
			continue;

		config_setting_lookup_strdup(rule_t, "name", &rule->name);
		config_setting_lookup_sloppy_bool(rule_t, "pinned", &rule->pinned);

		/* Allow layout to be referred to by name as well as index */
		if ((layout = config_setting_lookup(rule_t, "layout"))) {
			switch (config_setting_type(layout)) {
			case CONFIG_TYPE_INT:
				rule->layout = config_setting_get_int(layout);
				break;
			case CONFIG_TYPE_STRING:
				rule->layout = parse_layout(config_setting_get_string(layout));
				break;
			}
		}

		config_setting_lookup_int(rule_t, "monitor", &rule->monitor);
		config_setting_lookup_simple_float(rule_t, "mfact", &rule->mfact);
		config_setting_lookup_int(rule_t, "nmaster", &rule->nmaster);
		config_setting_lookup_int(rule_t, "nstack", &rule->nstack);
		config_setting_lookup_sloppy_bool(rule_t, "gaps", &rule->enablegaps);

		icons = config_setting_lookup(rule_t, "icons");
		if (icons) {
			if (!config_setting_lookup_strdup(icons, "def", &rule->icondef)) {
				rule->icondef = strdup("◉");
			}

			if (!config_setting_lookup_strdup(icons, "vac", &rule->iconvac)) {
				rule->iconvac = rule->icondef;
			}

			if (!config_setting_lookup_strdup(icons, "occ", &rule->iconocc)) {
				rule->iconocc = rule->icondef;
			}
		} else {
			rule->icondef = strdup("◉");
			rule->iconocc = strdup("●");
		}
	}
}

#define readfunc(F) if (config_lookup_sloppy_bool(cfg, "functionality." #F, &enabled)) { if (enabled) { enablefunc(F); } else { disablefunc(F); } }

void
load_functionality(config_t *cfg)
{
	int i, enabled;

	const config_setting_t *func_t = config_lookup(cfg, "functionality");
	if (!func_t)
		return;

	for (i = 0; functionality_names[i].name != NULL; i++) {
		if (config_setting_lookup_sloppy_bool(func_t, functionality_names[i].name, &enabled)) {
			setenabled(functionality_names[i].value, enabled);
		}
	}
}

#undef readfunc

void
load_layouts(config_t *cfg)
{
	int i;
	const char *string;
	const config_setting_t *lts, *lt;
	Layout *layout;

	/* Layouts */
	lts = config_lookup(cfg, "layouts");
	if (lts && config_setting_is_list(lts)) {
		num_layouts = config_setting_length(lts);
	}

	if (!num_layouts) {
		_cfg_layouts = layouts;
		num_layouts = LENGTH(layouts);
		return;
	}

	_cfg_layouts = ecalloc(num_layouts, sizeof(Layout));
	for (i = 0; i < num_layouts; i++) {
		layout = &_cfg_layouts[i];
		layout->preset.nmaster = -1;
		layout->preset.nstack = -1;
		layout->preset.layout = NO_SPLIT;
		layout->preset.masteraxis = 0;
		layout->preset.stack1axis = 0;
		layout->preset.stack2axis = 0;
		layout->preset.symbolfunc = NULL;
		layout->arrange = NULL;

		lt = config_setting_get_elem(lts, i);
		config_setting_lookup_int(lt, "nmaster", &layout->preset.nmaster);
		config_setting_lookup_int(lt, "nstack", &layout->preset.nstack);
		config_setting_lookup_strdup(lt, "symbol", &layout->symbol);
		config_setting_lookup_strdup(lt, "name", &layout->name);

		if (config_setting_lookup_string(lt, "split", &string))
			layout->preset.layout = parse_layout_split(string);
		if (config_setting_lookup_string(lt, "master", &string)) {
			layout->arrange = flextile;
			layout->preset.masteraxis = parse_layout_arrangement(string);
		}
		if (config_setting_lookup_string(lt, "stack", &string))
			layout->preset.stack1axis = parse_layout_arrangement(string);
		if (config_setting_lookup_string(lt, "stack2", &string))
			layout->preset.stack2axis = parse_layout_arrangement(string);
		if (config_setting_lookup_string(lt, "function", &string))
			layout->arrange = parse_layout_function(string);
		if (config_setting_lookup_string(lt, "symbolfunc", &string))
			layout->preset.symbolfunc = parse_symbol_function(string);
	}
}

void
load_indicators(config_t *cfg)
{
	const char *indicator;
	if (config_lookup_string(cfg, "indicator.WsOcc", &indicator))
		indicators[IndicatorWsOcc] = parse_indicator(indicator);
	if (config_lookup_string(cfg, "indicator.WsSel", &indicator))
		indicators[IndicatorWsSel] = parse_indicator(indicator);
	if (config_lookup_string(cfg, "indicator.WsVis", &indicator))
		indicators[IndicatorWsVis] = parse_indicator(indicator);
	if (config_lookup_string(cfg, "indicator.WsNorm", &indicator))
		indicators[IndicatorWsNorm] = parse_indicator(indicator);
	if (config_lookup_string(cfg, "indicator.PinnedWs", &indicator))
		indicators[IndicatorPinnedWs] = parse_indicator(indicator);
	if (config_lookup_string(cfg, "indicator.FakeFullScreen", &indicator))
		indicators[IndicatorFakeFullScreen] = parse_indicator(indicator);
	if (config_lookup_string(cfg, "indicator.FakeFullScreenActive", &indicator))
		indicators[IndicatorFakeFullScreenActive] = parse_indicator(indicator);
	if (config_lookup_string(cfg, "indicator.FloatFakeFullScreen", &indicator))
		indicators[IndicatorFloatFakeFullScreen] = parse_indicator(indicator);
	if (config_lookup_string(cfg, "indicator.FloatFakeFullScreenActive", &indicator))
		indicators[IndicatorFloatFakeFullScreenActive] = parse_indicator(indicator);
	if (config_lookup_string(cfg, "indicator.Tiled", &indicator))
		indicators[IndicatorTiled] = parse_indicator(indicator);
	if (config_lookup_string(cfg, "indicator.Floating", &indicator))
		indicators[IndicatorFloating] = parse_indicator(indicator);
	if (config_lookup_string(cfg, "indicator.Selected", &indicator))
		indicators[IndicatorSelected] = parse_indicator(indicator);

	config_lookup_strdup(cfg, "indicator.custom.custom_1", &_cfg_custom_2d_indicator_1);
	config_lookup_strdup(cfg, "indicator.custom.custom_2", &_cfg_custom_2d_indicator_2);
	config_lookup_strdup(cfg, "indicator.custom.custom_3", &_cfg_custom_2d_indicator_3);
	config_lookup_strdup(cfg, "indicator.custom.custom_4", &_cfg_custom_2d_indicator_4);
	config_lookup_strdup(cfg, "indicator.custom.custom_5", &_cfg_custom_2d_indicator_5);
	config_lookup_strdup(cfg, "indicator.custom.custom_6", &_cfg_custom_2d_indicator_6);
}

void *
cfg_get_command(const char *string)
{
	int i;

	for (i = 0; i < num_commands; i++) {
		if (!strcasecmp(string, _cfg_commands[i].name)) {
			return _cfg_commands[i].argv;
		}
	}

	return NULL;
}

#define map(S, I) if (!strcasecmp(string, S)) return I;

int
parse_align(const char *string)
{
	if (!strncasecmp(string, "BAR_ALIGN_", 10))
		string += 10;

	map("NONE", BAR_ALIGN_NONE);
	map("TOP", BAR_ALIGN_TOP);
	map("LEFT", BAR_ALIGN_LEFT);
	map("CENTER", BAR_ALIGN_CENTER);
	map("BOTTOM", BAR_ALIGN_BOTTOM);
	map("RIGHT", BAR_ALIGN_RIGHT);
	map("TOP_TOP", BAR_ALIGN_TOP_TOP);
	map("TOP_CENTER", BAR_ALIGN_TOP_CENTER);
	map("TOP_BOTTOM", BAR_ALIGN_TOP_BOTTOM);
	map("LEFT_LEFT", BAR_ALIGN_LEFT_LEFT);
	map("LEFT_RIGHT", BAR_ALIGN_LEFT_RIGHT);
	map("LEFT_CENTER", BAR_ALIGN_LEFT_CENTER);
	map("RIGHT_LEFT", BAR_ALIGN_RIGHT_LEFT);
	map("RIGHT_RIGHT", BAR_ALIGN_RIGHT_RIGHT);
	map("RIGHT_CENTER", BAR_ALIGN_RIGHT_CENTER);
	map("BOTTOM_BOTTOM", BAR_ALIGN_BOTTOM_BOTTOM);
	map("BOTTOM_CENTER", BAR_ALIGN_BOTTOM_CENTER);
	map("BOTTOM_TOP", BAR_ALIGN_BOTTOM_TOP);

	return BAR_ALIGN_NONE;
}

int
parse_click(const char *string)
{
	if (!strncasecmp(string, "Clk", 3))
		string += 3;

	map("LtSymbol", ClkLtSymbol);
	map("WinTitle", ClkWinTitle);
	map("StatusText", ClkStatusText);
	map("ClientWin", ClkClientWin);
	map("RootWin", ClkRootWin);
	map("WorkspaceBar", ClkWorkspaceBar);

	return 0;
}

ArgFunc
parse_arg_function(const char *string)
{
	/* Add custom arg function mappings below */
	map("changeopacity", changeopacity);
	map("clienttomon", clienttomon);
	map("clientstomon", clientstomon);
	map("comboviewwsbyname", comboviewwsbyname);
	map("cyclelayout", cyclelayout);
	map("dragmfact", dragmfact);
	map("dragwfact", dragwfact);
	map("enable", enable);
	map("enablews", enablews);
	map("enablewsbyindex", enablewsbyindex);
	map("enablewsbyname", enablewsbyname);
	map("defaultgaps", defaultgaps);
	map("disable", disable);
	map("floatpos", floatpos);
	map("focusdir", focusdir);
	map("focushidden", focushidden);
	map("focusmaster", focusmaster);
	map("focusmon", focusmon);
	map("focusstack", focusstack);
	map("focusurgent", focusurgent);
	map("focuswin", focuswin);
	map("group", group);
	map("hidebar", hidebar);
	map("incrgaps", incrgaps);
	map("incrigaps", incrigaps);
	map("incrogaps", incrogaps);
	map("incrihgaps", incrihgaps);
	map("incrivgaps", incrivgaps);
	map("incrohgaps", incrohgaps);
	map("incrovgaps", incrovgaps);
	map("incnmaster", incnmaster);
	map("incnstack", incnstack);
	map("inplacerotate", inplacerotate);
	map("killclient", killclient);
	map("killunsel", killunsel);
	map("layoutconvert", layoutconvert);
	map("mark", mark);
	map("markall", markall);
	map("markmouse", markmouse);
	map("maximize", maximize);
	map("maximizevert", maximizevert);
	map("maximizehorz", maximizehorz);
	map("mirrorlayout", mirrorlayout);
	map("moveorplace", moveorplace);
	map("movemouse", movemouse);
	map("movetowsbyindex", movetowsbyindex);
	map("movetowsbyname", movetowsbyname);
	map("sendtowsbyindex", sendtowsbyindex);
	map("sendtowsbyname", sendtowsbyname);
	map("movealltowsbyindex", movealltowsbyindex);
	map("movealltowsbyname", movealltowsbyname);
	map("moveallfromwsbyindex", moveallfromwsbyindex);
	map("moveallfromwsbyname", moveallfromwsbyname);
	map("movews", movews);
	map("movewsdir", movewsdir);
	map("rotatelayoutaxis", rotatelayoutaxis);
	map("rotatestack", rotatestack);
	map("placedir", placedir);
	map("pushdown", pushdown);
	map("pushup", pushup);
	map("quit", quit);
	map("removescratch", removescratch);
	map("restart", restart);
	map("resizemouse", resizemouse);
	map("resizeorfacts", resizeorfacts);
	map("rioresize", rioresize);
	map("riospawn", riospawn);
	map("setattachdefault", setattachdefault);
	map("setborderpx", setborderpx);
	map("setclientborderpx", setclientborderpx);
	map("setlayout", setlayout);
	map("setlayoutaxisex", setlayoutaxisex);
	map("setcfact", setcfact);
	map("setmfact", setmfact);
	map("setwfact", setwfact);
	map("setgapsex", setgapsex);
	map("setscratch", setscratch);
	map("settitle", settitle);
	map("showbar", showbar);
	map("showhideclient", showhideclient);
	map("spawn", spawn);
	map("stackfocus", stackfocus);
	map("stackpush", stackpush);
	map("stackswap", stackswap);
	map("statusclick", statusclick);
	map("swallow", swallow);
	map("switchcol", switchcol);
	map("swapws", swapws);
	map("swapwsbyindex", swapwsbyindex);
	map("swapwsbyname", swapwsbyname);
	map("swallowmouse", swallowmouse);
	map("toggle", toggle);
	map("togglebar", togglebar);
	map("togglebarpadding", togglebarpadding);
	map("togglecompact", togglecompact);
	map("toggleclientflag", toggleclientflag);
	map("togglefakefullscreen", togglefakefullscreen);
	map("togglefloating", togglefloating);
	map("togglefullscreen", togglefullscreen);
	map("togglegaps", togglegaps);
	map("togglekeybindings", togglekeybindings);
	map("togglemark", togglemark);
	map("togglemoveorplace", togglemoveorplace);
	map("togglenomodbuttons", togglenomodbuttons);
	map("togglepinnedws", togglepinnedws);
	map("togglescratch", togglescratch);
	map("togglesticky", togglesticky);
	map("togglews", togglews);
	map("transfer", transfer);
	map("transferall", transferall);
	map("unfloatvisible", unfloatvisible);
	map("ungroup", ungroup);
	map("unhideall", unhideall);
	map("unhidepop", unhidepop);
	map("unmark", unmark);
	map("unmarkall", unmarkall);
	map("unswallow", unswallow);
	map("viewallwsonmon", viewallwsonmon);
	map("viewalloccwsonmon", viewalloccwsonmon);
	map("viewselws", viewselws);
	map("viewws", viewws);
	map("viewwsbyindex", viewwsbyindex);
	map("viewwsbyname", viewwsbyname);
	map("viewwsdir", viewwsdir);
	map("xrdb", xrdb);
	map("zoom", zoom);

	fprintf(stderr, "Warning: config could not find arg function with name %s\n", string);
	return NULL;
}

WsFunc
parse_layout_function(const char *string)
{
	/* Add custom layout function mappings below */
	map("flextile", flextile);
	map("NULL", NULL);

	fprintf(stderr, "Warning: config could not find layout function with name %s\n", string);
	return NULL;
}

SymbolFunc
parse_symbol_function(const char *string)
{
	/* Add custom symbol function mappings below */
	map("monoclesymbols", monoclesymbols);
	map("decksymbols", decksymbols);

	fprintf(stderr, "Warning: config could not find symbol function with name %s\n", string);
	return NULL;
}

int
parse_indicator(const char *string)
{
	map("NONE", INDICATOR_NONE);
	map("TOP_LEFT_SQUARE", INDICATOR_TOP_LEFT_SQUARE);
	map("TOP_LEFT_LARGER_SQUARE", INDICATOR_TOP_LEFT_LARGER_SQUARE);
	map("TOP_RIGHT_TRIANGLE", INDICATOR_TOP_RIGHT_TRIANGLE);
	map("TOP_RIGHT_PIN", INDICATOR_TOP_RIGHT_PIN);
	map("TOP_BAR", INDICATOR_TOP_BAR);
	map("TOP_BAR_SLIM", INDICATOR_TOP_BAR_SLIM);
	map("TOP_CENTERED_DOT", INDICATOR_TOP_CENTERED_DOT);
	map("BOTTOM_BAR", INDICATOR_BOTTOM_BAR);
	map("BOTTOM_BAR_SLIM", INDICATOR_BOTTOM_BAR_SLIM);
	map("BOTTOM_BAR_SLIM_DOTS", INDICATOR_BOTTOM_BAR_SLIM_DOTS);
	map("BOTTOM_CENTERED_DOT", INDICATOR_BOTTOM_CENTERED_DOT);
	map("BOX", INDICATOR_BOX);
	map("BOX_WIDER", INDICATOR_BOX_WIDER);
	map("BOX_FULL", INDICATOR_BOX_FULL);
	map("CLIENT_DOTS", INDICATOR_CLIENT_DOTS);
	map("PLUS", INDICATOR_PLUS);
	map("PLUS_AND_SQUARE", INDICATOR_PLUS_AND_SQUARE);
	map("PLUS_AND_LARGER_SQUARE", INDICATOR_PLUS_AND_LARGER_SQUARE);
	map("CUSTOM_1", INDICATOR_CUSTOM_1);
	map("CUSTOM_2", INDICATOR_CUSTOM_2);
	map("CUSTOM_3", INDICATOR_CUSTOM_3);
	map("CUSTOM_4", INDICATOR_CUSTOM_4);
	map("CUSTOM_5", INDICATOR_CUSTOM_5);
	map("CUSTOM_6", INDICATOR_CUSTOM_6);

	fprintf(stderr, "Warning: config could not find indicator with name %s\n", string);
	return INDICATOR_NONE;
}

int
parse_key_type(const char *string)
{
	if (!strncasecmp(string, "Key", 3))
		string += 3;

	map("Press", KeyPress);
	map("Release", KeyRelease);

	fprintf(stderr, "Warning: config could not find key type with name %s\n", string);
	return KeyPress;
}

/* Look up the layout index based on the layout name */
int
parse_layout(const char *string)
{
	int i;
	for (i = 0; i < num_layouts; i++) {
		if (!strcasecmp(string, _cfg_layouts[i].name)) {
			return i;
		}
	}

	return 0;
}

int
parse_layout_split(const char *string)
{
	if (!strncasecmp(string, "SPLIT_", 6))
		string += 6;

	map("VERTICAL", SPLIT_VERTICAL);
	map("HORIZONTAL", SPLIT_HORIZONTAL);
	map("NO_SPLIT", NO_SPLIT);
	map("CENTERED_VERTICAL", SPLIT_CENTERED_VERTICAL);
	map("CENTERED_HORIZONTAL", SPLIT_CENTERED_HORIZONTAL);
	map("VERTICAL_DUAL_STACK", SPLIT_VERTICAL_DUAL_STACK);
	map("HORIZONTAL_DUAL_STACK", SPLIT_HORIZONTAL_DUAL_STACK);
	map("VERTICAL_FIXED", SPLIT_VERTICAL_FIXED);
	map("HORIZONTAL_FIXED", SPLIT_HORIZONTAL_FIXED);
	map("CENTERED_VERTICAL_FIXED", SPLIT_CENTERED_VERTICAL_FIXED);
	map("CENTERED_HORIZONTAL_FIXED", SPLIT_CENTERED_HORIZONTAL_FIXED);
	map("VERTICAL_DUAL_STACK_FIXED", SPLIT_VERTICAL_DUAL_STACK_FIXED);
	map("HORIZONTAL_DUAL_STACK_FIXED", SPLIT_HORIZONTAL_DUAL_STACK_FIXED);
	map("FLOATING_MASTER", FLOATING_MASTER);
	map("FLOATING_MASTER_FIXED", FLOATING_MASTER_FIXED);

	fprintf(stderr, "Warning: config could not find layout split with name %s\n", string);
	return NO_SPLIT;
}

int
parse_layout_arrangement(const char *string)
{
	map("TOP_TO_BOTTOM", TOP_TO_BOTTOM);
	map("TTP", TOP_TO_BOTTOM);
	map("LEFT_TO_RIGHT", LEFT_TO_RIGHT);
	map("LTR", LEFT_TO_RIGHT);
	map("MONOCLE", MONOCLE);
	map("GAPLESSGRID", GAPLESSGRID);
	map("GAPPLESSGRID", GAPPLESSGRID);
	map("GAPLESSGRID_CFACTS", GAPLESSGRID_CFACTS);
	map("GAPPLESSGRID_CFACTS", GAPPLESSGRID_CFACTS);
	map("GAPLESSGRID_ALT1", GAPLESSGRID_ALT1);
	map("GAPPLESSGRID_ALT1", GAPPLESSGRID_ALT1);
	map("GAPLESSGRID_ALT2", GAPLESSGRID_ALT2);
	map("GAPPLESSGRID_ALT2", GAPPLESSGRID_ALT2);
	map("GRIDMODE", GRIDMODE);
	map("HORIZGRID", HORIZGRID);
	map("DWINDLE", DWINDLE);
	map("DWINDLE_CFACTS", DWINDLE_CFACTS);
	map("SPIRAL", SPIRAL);
	map("SPIRAL_CFACTS", SPIRAL_CFACTS);
	map("TATAMI", TATAMI);
	map("TATAMI_CFACTS", TATAMI_CFACTS);
	map("ASPECTGRID", ASPECTGRID);
	map("TOP_TO_BOTTOM_AR", TOP_TO_BOTTOM_AR);
	map("TTB_AR", TOP_TO_BOTTOM_AR);
	map("LEFT_TO_RIGHT_AR", LEFT_TO_RIGHT_AR);
	map("LTR_AR", LEFT_TO_RIGHT_AR);

	fprintf(stderr, "Warning: config could not find arrangement with name %s\n", string);
	return TOP_TO_BOTTOM;
}

unsigned int
parse_modifier(const char *string)
{
	int i, len;
	unsigned int mask = 0;
	len = strlen(string) + 1;
	char buffer[len];
	strlcpy(buffer, string, len);
	const char *delims = "+-|:;, ";
	const char *modifier_strings[] = {
		"Super",
		"AltGr",
		"Alt",
		"ShiftGr",
		"Shift",
		"Ctrl",
		NULL
	};
	const unsigned int modifier_values[] = {
		Mod4Mask,
		Mod3Mask,
		Mod1Mask,
		Mod5Mask,
		ShiftMask,
		ControlMask
	};

	char *token = strtok(buffer, delims);
	while (token) {
		for (i = 0; modifier_strings[i]; i++) {
			if (!strcasecmp(token, modifier_strings[i])) {
				mask |= modifier_values[i];
				break;
			}
		}
		token = strtok(NULL, delims);
	}

	return mask;
}

int
parse_scheme(const char *string)
{
	if (startswith("Scheme", string))
		string += 6;

	map("Norm", SchemeNorm);
	map("Sel", SchemeSel);
	map("TitleNorm", SchemeTitleNorm);
	map("TitleSel", SchemeTitleSel);
	map("WsNorm", SchemeWsNorm);
	map("WsVisible", SchemeWsVisible);
	map("WsSel", SchemeWsSel);
	map("WsOcc", SchemeWsOcc);
	map("ScratchSel", SchemeScratchSel);
	map("ScratchNorm", SchemeScratchNorm);
	map("HidSel", SchemeHidSel);
	map("HidNorm", SchemeHidNorm);
	map("Urg", SchemeUrg);
	map("Urgent", SchemeUrg);
	map("Marked", SchemeMarked);

	fprintf(stderr, "Warning: config could not find color scheme with name %s\n", string);
	return SchemeNorm;
}

int
parse_stacker_placement(const char *string)
{
	if (startswith("Stacker", string))
		string += 7;

	map("RightOfWindowIcon", StackerRightOfWindowIcon);
	map("LeftOfWindowIcon", StackerLeftOfWindowIcon);
	map("TitlePrefix", StackerTitlePrefix);
	map("TitleSuffix", StackerTitleSuffix);
	map("TitleEllipsis", StackerTitleEllipsis);

	fprintf(stderr, "Warning: config could not find stacker placement with name %s\n", string);
	return StackerTitlePrefix;
}

int
parse_bar_rule_value(const char *string)
{
	if (startswith("Pwrl", string))
		string += 4;

	map("None", PwrlNone);
	map("RightArrow", PwrlRightArrow);
	map("LeftArrow", PwrlLeftArrow);
	map("ForwardSlash", PwrlForwardSlash);
	map("Backslash", PwrlBackslash);
	map("Solid", PwrlSolid);
	map("SolidRev", PwrlSolidRev);

	fprintf(stderr, "Warning: config could not find powerline option with name %s\n", string);
	return atoi(string);
}

const char *
parse_window_type(const char *string)
{
	if (startswith("_NET_WM_WINDOW_TYPE_", string))
		string += 20;

	map("NORMAL", "_NET_WM_WINDOW_TYPE_NORMAL");
	map("DESKTOP", "_NET_WM_WINDOW_TYPE_DESKTOP");
	map("DOCK", "_NET_WM_WINDOW_TYPE_DOCK");
	map("TOOLBAR", "_NET_WM_WINDOW_TYPE_TOOLBAR");
	map("MENU", "_NET_WM_WINDOW_TYPE_MENU");
	map("UTILITY", "_NET_WM_WINDOW_TYPE_UTILITY");
	map("SPLASH", "_NET_WM_WINDOW_TYPE_SPLASH");
	map("DIALOG", "_NET_WM_WINDOW_TYPE_DIALOG");
	map("DROPDOWN_MENU", "_NET_WM_WINDOW_TYPE_DROPDOWN_MENU");
	map("POPUP_MENU", "_NET_WM_WINDOW_TYPE_POPUP_MENU");
	map("TOOLTIP", "_NET_WM_WINDOW_TYPE_TOOLTIP");
	map("NOTIFICATION", "_NET_WM_WINDOW_TYPE_NOTIFICATION");
	map("COMBO", "_NET_WM_WINDOW_TYPE_COMBO");
	map("DND", "_NET_WM_WINDOW_TYPE_DND");

	return string;
}

void *
parse_void_reference(const char *string)
{
	int i;
	for (i = 0; i < num_commands; i++) {
		if (!strcasecmp(string, _cfg_commands[i].name)) {
			return _cfg_commands[i].argv;
		}
	}

	map("dmenucmd", dmenucmd);
	map("termcmd", termcmd);

	for (i = 0; i < num_cached_strings; i++) {
		if (!strcmp(string, _cfg_cached_strings_array[i])) {
			return _cfg_cached_strings_array[i];
		}
	}

	if (num_cached_strings + 1 > LENGTH(_cfg_cached_strings_array)) {
		fprintf(stderr, "Warning: config could not cache new void reference with name %s\n", string);
		return strdup(string);
	}

	_cfg_cached_strings_array[num_cached_strings] = strdup(string);

	return _cfg_cached_strings_array[num_cached_strings++];
}

#undef map
#define map(M, S, D, C, H) \
	if (!strcasecmp(string, M)) { \
		rule->sizefunc = S; \
		rule->drawfunc = D; \
		rule->clickfunc = C; \
		rule->hoverfunc = H; \
		return; \
	}

void
parse_module(const char *string, BarRule *rule)
{
	map("powerline", size_powerline, draw_powerline, NULL, NULL);
	map("status", size_status, draw_status, click_status, NULL);
	map("workspaces", size_workspaces, draw_workspaces, click_workspaces, hover_workspaces);
	map("systray", size_systray, draw_systray, click_systray, NULL);
	map("ltsymbol", size_ltsymbol, draw_ltsymbol, click_ltsymbol, NULL);
	map("flexwintitle", size_flexwintitle, draw_flexwintitle, click_flexwintitle, NULL);
	map("wintitle_floating", size_wintitle_floating, draw_wintitle_floating, click_wintitle_floating, NULL);
	map("wintitle_hidden", size_wintitle_hidden, draw_wintitle_hidden, click_wintitle_hidden, NULL);
	map("wintitle_sticky", size_wintitle_sticky, draw_wintitle_sticky, click_wintitle_sticky, NULL);
	map("wintitle_single", size_wintitle_single, draw_wintitle_single, click_wintitle_single, NULL);
	map("pwrl_ifhidfloat", size_pwrl_ifhidfloat, draw_powerline, NULL, NULL);
}

#undef map
#define map(M, VALUE) \
	if (!strcasecmp(string, M)) { \
		*ptr = VALUE; \
		return 1; \
	}

/* This attempts to parse function specific constants.
 * If a constant is found then that will be stored in the passed pointer,
 * whereas the return value indicates whether a constant was found or not.
 * The reason for implementing it like this is that some constants map to 0,
 * so it is not possible to tell from the return value alone whether a
 * constant was mapped or not. */
int
parse_arg_function_int_constant(const char *string, ArgFunc func, int *ptr)
{
	int value;

	if (func == stackfocus || func == stackpush || func == stackswap) {

		if (sscanf(string, "INC(%d)", &value) == 1) {
			*ptr = INC(value);
			return 1;
		}

		if (sscanf(string, "MASTER(%d)", &value) == 1) {
			*ptr = MASTER(value);
			return 1;
		}

		if (sscanf(string, "STACK(%d)", &value) == 1) {
			*ptr = STACK(value);
			return 1;
		}

		if (sscanf(string, "TILE(%d)", &value) == 1) {
			*ptr = TILE(value);
			return 1;
		}

		map("PREVSEL", PREVSEL);
		map("LASTTILED", LASTTILED);
		return 0;
	}

	if (func == setlayout) {
		*ptr = parse_layout(string);
		return 1;
	}

	if (func == rotatelayoutaxis) {
		map("LAYOUT", LAYOUT + 1);
		map("MASTER", MASTER + 1);
		map("STACK", STACK + 1);
		map("STACK1", STACK + 1);
		map("STACK2", STACK2 + 1);
		map("-LAYOUT", -LAYOUT - 1);
		map("-MASTER", -MASTER - 1);
		map("-STACK", -STACK - 1);
		map("-STACK1", -STACK - 1);
		map("-STACK2", -STACK2 - 1);
		map("MIRRORMASTER", -MASTER - 1);
		map("MIRRORSTACK", -STACK - 1);
		map("MIRRORSTACK1", -STACK - 1);
		map("MIRRORSTACK2", -STACK2 - 1);
		return 0;
	}

	if (func == markall) {
		if (startswith("MARKALL_", string))
			string += 8;

		map("ALL", MARKALL_ALL);
		map("FLOATING", MARKALL_FLOATING);
		map("HIDDEN", MARKALL_HIDDEN);
		map("TILED", MARKALL_TILED);
		return 0;
	}

	if (func == focusdir || func == placedir) {
		map("LEFT", LEFT);
		map("RIGHT", RIGHT);
		map("UP", UP);
		map("DOWN", DOWN);
		return 0;
	}

	map("INC", +1);
	map("DEC", -1)
	map("INCR", +1);
	map("DECR", -1);
	map("INC2", +2);
	map("DEC2", -2);

	return 0;
}

#undef map
