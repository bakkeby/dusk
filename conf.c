#include <libconfig.h>

/* libconfig helper functions */
int config_setting_lookup_strdup(const config_setting_t *cfg, const char *name, char **strptr);
int config_lookup_sloppy_bool(const config_t *cfg, const char *name, int *ptr);
int config_setting_lookup_sloppy_bool(const config_setting_t *cfg, const char *name, int *ptr);
int config_setting_get_sloppy_bool(const config_setting_t *cfg, int *ptr);
int setting_length(const config_setting_t *cfg);
const char *setting_get_string_elem(const config_setting_t *cfg, int i);
int setting_get_int_elem(const config_setting_t *cfg, int i);
const config_setting_t *setting_get_elem(const config_setting_t *cfg, int i);

void set_config_path(const char* filename, char *config_path, char *config_file);
void read_config(void);
void read_autostart(config_t *cfg);
void read_bar(config_t *cfg);
void read_button_bindings(config_t *cfg);
void read_clientrules(config_t *cfg);
void read_colors(config_t *cfg);
void read_commands(config_t *cfg);
void read_fonts(config_t *cfg);
void read_functionality(config_t *cfg);
void read_indicators(config_t *cfg);
void read_keybindings(config_t *cfg);
void read_layouts(config_t *cfg);
void read_workspace(config_t *cfg);
void read_singles(config_t *cfg);
int parse_align(const char *string);
int parse_click(const char *string);
ArgFunc parse_functionailty(const char *string);
int parse_indicator(const char *indicator);
int parse_function_int_constant(const char *string, ArgFunc func, int *ptr);
int parse_key_type(const char *string);
int parse_layout(const char *string);
int parse_layout_split(const char *string);
int parse_layout_arrangement(const char *string);
unsigned int parse_modifier(const char *string);
int parse_scheme(const char *string);
int parse_stacker_placement(const char *string);
int parse_value(const char *string);
void *parse_void_reference(const char *string);
const char *parse_window_type(const char *string);
void parse_module(const char *string, BarRule *rule);
void add_button_binding(unsigned int click, unsigned int mask, unsigned int button, ArgFunc function, int argument, void *void_argument, float float_argument);
#if USE_KEYCODES
void add_key_binding(int type, unsigned int mod, KeyCode keycode, ArgFunc function, int argument, void *void_argument, float float_argument);
#else
void add_key_binding(int type, unsigned int mod, KeySym keysym, ArgFunc function, int argument, void *void_argument, float float_argument);
#endif // USE_KEYCODES
void add_stacker_icon(config_t *cfg, const char *string, int value);

int
config_setting_lookup_strdup(const config_setting_t *cfg, const char *name, char **strptr)
{
	const char *string;
	if (config_setting_lookup_string(cfg, name, &string)) {
		free(*strptr);
		*strptr = strdup(string);
		return 1;
	}

	return 0;
}

int
config_lookup_sloppy_bool(const config_t *cfg, const char *name, int *ptr)
{
	return config_setting_get_sloppy_bool(config_lookup(cfg, name), ptr);
}

int
config_setting_lookup_sloppy_bool(const config_setting_t *cfg, const char *name, int *ptr)
{
	return config_setting_get_sloppy_bool(config_setting_lookup(cfg, name), ptr);
}

int
config_setting_get_sloppy_bool(const config_setting_t *cfg_item, int *ptr)
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
	const char* xdg_config_home = getenv("XDG_CONFIG_HOME");
	const char* home = getenv("HOME");

	if (xdg_config_home && xdg_config_home[0] != '\0') {
		snprintf(config_path, PATH_MAX, "%s/dusk/", xdg_config_home);
	} else if (home) {
		snprintf(config_path, PATH_MAX, "%s/.config/dusk/", home);
	} else {
		return;
	}

	snprintf(config_file, PATH_MAX, "%s/%s", config_path, filename);
}

void
read_config(void)
{
	config_t cfg;
	char config_path[PATH_MAX] = {0};
	char config_file[PATH_MAX] = {0};

	set_config_path("dusk.cfg", config_path, config_file);
	config_init(&cfg);
	config_set_include_dir(&cfg, config_path);
	if (!config_read_file(&cfg, config_file)) {
		if (strcmp(config_error_text(&cfg), "file I/O error")) {
			config_error = ecalloc(PATH_MAX + 255, sizeof(char));
			snprintf(config_error, PATH_MAX + 255,
				"Config %s: %s:%d",
				config_error_text(&cfg),
				config_file,
				config_error_line(&cfg));
		}

		fprintf(stderr, "Error reading config at %s\n", config_file);
		fprintf(stderr, "%s:%d - %s\n",
				config_error_file(&cfg),
				config_error_line(&cfg),
				config_error_text(&cfg));

		config_destroy(&cfg);
		return;
	}

	read_singles(&cfg);
	read_commands(&cfg);
	read_autostart(&cfg);
	read_bar(&cfg);
	read_button_bindings(&cfg);
	read_clientrules(&cfg);
	read_colors(&cfg);
	read_fonts(&cfg);
	read_functionality(&cfg);
	read_indicators(&cfg);
	read_keybindings(&cfg);
	read_layouts(&cfg);
	read_workspace(&cfg);
	config_destroy(&cfg);
}

