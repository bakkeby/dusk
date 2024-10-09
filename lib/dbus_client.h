#include "dbus.h"

#define FUNCALIAS(NAME, FUNC, ...) \
	{ NAME, NULL, (int[]){__VA_ARGS__}, sizeof((int[]){__VA_ARGS__}) / sizeof(int) }

#define FUNCPARAM(FUNC, ...) \
	{ #FUNC, NULL, (int[]){__VA_ARGS__}, sizeof((int[]){__VA_ARGS__}) / sizeof(int) }

typedef struct IPCCommand {
	char *name;
	char *unused;
	int *args;
	size_t argc;
} IPCCommand;
