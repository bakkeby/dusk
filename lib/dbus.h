#include <dbus/dbus.h>
#include <stdio.h>
#include <stdlib.h>

#define DBUS_TYPE_NONE ((int) '0')

static void initialise_dbus(void);
static void cleanup_dbus(void);
static char *dbus_type_to_string(int dbus_type);
static char *prepare_dbus_name(char *dbus_name, char *base_name);
static void send_dbus_message(DBusMessage *msg, int wait_for_reply);
static DBusMessage * create_dbus_message(
	const char *destination,
	const char *object_path,
	const char *interface_name,
	const char *method_name
);
