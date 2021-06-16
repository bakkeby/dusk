static int restartsig = 0;

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
