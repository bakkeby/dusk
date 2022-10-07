int
loadxrdbcolor(XrmDatabase xrdb, char **dest, unsigned int *alpha, char *resource)
{
	XrmValue value;
	char *type;
	unsigned int rgb, a;

	if (XrmGetResource(xrdb, resource, NULL, &type, &value) != True)
		return 0;

	if (value.addr == NULL)
		return 0;

	strcpy(*dest, value.addr);
	switch(sscanf(value.addr, "#%6x%2x", &rgb, &a)) {
		case 1:
			sprintf(*dest, "#%.6x", rgb);
			return 1;
		case 2:
			sprintf(*dest, "#%.6x", rgb);
			*alpha = a;
			return 1;
	}
	return 1;
}

int
loadxrdbalpha(XrmDatabase xrdb, unsigned int *alpha, char *resource)
{
	XrmValue value;
	char *type;

	if (XrmGetResource(xrdb, resource, NULL, &type, &value) != True)
		return 0;

	if (value.addr == NULL)
		return 0;

	*alpha = atoi(value.addr);
	return 1;
}

void
loadxrdbconfig(XrmDatabase xrdb, char *name, enum resource_type rtype, void *dst)
{
	char *sdst = NULL;
	int *idst = NULL;
	float *fdst = NULL;

	sdst = dst;
	idst = dst;
	fdst = dst;

	char fullname[256];
	char *type;
	XrmValue ret;

	snprintf(fullname, sizeof(fullname), "%s.%s", "dusk", name);
	fullname[sizeof(fullname) - 1] = '\0';

	XrmGetResource(xrdb, fullname, "*", &type, &ret);
	if (!(ret.addr == NULL || strncmp("String", type, 64)))
	{
		switch (rtype) {
		case STRING:
			strcpy(sdst, ret.addr);
			break;
		case INTEGER:
			*idst = strtoul(ret.addr, NULL, 10);
			break;
		case FLOAT:
			*fdst = strtof(ret.addr, NULL);
			break;
		}
	}
}

void
loadxrdb()
{
	Display *display;
	char * resm;
	XrmDatabase xrdb;
	const ResourcePref *p;

	int s, c;
	char resource[40];
	char *pattern = "dusk.%s.%s.%s";
	char *oldpattern = "dusk.%s%s%s";

	char fg[20] = {0};
	char bg[20] = {0};
	char bd[20] = {0};
	unsigned int alpha[] = { 0, 0, 0 };
	char *clrnames[3] = { fg, bg, bd };
	const char *columns[] = { "fg", "bg", "border" };

	if (disabled(Xresources))
		return;

	display = XOpenDisplay(NULL);

	if (display != NULL) {
		resm = XResourceManagerString(display);

		if (resm != NULL) {
			xrdb = XrmGetStringDatabase(resm);

			if (xrdb != NULL) {
				for (s = 0; s < SchemeLast; s++) {
					/* Skip schemes that do not specify a resource string */
					if (colors[s][ColCount][0] == '\0')
						continue;

					for (c = 0; c < ColCount; c++) {
						sprintf(resource, pattern, colors[s][ColCount], columns[c], "alpha");
						if (!loadxrdbalpha(xrdb, &alpha[c], resource))
							alpha[c] = default_alphas[c];
						sprintf(resource, pattern, colors[s][ColCount], columns[c], "color");
						if (!loadxrdbcolor(xrdb, &clrnames[c], &alpha[c], resource)) {
							/* Temporary backwards compatibility, should be removed at some point
							 * in the future. */
							sprintf(resource, oldpattern, colors[s][ColCount], columns[c], "color");
							if (!loadxrdbcolor(xrdb, &clrnames[c], &alpha[c], resource))
								strcpy(clrnames[c], colors[s][c]);
						}
					}

					free(scheme[s]);
					scheme[s] = drw_scm_create(drw, clrnames, alpha, 3);
				}

				/* status2d terminal colours */
				for (s = 0; s < 16; s++) {
					sprintf(resource, "dusk.color%d", s);
					loadxrdbcolor(xrdb, &termcolor[s], &alpha[0], resource);
				}

				/* other preferences */
				for (p = resources; p < resources + LENGTH(resources); p++)
					loadxrdbconfig(xrdb, p->name, p->type, p->dst);
			}
		}
	}

	XCloseDisplay(display);
}

void
xrdb(const Arg *arg)
{
	loadxrdb();
	setbackground();
	focus(NULL);
	arrange(NULL);
}
