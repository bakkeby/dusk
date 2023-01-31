/* execute command from autostart array */
static int autostart_startup = 1;
static pid_t autostart_pids[LENGTH(autostart) + LENGTH(autorestart) + 10] = {0};

void
autostart_exec(void)
{
	const char *const *p;
	pid_t pid;

	for (p = autostart_startup ? autostart : autorestart; *p; p++) {
		if ((pid = fork()) == 0) {
			setsid();
			execvp(*p, (char *const *)p);
			fprintf(stderr, "dusk: execvp %s\n", *p);
			perror(" failed");
			_exit(EXIT_FAILURE);
		}

		autostart_addpid(pid);

		/* skip arguments */
		while (*++p);
	}
}

void
autostart_addpid(pid_t pid)
{
	unsigned int i;
	for (i = 0; i < LENGTH(autostart_pids); i++) {
		if (autostart_pids[i] <= 0) {
			autostart_pids[i] = pid;
			break;
		}
	}
}

void
autostart_removepid(pid_t pid)
{
	unsigned int i;
	for (i = 0; i < LENGTH(autostart_pids); i++) {
		if (autostart_pids[i] == 0)
			break;

		if (autostart_pids[i] == pid) {
			autostart_pids[i] = -1;
			break;
		}
	}
}

void
autostart_killpids(void)
{
	unsigned int i;
	for (i = 0; i < LENGTH(autostart_pids); i++) {
		if (autostart_pids[i] == 0)
			break;

		if (autostart_pids[i] == -1)
			continue;

		kill(autostart_pids[i], SIGTERM);
		waitpid(autostart_pids[i], NULL, 0);
	}
}