void
read_autostart(config_t *cfg)
{
	int i;
	config_setting_t *auto_cfg;

	auto_cfg = config_lookup(cfg, "autostart");
	if (auto_cfg && config_setting_is_array(auto_cfg)) {
		num_autostart = config_setting_length(auto_cfg);
		if (num_autostart) {
			autostart = ecalloc(num_autostart + 1, sizeof(char **));
			for (i = 0; i < num_autostart; i++) {
				autostart[i] = parse_void_reference(config_setting_get_string_elem(auto_cfg, i));
			}
		}
	}

	auto_cfg = config_lookup(cfg, "autorestart");
	if (auto_cfg && config_setting_is_array(auto_cfg)) {
		num_autorestart = config_setting_length(auto_cfg);
		if (num_autorestart) {
			autorestart = ecalloc(num_autorestart + 1, sizeof(char **));
			for (i = 0; i < num_autorestart; i++) {
				autorestart[i] = parse_void_reference(config_setting_get_string_elem(auto_cfg, i));
			}
		}
	}
}

void
read_bar(config_t *cfg)
{
	int i, alpha;
	const char *string;
	config_setting_t *barconfig, *bar, *rules, *rule, *monitor, *scheme, *value, *align;

	config_lookup_sloppy_bool(cfg, "bar.showbar", &initshowbar);

	config_lookup_int(cfg, "bar.height", &bar_height);
	if (!config_lookup_int(cfg, "bar.vertpad", &vertpad))
		vertpad = borderpx;
	if (!config_lookup_int(cfg, "bar.sidepad", &sidepad))
		sidepad = borderpx;
	config_lookup_int(cfg, "bar.text_padding", &horizpadbar);
	config_lookup_int(cfg, "bar.height_padding", &vertpadbar);
	config_lookup_int(cfg, "bar.systrayspacing", &systrayspacing);

	if (config_lookup_int(cfg, "bar.alpha_fg", &alpha))
		default_alphas[ColFg] = alpha;
	if (config_lookup_int(cfg, "bar.alpha_bg", &alpha))
		default_alphas[ColBg] = alpha;
	if (config_lookup_int(cfg, "bar.alpha_border", &alpha))
		default_alphas[ColBorder] = alpha;

	/* Bars */
	barconfig = config_lookup(cfg, "bar.bars");
	if (barconfig && config_setting_is_list(barconfig)) {
		if ((num_bars = config_setting_length(barconfig))) {
			bars = ecalloc(num_bars, sizeof(BarDef));
			for (i = 0; i < num_bars; i++) {
				bars[i].barpos = NULL;
				bars[i].extclass = NULL;
				bars[i].extinstance = NULL;
				bars[i].extname = NULL;
				bars[i].name = NULL;

				bar = config_setting_get_elem(barconfig, i);
				config_setting_lookup_int(bar, "monitor", &bars[i].monitor);
				config_setting_lookup_int(bar, "bar", &bars[i].idx);
				config_setting_lookup_int(bar, "vert", &bars[i].vert);
				config_setting_lookup_strdup(bar, "pos", &bars[i].barpos);
				config_setting_lookup_strdup(bar, "extclass", &bars[i].extclass);
				config_setting_lookup_strdup(bar, "extinst", &bars[i].extinstance);
				config_setting_lookup_strdup(bar, "extname", &bars[i].extname);
				config_setting_lookup_strdup(bar, "name", &bars[i].name);
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

	barrules = ecalloc(num_barrules, sizeof(BarRule));
	for (i = 0; i < num_barrules; i++) {
		/* Default values */
		barrules[i].monitor = -1;
		barrules[i].bar = 0;
		barrules[i].scheme = SchemeNorm;
		barrules[i].lpad = 0;
		barrules[i].rpad = 0;
		barrules[i].value = 0;
		barrules[i].alignment = 0;
		barrules[i].name = NULL;

		rule = config_setting_get_elem(rules, i);
		if (!rule)
			continue;

		if ((monitor = config_setting_lookup(rule, "monitor"))) {
			switch (config_setting_type(monitor)) {
			case CONFIG_TYPE_INT:
				barrules[i].monitor = config_setting_get_int(monitor);
				break;
			case CONFIG_TYPE_STRING:
				string = config_setting_get_string(monitor);
				if (!strcasecmp("A", string)) {
					barrules[i].scheme = 'A';
				} else {
					barrules[i].scheme = atoi(string);
				}
				break;
			}
		}

		config_setting_lookup_int(rule, "bar", &barrules[i].bar);

		if ((scheme = config_setting_lookup(rule, "scheme"))) {
			switch (config_setting_type(scheme)) {
			case CONFIG_TYPE_INT:
				barrules[i].scheme = config_setting_get_int(scheme);
				break;
			case CONFIG_TYPE_STRING:
				barrules[i].scheme = parse_scheme(config_setting_get_string(scheme));
				break;
			}
		}

		config_setting_lookup_int(rule, "padding", &barrules[i].lpad);
		config_setting_lookup_int(rule, "padding", &barrules[i].rpad);
		config_setting_lookup_int(rule, "lpad", &barrules[i].lpad);
		config_setting_lookup_int(rule, "rpad", &barrules[i].rpad);

		if ((value = config_setting_lookup(rule, "value"))) {
			switch (config_setting_type(value)) {
			case CONFIG_TYPE_INT:
				barrules[i].value = config_setting_get_int(value);
				break;
			case CONFIG_TYPE_STRING:
				barrules[i].value = parse_value(config_setting_get_string(value));
				break;
			}
		}

		if ((align = config_setting_lookup(rule, "align"))) {
			switch (config_setting_type(align)) {
			case CONFIG_TYPE_INT:
				barrules[i].alignment = config_setting_get_int(align);
				break;
			case CONFIG_TYPE_STRING:
				barrules[i].alignment = parse_align(config_setting_get_string(align));
				break;
			}
		}

		config_setting_lookup_string(rule, "module", &string);
		parse_module(string, &barrules[i]);

		config_setting_lookup_strdup(rule, "name", &barrules[i].name);
	}
}

void
read_button_bindings(config_t *cfg)
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

	buttons = ecalloc(MAX(num_bindings * 2, 60), sizeof(Button));

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
			function_arr[j] = parse_functionailty(setting_get_string_elem(function, j));
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
				if (parse_function_int_constant(string, function_arr[j % num_functions], &value)) {
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
	buttons[num_button_bindings].click = click;
	buttons[num_button_bindings].mask = mask;
	buttons[num_button_bindings].button = button;
	buttons[num_button_bindings].func = function;
	if (void_argument != NULL) {
		buttons[num_button_bindings].arg.v = void_argument;
	} else if (float_argument != 0) {
		buttons[num_button_bindings].arg.f = float_argument;
	} else {
		buttons[num_button_bindings].arg.i = argument;
	}

	num_button_bindings++;
}

void
read_clientrules(config_t *cfg)
{
	int i, f, num_flags;
	Rule *r;
	const char *string;

	config_setting_t *rules, *rule, *flags;

	rules = config_lookup(cfg, "client_rules");
	if (!rules || !config_setting_is_list(rules))
		return;

	num_client_rules = config_setting_length(rules);
	if (!num_client_rules)
		return;

	clientrules = ecalloc(num_client_rules, sizeof(Rule));

	for (i = 0; i < num_client_rules; i++) {

		r = &clientrules[i];

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
read_colors(config_t *cfg)
{
	int i, num_cols;
	config_setting_t *cols, *col;

	cols = config_lookup(cfg, "colors");
	if (!cols || !config_setting_is_group(cols))
		return;

	num_cols = config_setting_length(cols);
	if (!num_cols)
		return;

	/* Parse and set the colors based on config */
	for (i = 0; i < num_cols; i++) {
		col = config_setting_get_elem(cols, i);
		int scheme = parse_scheme(config_setting_name(col));

		config_setting_lookup_strdup(col, "fg", &colors[scheme][ColFg]);
		config_setting_lookup_strdup(col, "bg", &colors[scheme][ColBg]);
		config_setting_lookup_strdup(col, "border", &colors[scheme][ColBorder]);
	}
}

void
read_commands(config_t *cfg)
{
	int i, j, num_cmd_elements;
	config_setting_t *commands_list, *command_entry, *command;

	commands_list = config_lookup(cfg, "commands");
	if (!commands_list || !config_setting_is_list(commands_list))
		return;

	num_commands = config_setting_length(commands_list);
	if (!num_commands)
		return;

	commands = ecalloc(num_commands, sizeof(Command));
	for (i = 0; i < num_commands; i++) {
		command_entry = config_setting_get_elem(commands_list, i);
		commands[i].name = NULL;
		commands[i].argv = NULL;

		config_setting_lookup_strdup(command_entry, "name", &commands[i].name);

		command = config_setting_lookup(command_entry, "command");
		num_cmd_elements = config_setting_length(command);
		commands[i].argv = ecalloc(num_cmd_elements + 2, sizeof(char*));

		if (!config_setting_lookup_strdup(command_entry, "scratchkey", &commands[i].argv[0])) {
			commands[i].argv[0] = NULL;
		}

		for (j = 0; j < num_cmd_elements; j++) {
			commands[i].argv[j + 1] = strdup(config_setting_get_string_elem(command, j));
		}
		commands[i].argv[j + 1] = NULL;

		if (commands[i].name == NULL || commands[i].argv == NULL) {
			fprintf(stderr, "Warning: config found incomplete command at line %d\n", config_setting_source_line(command_entry));
		}
	}
}

void
read_keybindings(config_t *cfg)
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

	keys = ecalloc(MAX(num_bindings * 2, 200), sizeof(Key));

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
			function_arr[j] = parse_functionailty(setting_get_string_elem(function, j));
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
				if (parse_function_int_constant(string, function_arr[j % num_functions], &value)) {
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

void add_key_binding(
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
	keys[num_key_bindings].type = type;
	keys[num_key_bindings].mod = mod;
	#if USE_KEYCODES
	keys[num_key_bindings].keycode = keycode;
	#else
	keys[num_key_bindings].keysym = keysym;
	#endif // USE_KEYCODES
	keys[num_key_bindings].func = function;
	if (void_argument != NULL) {
		keys[num_key_bindings].arg.v = void_argument;
	} else if (float_argument != 0) {
		keys[num_key_bindings].arg.f = float_argument;
	} else {
		keys[num_key_bindings].arg.i = argument;
	}

	num_key_bindings++;
}

void
add_stacker_icon(config_t *cfg, const char *string, int value)
{
	const char *prefix, *suffix, *pos_string, *icon_char, *replace_str;
	int i, num_overrides, overridden = 0;
	int position = StackerTitlePrefix;
	config_setting_t *stacker_cfg, *overrides, *override;

	if (stackericons == NULL) {
		stackericons = ecalloc(30, sizeof(StackerIcon));
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

	stackericons[num_stackericons].icon = icon;
	stackericons[num_stackericons].arg.i = value;
	stackericons[num_stackericons].pos = position;

	num_stackericons++;
}

void
read_fonts(config_t *cfg)
{
	int i, num_fonts;
	config_setting_t *fonts;

	fonts = config_lookup(cfg, "fonts");
	if (!fonts)
		return;

	num_fonts = config_setting_length(fonts);

	for (i = 0; i < num_fonts; i++) {
		drw_font_add(drw, config_setting_get_string_elem(fonts, i));
	}
}

void
read_singles(config_t *cfg)
{
	const char *string;

	config_lookup_int(cfg, "borderpx", &borderpx);
	config_lookup_int(cfg, "gaps.ih", &gappih);
	config_lookup_int(cfg, "gaps.iv", &gappiv);
	config_lookup_int(cfg, "gaps.oh", &gappoh);
	config_lookup_int(cfg, "gaps.ov", &gappov);
	config_lookup_int(cfg, "gaps.fl", &gappfl);
	config_lookup_sloppy_bool(cfg, "gaps.enabled", &enablegaps);
	config_lookup_int(cfg, "gaps.smartgaps_fact", &smartgaps_fact);

	config_lookup_int(cfg, "nmaster", &nmaster);
	config_lookup_int(cfg, "nstack", &nstack);
	config_lookup_float(cfg, "mfact", &mfact);

	if (config_lookup_string(cfg, "attachdefault", &string)) {
		uint64_t flag = getflagbyname(string) & AttachFlag;

		if (flag) {
			attachdefault = flag;
		}
	}

	if (config_lookup_string(cfg, "slop.spawnstyle", &string))
		strlcpy(slopspawnstyle, string, LENGTH(slopspawnstyle));
	if (config_lookup_string(cfg, "slop.resizestyle", &string))
		strlcpy(slopresizestyle, string, LENGTH(slopresizestyle));

	/* floatpos settings */
	config_lookup_int(cfg, "floatpos.grid_x", &floatposgrid_x);
	config_lookup_int(cfg, "floatpos.grid_y", &floatposgrid_y);

	if (config_lookup_string(cfg, "floatpos.toggle_pos", &string))
		strlcpy(toggle_float_pos, string, LENGTH(toggle_float_pos));

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
}

void
read_workspace(config_t *cfg)
{
	const char *string;
	config_setting_t *rules, *rule, *icons, *layout;
	int i;

	config_lookup_float(cfg, "workspace.preview_factor", &pfact);

	if (config_lookup_string(cfg, "workspace.labels.occupied_format", &string))
		strlcpy(occupied_workspace_label_format, string, LENGTH(occupied_workspace_label_format));
	if (config_lookup_string(cfg, "workspace.labels.vacant_format", &string))
		strlcpy(vacant_workspace_label_format, string, LENGTH(vacant_workspace_label_format));

	config_lookup_sloppy_bool(cfg, "workspace.labels.lowercase", &lowercase_workspace_labels);
	config_lookup_sloppy_bool(cfg, "workspace.labels.prefer_window_icons", &prefer_window_icons_over_workspace_labels);
	config_lookup_sloppy_bool(cfg, "workspace.labels.swap_occupied_format", &swap_occupied_workspace_label_format_strings);
	config_lookup_sloppy_bool(cfg, "workspace.per_mon", &workspaces_per_mon);

	/* Workspace rules */
	rules = config_lookup(cfg, "workspace.rules");
	if (!rules || !config_setting_is_list(rules))
		return;

	num_wsrules = config_setting_length(rules);
	if (!num_wsrules)
		return;

	wsrules = ecalloc(num_wsrules, sizeof(WorkspaceRule));
	for (i = 0; i < num_wsrules; i++) {
		/* Default values */
		wsrules[i].norm_scheme = SchemeWsNorm;
		wsrules[i].vis_scheme = SchemeWsVisible;
		wsrules[i].sel_scheme = SchemeWsSel;
		wsrules[i].occ_scheme = SchemeWsOcc;
		wsrules[i].monitor = -1;
		wsrules[i].pinned = 0;
		wsrules[i].layout = 0;
		wsrules[i].mfact = -1;
		wsrules[i].nmaster = -1;
		wsrules[i].nstack = -1;
		wsrules[i].enablegaps = -1;
		wsrules[i].name = NULL;
		wsrules[i].icondef = NULL;
		wsrules[i].iconvac = NULL;
		wsrules[i].iconocc = NULL;

		rule = config_setting_get_elem(rules, i);
		if (!rule)
			continue;

		config_setting_lookup_strdup(rule, "name", &wsrules[i].name);
		config_setting_lookup_sloppy_bool(rule, "pinned", &wsrules[i].pinned);

		/* Allow layout to be referred to by name as well as index */
		if ((layout = config_setting_lookup(rule, "layout"))) {
			switch (config_setting_type(layout)) {
			case CONFIG_TYPE_INT:
				wsrules[i].layout = config_setting_get_int(layout);
				break;
			case CONFIG_TYPE_STRING:
				wsrules[i].layout = parse_layout(config_setting_get_string(layout));
				break;
			}
		}

		config_setting_lookup_int(rule, "monitor", &wsrules[i].monitor);
		config_setting_lookup_float(rule, "mfact", &wsrules[i].mfact);
		config_setting_lookup_int(rule, "nmaster", &wsrules[i].nmaster);
		config_setting_lookup_int(rule, "nstack", &wsrules[i].nstack);
		config_setting_lookup_sloppy_bool(rule, "gaps", &wsrules[i].enablegaps);

		icons = config_setting_lookup(rule, "icons");
		if (icons) {
			if (!config_setting_lookup_strdup(icons, "def", &wsrules[i].icondef)) {
				wsrules[i].icondef = strdup("◉");
			}

			if (!config_setting_lookup_strdup(icons, "vac", &wsrules[i].iconvac)) {
				wsrules[i].iconvac = wsrules[i].icondef;
			}

			if (!config_setting_lookup_strdup(icons, "occ", &wsrules[i].iconocc)) {
				wsrules[i].iconocc = wsrules[i].icondef;
			}
		} else {
			wsrules[i].icondef = strdup("◉");
			wsrules[i].iconocc = strdup("●");
		}
	}

}

#define readfunc(F) if (config_lookup_sloppy_bool(cfg, "functionality." #F, &enabled)) { if (enabled) { enablefunc(F); } else { disablefunc(F); } }

void
read_functionality(config_t *cfg)
{
	int enabled;
	readfunc(AutoReduceNmaster);
	readfunc(BanishMouseCursor);
	readfunc(BanishMouseCursorToCorner);
	readfunc(SmartGaps);
	readfunc(SmartGapsMonocle);
	readfunc(Systray);
	readfunc(SystrayNoAlpha);
	readfunc(Swallow);
	readfunc(SwallowFloating);
	readfunc(CenteredWindowName);
	readfunc(BarActiveGroupBorderColor);
	readfunc(BarMasterGroupBorderColor);
	readfunc(FlexWinBorders);
	readfunc(SpawnCwd);
	readfunc(ColorEmoji);
	readfunc(Status2DNoAlpha);
	readfunc(BarBorder);
	readfunc(BarBorderColBg);
	readfunc(BarPadding);
	readfunc(NoBorders);
	readfunc(Warp);
	readfunc(DecorationHints);
	readfunc(FocusedOnTop);
	readfunc(FocusedOnTopTiled);
	readfunc(FocusFollowMouse);
	readfunc(FocusOnClick);
	readfunc(FocusOnNetActive);
	readfunc(AllowNoModifierButtons);
	readfunc(CenterSizeHintsClients);
	readfunc(ResizeHints);
	readfunc(SnapToWindows);
	readfunc(SnapToGaps);
	readfunc(SortScreens);
	readfunc(ViewOnWs);
	readfunc(Xresources);
	readfunc(Debug);
	readfunc(AltWindowTitles);
	readfunc(AltWorkspaceIcons);
	readfunc(GreedyMonitor);
	readfunc(SmartLayoutConversion);
	readfunc(AutoHideScratchpads);
	readfunc(RioDrawIncludeBorders);
	readfunc(RioDrawSpawnAsync);
	readfunc(RestrictFocusstackToMonitor);
	readfunc(WinTitleIcons);
	readfunc(StackerIcons);
	readfunc(WorkspaceLabels);
	readfunc(WorkspacePreview);
}

#undef readfunc

void
read_layouts(config_t *cfg)
{
	int i;
	const char *string;
	config_setting_t *lts, *lt;

	/* Layouts */
	lts = config_lookup(cfg, "layouts");
	if (!lts || !config_setting_is_list(lts))
		return;

	num_layouts = config_setting_length(lts);
	if (!num_layouts)
		return;

	layouts = ecalloc(num_layouts, sizeof(Layout));
	for (i = 0; i < num_layouts; i++) {
		layouts[i].preset.nmaster = -1;
		layouts[i].preset.nstack = -1;
		layouts[i].preset.layout = NO_SPLIT;
		layouts[i].preset.masteraxis = 0;
		layouts[i].preset.stack1axis = 0;
		layouts[i].preset.stack2axis = 0;
		layouts[i].preset.symbolfunc = NULL;
		layouts[i].arrange = NULL;

		lt = config_setting_get_elem(lts, i);
		config_setting_lookup_int(lt, "nmaster", &layouts[i].preset.nmaster);
		config_setting_lookup_int(lt, "nstack", &layouts[i].preset.nstack);
		config_setting_lookup_strdup(lt, "symbol", &layouts[i].symbol);
		config_setting_lookup_strdup(lt, "name", &layouts[i].name);

		if (config_setting_lookup_string(lt, "split", &string))
			layouts[i].preset.layout = parse_layout_split(string);
		if (config_setting_lookup_string(lt, "master", &string)) {
			layouts[i].arrange = flextile;
			layouts[i].preset.masteraxis = parse_layout_arrangement(string);
		}
		if (config_setting_lookup_string(lt, "stack", &string))
			layouts[i].preset.stack1axis = parse_layout_arrangement(string);
		if (config_setting_lookup_string(lt, "stack2", &string))
			layouts[i].preset.stack2axis = parse_layout_arrangement(string);
	}
}

void
read_indicators(config_t *cfg)
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

	if (config_lookup_string(cfg, "indicator.custom.custom_1", &indicator))
		strlcpy(custom_2d_indicator_1, indicator, LENGTH(custom_2d_indicator_1));
	if (config_lookup_string(cfg, "indicator.custom.custom_2", &indicator))
		strlcpy(custom_2d_indicator_2, indicator, LENGTH(custom_2d_indicator_2));
	if (config_lookup_string(cfg, "indicator.custom.custom_3", &indicator))
		strlcpy(custom_2d_indicator_3, indicator, LENGTH(custom_2d_indicator_3));
	if (config_lookup_string(cfg, "indicator.custom.custom_4", &indicator))
		strlcpy(custom_2d_indicator_4, indicator, LENGTH(custom_2d_indicator_4));
	if (config_lookup_string(cfg, "indicator.custom.custom_5", &indicator))
		strlcpy(custom_2d_indicator_5, indicator, LENGTH(custom_2d_indicator_5));
	if (config_lookup_string(cfg, "indicator.custom.custom_6", &indicator))
		strlcpy(custom_2d_indicator_6, indicator, LENGTH(custom_2d_indicator_6));
}

#define map(S, I) if (!strcasecmp(string, S)) return I;

int
parse_align(const char *string)
{
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

	map("BAR_ALIGN_NONE", BAR_ALIGN_NONE);
	map("BAR_ALIGN_TOP", BAR_ALIGN_TOP);
	map("BAR_ALIGN_LEFT", BAR_ALIGN_LEFT);
	map("BAR_ALIGN_CENTER", BAR_ALIGN_CENTER);
	map("BAR_ALIGN_BOTTOM", BAR_ALIGN_BOTTOM);
	map("BAR_ALIGN_RIGHT", BAR_ALIGN_RIGHT);
	map("BAR_ALIGN_TOP_TOP", BAR_ALIGN_TOP_TOP);
	map("BAR_ALIGN_TOP_CENTER", BAR_ALIGN_TOP_CENTER);
	map("BAR_ALIGN_TOP_BOTTOM", BAR_ALIGN_TOP_BOTTOM);
	map("BAR_ALIGN_LEFT_LEFT", BAR_ALIGN_LEFT_LEFT);
	map("BAR_ALIGN_LEFT_RIGHT", BAR_ALIGN_LEFT_RIGHT);
	map("BAR_ALIGN_LEFT_CENTER", BAR_ALIGN_LEFT_CENTER);
	map("BAR_ALIGN_RIGHT_LEFT", BAR_ALIGN_RIGHT_LEFT);
	map("BAR_ALIGN_RIGHT_RIGHT", BAR_ALIGN_RIGHT_RIGHT);
	map("BAR_ALIGN_RIGHT_CENTER", BAR_ALIGN_RIGHT_CENTER);
	map("BAR_ALIGN_BOTTOM_BOTTOM", BAR_ALIGN_BOTTOM_BOTTOM);
	map("BAR_ALIGN_BOTTOM_CENTER", BAR_ALIGN_BOTTOM_CENTER);
	map("BAR_ALIGN_BOTTOM_TOP", BAR_ALIGN_BOTTOM_TOP);

	return BAR_ALIGN_NONE;
}

int
parse_click(const char *string)
{
	map("ClkLtSymbol", ClkLtSymbol);
	map("ClkWinTitle", ClkWinTitle);
	map("ClkStatusText", ClkStatusText);
	map("ClkClientWin", ClkClientWin);
	map("ClkRootWin", ClkRootWin);
	map("ClkWorkspaceBar", ClkWorkspaceBar);

	return 0;
}

ArgFunc
parse_functionailty(const char *string)
{
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

	fprintf(stderr, "Warning: config could not find function with name %s\n", string);
	return 0;
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
	map("KeyPress", KeyPress);
	map("KeyRelease", KeyRelease);
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
		if (!strcasecmp(string, layouts[i].name)) {
			return i;
		}
	}

	return 0;
}

int
parse_layout_split(const char *string)
{
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
	map("SPLIT_VERTICAL", SPLIT_VERTICAL);
	map("SPLIT_HORIZONTAL", SPLIT_HORIZONTAL);
	map("SPLIT_CENTERED_VERTICAL", SPLIT_CENTERED_VERTICAL);
	map("SPLIT_CENTERED_HORIZONTAL", SPLIT_CENTERED_HORIZONTAL);
	map("SPLIT_VERTICAL_DUAL_STACK", SPLIT_VERTICAL_DUAL_STACK);
	map("SPLIT_HORIZONTAL_DUAL_STACK", SPLIT_HORIZONTAL_DUAL_STACK);
	map("FLOATING_MASTER", FLOATING_MASTER);
	map("SPLIT_VERTICAL_FIXED", SPLIT_VERTICAL_FIXED);
	map("SPLIT_HORIZONTAL_FIXED", SPLIT_HORIZONTAL_FIXED);
	map("SPLIT_CENTERED_VERTICAL_FIXED", SPLIT_CENTERED_VERTICAL_FIXED);
	map("SPLIT_CENTERED_HORIZONTAL_FIXED", SPLIT_CENTERED_HORIZONTAL_FIXED);
	map("SPLIT_VERTICAL_DUAL_STACK_FIXED", SPLIT_VERTICAL_DUAL_STACK_FIXED);
	map("SPLIT_HORIZONTAL_DUAL_STACK_FIXED", SPLIT_HORIZONTAL_DUAL_STACK_FIXED);
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
	int i;
	unsigned int mask = 0;
	char buffer[256];
	strlcpy(buffer, string, LENGTH(buffer));
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
	map("SchemeNorm", SchemeNorm);
	map("Norm", SchemeNorm);
	map("SchemeSel", SchemeSel);
	map("Sel", SchemeSel);
	map("SchemeTitleNorm", SchemeTitleNorm);
	map("TitleNorm", SchemeTitleNorm);
	map("SchemeTitleSel", SchemeTitleSel);
	map("TitleSel", SchemeTitleSel);
	map("SchemeWsNorm", SchemeWsNorm);
	map("WsNorm", SchemeWsNorm);
	map("SchemeWsVisible", SchemeWsVisible);
	map("WsVisible", SchemeWsVisible);
	map("SchemeWsSel", SchemeWsSel);
	map("WsSel", SchemeWsSel);
	map("SchemeWsOcc", SchemeWsOcc);
	map("WsOcc", SchemeWsOcc);
	map("SchemeScratchSel", SchemeScratchSel);
	map("ScratchSel", SchemeScratchSel);
	map("SchemeScratchNorm", SchemeScratchNorm);
	map("ScratchNorm", SchemeScratchNorm);
	map("SchemeHidSel", SchemeHidSel);
	map("HidSel", SchemeHidSel);
	map("SchemeHidNorm", SchemeHidNorm);
	map("HidNorm", SchemeHidNorm);
	map("SchemeUrg", SchemeUrg);
	map("Urg", SchemeUrg);
	map("SchemeMarked", SchemeMarked);
	map("Marked", SchemeMarked);

	fprintf(stderr, "Warning: config could not find color scheme with name %s\n", string);
	return SchemeNorm;
}

int
parse_stacker_placement(const char *string)
{
	map("RightOfWindowIcon", StackerRightOfWindowIcon);
	map("LeftOfWindowIcon", StackerLeftOfWindowIcon);
	map("TitlePrefix", StackerTitlePrefix);
	map("TitleSuffix", StackerTitleSuffix);
	map("TitleEllipsis", StackerTitleEllipsis);
	map("StackerRightOfWindowIcon", StackerRightOfWindowIcon);
	map("StackerLeftOfWindowIcon", StackerLeftOfWindowIcon);
	map("StackerTitlePrefix", StackerTitlePrefix);
	map("StackerTitleSuffix", StackerTitleSuffix);
	map("StackerTitleEllipsis", StackerTitleEllipsis);

	fprintf(stderr, "Warning: config could not find stacker placement with name %s\n", string);
	return StackerTitlePrefix;
}

int
parse_value(const char *string)
{
	map("None", PwrlNone);
	map("RightArrow", PwrlRightArrow);
	map("LeftArrow", PwrlLeftArrow);
	map("ForwardSlash", PwrlForwardSlash);
	map("Backslash", PwrlBackslash);
	map("Solid", PwrlSolid);
	map("SolidRev", PwrlSolidRev);
	map("PwrlNone", PwrlNone);
	map("PwrlRightArrow", PwrlRightArrow);
	map("PwrlLeftArrow", PwrlLeftArrow);
	map("PwrlForwardSlash", PwrlForwardSlash);
	map("PwrlBackslash", PwrlBackslash);
	map("PwrlSolid", PwrlSolid);
	map("PwrlSolidRev", PwrlSolidRev);

	fprintf(stderr, "Warning: config could not find powerline option with name %s\n", string);
	return atoi(string);
}

const char *
parse_window_type(const char *string)
{
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
		if (!strcasecmp(string, commands[i].name)) {
			return commands[i].argv;
		}
	}

	map("dmenucmd", dmenucmd);
	map("termcmd", termcmd);

	for (i = 0; i < num_cached_strings; i++) {
		if (!strcmp(string, cached_strings_array[i])) {
			return cached_strings_array[i];
		}
	}

	if (num_cached_strings + 1 > LENGTH(cached_strings_array)) {
		fprintf(stderr, "Warning: config could not cache new void reference with name %s\n", string);
		return strdup(string);
	}

	cached_strings_array[num_cached_strings] = strdup(string);

	return cached_strings_array[num_cached_strings++];
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
parse_function_int_constant(const char *string, ArgFunc func, int *ptr)
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

	if (func == markall) {
		map("ALL", MARKALL_ALL);
		map("FLOATING", MARKALL_FLOATING);
		map("HIDDEN", MARKALL_HIDDEN);
		map("TILED", MARKALL_TILED);
		map("MARKALL_ALL", MARKALL_ALL);
		map("MARKALL_FLOATING", MARKALL_FLOATING);
		map("MARKALL_HIDDEN", MARKALL_HIDDEN);
		map("MARKALL_TILED", MARKALL_TILED);
		return 0;
	}

	if (func == focusdir || func == placedir) {
		map("LEFT", LEFT);
		map("RIGHT", RIGHT);
		map("UP", UP);
		map("DOWN", DOWN);
		return 0;
	}

	return 0;
}

#undef map
