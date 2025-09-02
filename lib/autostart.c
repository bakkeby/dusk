/* execute command from autostart array */
static int autostart_startup = 1;
static pid_t *autostart_pids = NULL;
static int num_autostart_pids = 0;

void
autostart_exec(void)
{
	int i;

	if (autostart_pids == NULL) {
		autostart_init_pids();
	}

	if (autostart_startup) {
		for (i = 0; i < num_autostart; i++) {
			autostart_addpid(spawncmd(&((Arg) { .v = _cfg_autostart[i] }), 0, 1));
		}
	} else {
		for (i = 0; i < num_autorestart; i++) {
			autostart_addpid(spawncmd(&((Arg) { .v = _cfg_autorestart[i] }), 0, 1));
		}
	}
}

void
autostart_init_pids(void)
{
	int i, num_autostart_pids;
	num_autostart_pids = num_autostart + num_autorestart + 10;
	autostart_pids = ecalloc(num_autostart_pids, sizeof(pid_t));
	for (i = 0; i < num_autostart_pids; i++) {
		autostart_pids[i] = 0;
	}
}

void
autostart_addpid(pid_t pid)
{
	unsigned int i;
	for (i = 0; i < num_autostart_pids; i++) {
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
	for (i = 0; i < num_autostart_pids; i++) {
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
	for (i = 0; i < num_autostart_pids; i++) {
		if (autostart_pids[i] == 0)
			break;

		if (autostart_pids[i] == -1)
			continue;

		kill(autostart_pids[i], SIGTERM);
		waitpid(autostart_pids[i], NULL, 0);
	}
}

void
autostart_cleanup(void)
{
	free(autostart_pids);
}
