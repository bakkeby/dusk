#include "dbus.h"
#include <stdarg.h>

#define FUNCALIAS(NAME, FUNC, ...) \
	{ NAME, FUNC, (int[]){__VA_ARGS__}, sizeof((int[]){__VA_ARGS__}) / sizeof(int) }

#define FUNCPARAM(FUNC, ...) \
	{ #FUNC, FUNC, (int[]){__VA_ARGS__}, sizeof((int[]){__VA_ARGS__}) / sizeof(int) }

typedef struct IPCCommand {
	char *name;
	void (*function)(const Arg *);
	int *args;
	size_t argc;
} IPCCommand;

static int register_dbus(char *request_name, int *dbus_fd);
static void handle_dbus_message(DBusMessage *msg);
static void handle_null_function_command(DBusMessage *msg, IPCCommand *command);
static void reply_with_formatted_message(DBusMessage *msg, const char *text, ...);
static void reply_with_message(DBusMessage *msg, const char *text);
static void setstatus_dbus(const Arg *arg);
static void setwintitle_dbus(const Arg *arg);
static void customlayout_dbus(const Arg *arg);
