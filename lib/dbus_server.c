#include "dbus.c"
#include "yajl_dumps.c"

void
handle_dbus_message(DBusMessage *msg)
{
	IPCCommand *command;
	Arg arg;
	char *str_param;
	double dbl_param;
	int int_param;

	const char *method = dbus_message_get_member(msg);

	if (method == NULL) {
		reply_with_formatted_message(msg, "Error: No method passed\n");
		return;
	}

	command = lookup_command(method);

	if (command == NULL) {
		reply_with_formatted_message(msg, "Error: Command %s not found\n", method);
		return;
	}

	if (command->function == NULL) {
		handle_null_function_command(msg, command);
		return;
	}

	switch (command->argc) {
	case 0:
		arg.v = NULL;
		break;
	case 1:
		switch (command->args[0]) {
		case DBUS_TYPE_NONE:
			arg.v = NULL;
			break;
		case DBUS_TYPE_STRING:
			if (!dbus_message_get_args(msg, NULL, DBUS_TYPE_STRING, &str_param, DBUS_TYPE_INVALID)) {
				reply_with_formatted_message(msg, "Error: Command %s failed to read string argument\n", method);
				return;
			}
			arg.v = str_param;
			break;
		case DBUS_TYPE_DOUBLE:
			if (!dbus_message_get_args(msg, NULL, DBUS_TYPE_DOUBLE, &dbl_param, DBUS_TYPE_INVALID)) {
				reply_with_formatted_message(msg, "Error: Command %s failed to read double argument\n", method);
				return;
			}
			arg.f = dbl_param;
			break;
		case DBUS_TYPE_INT32:
			if (!dbus_message_get_args(msg, NULL, DBUS_TYPE_INT32, &int_param, DBUS_TYPE_INVALID)) {
				reply_with_formatted_message(msg, "Error: Command %s failed to read int argument\n", method);
				return;
			}
			arg.i = int_param;
			break;
		default:
			reply_with_formatted_message(msg, "Error: Command %s unknown argument type %d\n", method, command->args[0]);
			return;
		}
		break;
	default:
		arg.v = msg;
		break;
	}

	command->function(&arg);
	reply_with_message(msg, "OK");
}

int
register_dbus(char *request_name, int *dbus_fd)
{
	int ret;

	init_dbus();
	ret = dbus_bus_request_name(dbus_conn, request_name, DBUS_NAME_FLAG_REPLACE_EXISTING, &err);

	if (dbus_error_is_set(&err)) {
		fprintf(stderr, "Name Error (%s)\n", err.message);
		dbus_error_free(&err);
	}

	if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER && ret != DBUS_REQUEST_NAME_REPLY_ALREADY_OWNER) {
		fprintf(stderr, "Not primary owner (%d)\n", ret);
		return 0;
	}

	/* Get the file descriptor for D-Bus */
	dbus_connection_get_unix_fd(dbus_conn, dbus_fd);

	if (dbus_error_is_set(&err)) {
	    fprintf(stderr, "D-Bus FD Error: %s\n", err.message);
	    dbus_error_free(&err);
	    return 0;
	}

	if (enabled(Debug))
		fprintf(stderr, "Registered dbus name %s on fd %d\n", request_name, *dbus_fd);

	return 1;
}

void
reply_with_formatted_message(DBusMessage *msg, const char *text, ...)
{
	char response_buffer[256] = {0};

	va_list args;
	va_start(args, 0);
	vsnprintf(response_buffer, sizeof(response_buffer) - 1, text, args);
	va_end(args);

	reply_with_message(msg, response_buffer);
}

void
reply_with_message(DBusMessage *msg, const char *text)
{
	if (dbus_message_get_no_reply(msg))
		return;

	DBusMessage *response = dbus_message_new_method_return(msg);
	if (response == NULL)
		return;

	dbus_message_append_args(response, DBUS_TYPE_STRING, &text, DBUS_TYPE_INVALID);
	if (!dbus_connection_send(dbus_conn, response, NULL)) {
		fprintf(stderr, "Error: failed to send dbus message response; %s\n", text);
	}

	dbus_message_unref(response);
}

