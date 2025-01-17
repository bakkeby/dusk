static DBusError err;
static DBusConnection *dbus_conn;
static int dbus_init = 0;

static char *dbus_base_name = "wm.dusk";

static IPCCommand ipccommands[] = {
	FUNCPARAM( changeopacity, DBUS_TYPE_DOUBLE ),
	FUNCPARAM( clienttomon, DBUS_TYPE_INT32 ),
	FUNCPARAM( clientstomon, DBUS_TYPE_INT32 ),
	FUNCALIAS( "customlayout", customlayout_dbus, DBUS_TYPE_INT32, DBUS_TYPE_STRING, DBUS_TYPE_INT32, DBUS_TYPE_INT32, DBUS_TYPE_INT32, DBUS_TYPE_INT32, DBUS_TYPE_INT32, DBUS_TYPE_INT32),
	FUNCPARAM( cyclelayout, DBUS_TYPE_INT32 ),
	FUNCPARAM( enable, DBUS_TYPE_STRING ),
	FUNCPARAM( enablewsbyindex, DBUS_TYPE_INT32 ),
	FUNCPARAM( enablewsbyname, DBUS_TYPE_STRING ),
	FUNCPARAM( defaultgaps, DBUS_TYPE_NONE ),
	FUNCPARAM( disable, DBUS_TYPE_STRING ),
	FUNCPARAM( floatpos, DBUS_TYPE_STRING ),
	FUNCPARAM( focusdir, DBUS_TYPE_INT32 ),
	FUNCPARAM( focushidden, DBUS_TYPE_INT32 ),
	FUNCPARAM( focusmaster, DBUS_TYPE_NONE ),
	FUNCPARAM( focusmon, DBUS_TYPE_INT32 ),
	FUNCPARAM( focusstack, DBUS_TYPE_INT32 ),
	FUNCPARAM( focusurgent, DBUS_TYPE_NONE ),
	FUNCALIAS( "get_monitors", NULL, DBUS_TYPE_NONE ),
	FUNCALIAS( "get_layouts", NULL, DBUS_TYPE_NONE ),
	FUNCALIAS( "get_client", NULL, DBUS_TYPE_INT32 ),
	FUNCALIAS( "get_bar_height", NULL, DBUS_TYPE_NONE ),
	FUNCALIAS( "get_workspaces", NULL, DBUS_TYPE_NONE ),
	FUNCALIAS( "get_settings", NULL, DBUS_TYPE_NONE ),
	FUNCALIAS( "get_commands", NULL, DBUS_TYPE_NONE ),
	FUNCALIAS( "get_systray_windows", NULL, DBUS_TYPE_NONE ),
	FUNCPARAM( hidebar, DBUS_TYPE_NONE ),
	FUNCPARAM( incrgaps, DBUS_TYPE_INT32 ),
	FUNCPARAM( incrigaps, DBUS_TYPE_INT32 ),
	FUNCPARAM( incrogaps, DBUS_TYPE_INT32 ),
	FUNCPARAM( incrihgaps, DBUS_TYPE_INT32 ),
	FUNCPARAM( incrivgaps, DBUS_TYPE_INT32 ),
	FUNCPARAM( incrohgaps, DBUS_TYPE_INT32 ),
	FUNCPARAM( incrovgaps, DBUS_TYPE_INT32 ),
	FUNCPARAM( incnmaster, DBUS_TYPE_INT32 ),
	FUNCPARAM( incnstack, DBUS_TYPE_INT32 ),
	FUNCPARAM( inplacerotate, DBUS_TYPE_INT32 ),
	FUNCPARAM( killclient, DBUS_TYPE_NONE ),
	FUNCPARAM( killunsel, DBUS_TYPE_NONE ),
	FUNCPARAM( layoutconvert, DBUS_TYPE_NONE ),
	FUNCPARAM( mark, DBUS_TYPE_NONE ),
	FUNCPARAM( markall, DBUS_TYPE_INT32 ), // 0 = mark all, 1 = mark floating, 2 = mark hidden
	FUNCPARAM( maximize, DBUS_TYPE_NONE ),
	FUNCPARAM( maximizevert, DBUS_TYPE_NONE ),
	FUNCPARAM( maximizehorz, DBUS_TYPE_NONE ),
	FUNCPARAM( mirrorlayout, DBUS_TYPE_NONE ),
	FUNCPARAM( movetowsbyindex, DBUS_TYPE_INT32 ),
	FUNCPARAM( movetowsbyname, DBUS_TYPE_STRING ),
	FUNCPARAM( sendtowsbyindex, DBUS_TYPE_INT32 ),
	FUNCPARAM( sendtowsbyname, DBUS_TYPE_STRING ),
	FUNCPARAM( movealltowsbyindex, DBUS_TYPE_INT32 ),
	FUNCPARAM( movealltowsbyname, DBUS_TYPE_STRING ),
	FUNCPARAM( moveallfromwsbyindex, DBUS_TYPE_INT32 ),
	FUNCPARAM( moveallfromwsbyname, DBUS_TYPE_STRING ),
	FUNCPARAM( movewsdir, DBUS_TYPE_INT32 ),
	FUNCPARAM( rotatelayoutaxis, DBUS_TYPE_INT32 ),
	FUNCPARAM( rotatestack, DBUS_TYPE_INT32 ),
	FUNCPARAM( placedir, DBUS_TYPE_INT32 ),
	FUNCPARAM( pushdown, DBUS_TYPE_NONE ),
	FUNCPARAM( pushup, DBUS_TYPE_NONE ),
	FUNCPARAM( quit, DBUS_TYPE_NONE ),
	FUNCPARAM( restart, DBUS_TYPE_NONE ),
	FUNCPARAM( rioresize, DBUS_TYPE_NONE ),
	FUNCPARAM( setattachdefault, DBUS_TYPE_STRING),
	FUNCPARAM( setborderpx, DBUS_TYPE_INT32 ),
	FUNCPARAM( setclientborderpx, DBUS_TYPE_INT32 ),
	FUNCPARAM( setlayoutaxisex, DBUS_TYPE_INT32 ),
	FUNCPARAM( setlayout, DBUS_TYPE_INT32 ),
	FUNCPARAM( setcfact, DBUS_TYPE_DOUBLE ),
	FUNCPARAM( setmfact, DBUS_TYPE_DOUBLE ),
	FUNCPARAM( setwfact, DBUS_TYPE_DOUBLE ),
	FUNCPARAM( setgapsex, DBUS_TYPE_INT32 ),
	FUNCALIAS( "setstatus", setstatus_dbus, DBUS_TYPE_INT32, DBUS_TYPE_STRING),
	FUNCPARAM( settitle, DBUS_TYPE_STRING ),
	FUNCALIAS( "setwintitle", setwintitle_dbus, DBUS_TYPE_INT32, DBUS_TYPE_STRING ),
	FUNCPARAM( showbar, DBUS_TYPE_NONE ),
	FUNCPARAM( showhideclient, DBUS_TYPE_NONE ),
	FUNCPARAM( stackfocus, DBUS_TYPE_INT32 ),
	FUNCPARAM( stackpush, DBUS_TYPE_INT32 ),
	FUNCPARAM( stackswap, DBUS_TYPE_INT32 ),
	FUNCPARAM( swallow, DBUS_TYPE_NONE ),
	FUNCPARAM( switchcol, DBUS_TYPE_NONE ),
	FUNCPARAM( swapwsbyindex, DBUS_TYPE_INT32 ),
	FUNCPARAM( swapwsbyname, DBUS_TYPE_STRING ),
	FUNCPARAM( toggle, DBUS_TYPE_STRING ),
	FUNCPARAM( togglebar, DBUS_TYPE_NONE ),
	FUNCPARAM( togglebarpadding, DBUS_TYPE_NONE ),
	FUNCPARAM( togglecompact, DBUS_TYPE_NONE ),
	FUNCPARAM( toggleclientflag, DBUS_TYPE_STRING ),
	FUNCPARAM( togglefakefullscreen, DBUS_TYPE_NONE ),
	FUNCPARAM( togglefloating, DBUS_TYPE_NONE ),
	FUNCPARAM( togglefullscreen, DBUS_TYPE_NONE ),
	FUNCPARAM( togglegaps, DBUS_TYPE_NONE ),
	FUNCPARAM( togglekeybindings, DBUS_TYPE_NONE ),
	FUNCPARAM( togglemark, DBUS_TYPE_NONE ),
	FUNCPARAM( togglenomodbuttons, DBUS_TYPE_NONE ),
	FUNCPARAM( togglepinnedws, DBUS_TYPE_NONE ),
	FUNCPARAM( togglesticky, DBUS_TYPE_NONE ),
	FUNCPARAM( togglews, DBUS_TYPE_NONE ),
	FUNCPARAM( transfer, DBUS_TYPE_NONE ),
	FUNCPARAM( transferall, DBUS_TYPE_NONE ),
	FUNCPARAM( unfloatvisible, DBUS_TYPE_NONE ),
	FUNCPARAM( unhideall, DBUS_TYPE_NONE ),
	FUNCPARAM( unhidepop, DBUS_TYPE_NONE ),
	FUNCPARAM( unmark, DBUS_TYPE_NONE ),
	FUNCPARAM( unmarkall, DBUS_TYPE_NONE ),
	FUNCPARAM( unswallow, DBUS_TYPE_NONE ),
	FUNCPARAM( viewallwsonmon, DBUS_TYPE_NONE ),
	FUNCPARAM( viewalloccwsonmon, DBUS_TYPE_NONE ),
	FUNCPARAM( viewselws, DBUS_TYPE_NONE ),
	FUNCPARAM( viewwsbyindex, DBUS_TYPE_INT32 ),
	FUNCPARAM( viewwsbyname, DBUS_TYPE_STRING ),
	FUNCPARAM( viewwsdir, DBUS_TYPE_INT32 ),
	FUNCPARAM( xrdb, DBUS_TYPE_NONE ),
	FUNCPARAM( zoom, DBUS_TYPE_NONE ),
};

