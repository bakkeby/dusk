/* execute command from autostart array */
static int run_autostart = 1;

static void
autostart_exec()
{
	const char *const *p;

	for (p = autostart; *p; p++) {
		if (fork() == 0) {
			setsid();
			execvp(*p, (char *const *)p);
			fprintf(stderr, "dusk: execvp %s\n", *p);
			perror(" failed");
			_exit(EXIT_FAILURE);
		}
		/* skip arguments */
		while (*++p);
	}
}
