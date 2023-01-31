static int restartsig = 0;

void
sigchld(int unused)
{
	pid_t pid;

	while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
		autostart_removepid(pid);
	}
}

void
sighup(int unused)
{
	restart(NULL);
}

void
sigterm(int unused)
{
	quit(NULL);
}