void
init_dbus(void)
{
	if (dbus_init)
		return;

	dbus_error_init(&err);
	dbus_conn = dbus_bus_get(DBUS_BUS_SESSION, &err);

	if (dbus_error_is_set(&err)) {
		fprintf(stderr, "Connection Error: %s\n", err.message);
		dbus_error_free(&err);
		return;
	}

	if (!dbus_conn) {
		fprintf(stderr, "Failed to connect to the D-Bus session bus\n");
		return;
	}

	dbus_init = 1;
}

void
cleanup_dbus(void)
{
	/* Close dbus connection */
	dbus_connection_unref(dbus_conn);
	dbus_init = 0;
}

/*
 *  .-----------------------.
 *  | DBUS_TYPE_BYTE        |
 *  | DBUS_TYPE_BOOLEAN     |
 *  | DBUS_TYPE_INT16       |
 *  | DBUS_TYPE_UINT16      |
 *  | DBUS_TYPE_INT32       |
 *  | DBUS_TYPE_UINT32      |
 *  | DBUS_TYPE_INT64       |
 *  | DBUS_TYPE_UINT64      |
 *  | DBUS_TYPE_DOUBLE      |
 *  | DBUS_TYPE_STRING      |
 *  | DBUS_TYPE_OBJECT_PATH |
 *  | DBUS_TYPE_SIGNATURE   |
 *  ^-----------------------^
*/
void
add_string_argument(DBusMessage *msg, const char *argument)
{
	DBusMessageIter args;
	dbus_message_iter_init_append(msg, &args);
	dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &argument);
}

