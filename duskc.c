#include <string.h>

#define arg(A) (!strcmp(argv[i], A))
#define LENGTH(X) (sizeof X / sizeof X[0])

#include "util.c"
#include "lib/dbus_client.h"
#include "lib/dbus_client.c"

void
usage(FILE *stream)
{
	fputs(
		"usage: duskc [options] <command> [...]\n\n"
		"Commands:\n"
		"  run_command <name> [args...]    Run an IPC command\n\n"
		"  get_monitors                    Get monitor properties\n\n"
		"  get_layouts                     Get list of layouts\n\n"
		"  get_client <window_id>          Get client properties\n\n"
		"  get_bar_height                  Get the bar height\n\n"
		"  get_workspaces                  Get list of workspaces\n\n"
		"  get_settings                    Get list of settings\n\n"
		"  get_commands                    Get list of commands\n\n"
		"  get_systray_windows             Get list of system tray windows\n\n"
		"  help                            Display this message\n\n"
		"Options:\n"
		"  -q, --ignore-reply              Don't print reply messages from run_command.\n"
		"\n", stream
	);
}

void
usage_run_command(IPCCommand command, FILE *stream, char *format)
{
	int a;
	fprintf(stream, format, command.name);

	for (a = 0; a < command.argc; a++)
		fprintf(stream, " <%s>", dbus_type_to_string(command.args[a]));
	fprintf(stream, "\n");
}

void
usage_run_commands(FILE *stream)
{
	int i;
	fputs("usage: duskc run_command <name> [args...]", stream);
	fprintf(stream, "\n\nAvailable commands:\n");

	char format[] = "   %s";
	for (i = 0; i < LENGTH(ipccommands); i++) {
		usage_run_command(ipccommands[i], stream, format);
	}

	fprintf(stream, "\n");
}

void
parseargs(int argc, char *argv[])
{
	int i, a, scanned, arg_int;
	unsigned int arg_uint;
	double arg_double;
	DBusMessage *msg;
	IPCCommand *command = NULL;
	int wait_for_reply = 1;
	char dbus_name[256];
	prepare_dbus_name(dbus_name, dbus_base_name);
	char *dbus_path = "/wm/dusk";
	char *dbus_interface = "wm.dusk";

	if (argc == 1) {
		usage(stdout);
		exit(0);
	}

	for (i = 1; i < argc; i++) {

		if (arg("--help") || arg("help")) {
			usage(stdout);
			exit(0);
		}

		if (arg("-q") || arg("--ignore-reply")) {
			wait_for_reply = 0;
			continue;
		}

		if arg("run_command") {
			if (i + 1 == argc) {
				usage_run_commands(stdout);
				exit(0);
			}

			command = lookup_command(argv[++i]);
			if (command == NULL) {
				fprintf(stderr, "Error: Command %s not found\n", argv[i]);
				exit(1);
			}
		} else {
			command = lookup_command(argv[i]);
		}

		if (command != NULL) {
			msg = create_dbus_message(dbus_name, dbus_path, dbus_interface, command->name);

			for (a = 0, i++; i < argc && a < command->argc; i++, a++) {
				switch (command->args[a]) {
				case DBUS_TYPE_STRING:
					add_string_argument(msg, argv[i]);
					break;
				case DBUS_TYPE_DOUBLE:
					scanned = sscanf(argv[i], "%lf", &arg_double);

					if (scanned != 1) {
						fprintf(stderr, "Error: Command %s argument %d is not a float: %s\n", command->name, a+1, argv[i]);
						exit(1);
					}
					add_double_argument(msg, arg_double);
					break;
				case DBUS_TYPE_INT32:
					/* If the argument starts with 0x then try to parse it as hex instead of integer. */
					if (startswith("0x", argv[i])) {
						scanned = sscanf(argv[i], "%x", &arg_uint);
						arg_int = arg_uint;
					} else {
						scanned = sscanf(argv[i], "%d", &arg_int);
					}

					if (scanned != 1) {
						fprintf(stderr, "Error: Command %s argument %d is not an integer: %s\n", command->name, a+1, argv[i]);
						exit(1);
					}

					add_int_argument(msg, arg_int);
					break;
				}
			}

			if (a < command->argc && command->args[0] != DBUS_TYPE_NONE) {
				fprintf(stderr, "Error: Not enough arguments for command %s\n", command->name);
				usage_run_command(*command, stderr, "Usage: %s");
				dbus_message_unref(msg);
				exit(1);
			}

			send_dbus_message(msg, wait_for_reply);
			dbus_message_unref(msg);
			break;
		}
	}
}

int
main(int argc, char *argv[])
{
	init_dbus();
	parseargs(argc, argv);
	cleanup_dbus();
	return 0;
}
