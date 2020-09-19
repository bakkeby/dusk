void
nametag(const Arg *arg)
{
	char *p, name[MAX_TAGLEN];
	FILE *f;
	int i;

	errno = 0; // popen(3p) says on failure it "may" set errno
	if (!(f = popen("dmenu < /dev/null", "r"))) {
		fprintf(stderr, "dwm: popen 'dmenu < /dev/null' failed%s%s\n", errno ? ": " : "", errno ? strerror(errno) : "");
		return;
	}
	if (!(p = fgets(name, MAX_TAGLEN, f)) && (i = errno) && ferror(f))
		fprintf(stderr, "dwm: fgets failed: %s\n", strerror(i));
	if (pclose(f) < 0)
		fprintf(stderr, "dwm: pclose failed: %s\n", strerror(errno));
	if (!p)
		return;
	if ((p = strchr(name, '\n')))
		*p = '\0';

	for (i = 0; i < NUMTAGS; i++)
		if (selmon->tagset[selmon->seltags] & (1 << i))
			strcpy(tags[i], name);
	drawbars();
}