void
add_int_argument(DBusMessage *msg, const int argument)
{
	DBusMessageIter args;
	dbus_message_iter_init_append(msg, &args);
	dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &argument);
}

void
add_smallint_argument(DBusMessage *msg, const short argument)
{
	DBusMessageIter args;
	dbus_message_iter_init_append(msg, &args);
	dbus_message_iter_append_basic(&args, DBUS_TYPE_INT16, &argument);
}

void
add_double_argument(DBusMessage *msg, const double argument)
{
	DBusMessageIter args;
	dbus_message_iter_init_append(msg, &args);
	dbus_message_iter_append_basic(&args, DBUS_TYPE_DOUBLE, &argument);
}

char *
dbus_type_to_string(int dbus_type)
{
	switch (dbus_type) {
	case DBUS_TYPE_NONE:
		return "none";
	case DBUS_TYPE_STRING:
		return "string";
	case DBUS_TYPE_DOUBLE:
		return "float";
	case DBUS_TYPE_INT32:
		return "int";
	default:
		return "unknown";
	}
}

DBusMessage *
create_dbus_message(
	const char *destination,
	const char *object_path,
	const char *interface_name,
	const char *method_name
) {
	DBusMessage *msg = dbus_message_new_method_call(
		destination,
		object_path,
		interface_name,
		method_name
	);

	if (!msg) {
		fprintf(stderr, "Failed to create a new message\n");
		exit(1);
	}

	return msg;
}

IPCCommand *
lookup_command(const char *command)
{
	int i;

	if (!command)
		return NULL;

	for (i = 0; i < LENGTH(ipccommands); i++)
		if (!strcasecmp(ipccommands[i].name, command))
			return &ipccommands[i];
	return NULL;
}

void
send_dbus_message(DBusMessage *msg, int wait_for_reply)
{
	DBusMessage *reply;
	char *response;

	if (!wait_for_reply) {
		dbus_uint32_t serial = 0;
		dbus_connection_send(dbus_conn, msg, &serial);
		return;
	}

	/* Send the message and block for the reply */
	reply = dbus_connection_send_with_reply_and_block(dbus_conn, msg, -1, &err);
	if (dbus_error_is_set(&err)) {
		fprintf(stderr, "Error sending message: %s\n", err.message);
		dbus_error_free(&err);
		return;
	}

	/* Read the reply */
	if (!dbus_message_get_args(reply, &err, DBUS_TYPE_STRING, &response, DBUS_TYPE_INVALID)) {
		fprintf(stderr, "Failed to parse reply: %s\n", err.message);
		dbus_error_free(&err);
	} else {
		printf("%s\n", response);
	}

	/* Free the reply message */
	dbus_message_unref(reply);
}

char *
prepare_dbus_name(char *dbus_name, char *base_name)
{
	const char *display = getenv("DISPLAY");

	char hostname[256];
	int display_no = -1;
	int screen_no;

	if (display && sscanf(display, "%255[^:]:%d.%d", hostname, &display_no, &screen_no) < 2)
		sscanf(display, ":%d.%d", &display_no, &screen_no);

	if (display_no != -1) {
		snprintf(dbus_name, 255, "%s.d%d", dbus_base_name, display_no);
	} else {
		strlcpy(dbus_name, base_name, strlen(base_name));
	}

	return dbus_name;
}
