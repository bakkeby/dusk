/* execute command from autostart array */
static int autostart_startup = 1;

static void
autostart_exec(void)
{
	const char *const *p;

	for (p = autostart_startup ? autostart : autorestart; *p; p++) {
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