void
setstatus_dbus(const Arg *arg)
{
	DBusMessage *msg = (DBusMessage*)arg->v;
	int status_id;
	char *status_str;
	if (!dbus_message_get_args(msg, NULL,
			DBUS_TYPE_INT32, &status_id,
			DBUS_TYPE_STRING, &status_str, DBUS_TYPE_INVALID)) {
		reply_with_formatted_message(msg, "Error: Command setstatus failed to read int and string arguments\n");
		return;
	}
	setstatus(status_id, status_str);
}

void
setwintitle_dbus(const Arg *arg)
{
	DBusMessage *msg = (DBusMessage*)arg->v;
	int window_id;
	char *title_str;
	if (!dbus_message_get_args(msg, NULL,
			DBUS_TYPE_INT32, &window_id,
			DBUS_TYPE_STRING, &title_str, DBUS_TYPE_INVALID)) {
		reply_with_formatted_message(msg, "Error: Command setwintitle failed to read int and string arguments\n");
		return;
	}
	setwintitle(window_id, title_str);
}

void
customlayout_dbus(const Arg *arg)
{
	DBusMessage *msg = (DBusMessage*)arg->v;
	int ws_num, nmaster, nstack, split, master_axis, stack_axis, secondary_stack_axis;
	char *symbol;
	if (!dbus_message_get_args(msg, NULL,
			DBUS_TYPE_INT32, &ws_num,
			DBUS_TYPE_STRING, &symbol,
			DBUS_TYPE_INT32, &nmaster,
			DBUS_TYPE_INT32, &nstack,
			DBUS_TYPE_INT32, &split,
			DBUS_TYPE_INT32, &master_axis,
			DBUS_TYPE_INT32, &stack_axis,
			DBUS_TYPE_INT32, &secondary_stack_axis,
			DBUS_TYPE_INVALID)
	) {
		reply_with_formatted_message(msg, "Error: Command customlayout failed to read arguments\n");
		return;
	}
	customlayout(ws_num, symbol, nmaster, nstack, split, master_axis, stack_axis, secondary_stack_axis);
}

void
handle_null_function_command(DBusMessage *msg, IPCCommand *command)
{
	Client *c;
	int int_param;
	const char *method = command->name;

	yajl_gen gen = yajl_gen_alloc(NULL);
	yajl_gen_config(gen, yajl_gen_beautify, 1);

	if (!strcasecmp("get_monitors", method)) {
		dump_monitors(gen, mons, selmon);
	} else if (!strcasecmp("get_layouts", method)) {
		dump_layouts(gen, layouts, LENGTH(layouts));
	} else if (!strcasecmp("get_client", method)) {
		if (!dbus_message_get_args(msg, NULL, DBUS_TYPE_INT32, &int_param, DBUS_TYPE_INVALID)) {
			reply_with_formatted_message(msg, "Error: Command get_client failed to read int argument\n");
			goto end;
		}
		c = wintoclient(int_param);
		if (!c) {
			reply_with_message(msg, "Error: get_client - no such window found\n");
			goto end;
		}
		dump_client(gen, c);
	} else if (!strcasecmp("get_bar_height", method)) {
		dump_bar_height(gen);
	} else if (!strcasecmp("get_workspaces", method)) {
		dump_workspaces(gen);
	} else if (!strcasecmp("get_settings", method)) {
		dump_settings(gen);
	} else if (!strcasecmp("get_commands", method)) {
		dump_commands(gen);
	} else if (!strcasecmp("get_systray_windows", method)) {
		dump_systray_icons(gen);
	} else {
		fprintf(stderr, "Unknown command %s\n", method);
		goto end;
	}

	const unsigned char *buffer;
	size_t len = 0;

	yajl_gen_get_buf(gen, &buffer, &len);
	len++; // for null char

	reply_with_message(msg, (const char *)buffer);
end:
	yajl_gen_free(gen);
}
