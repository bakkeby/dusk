#define BUFFERSIZE 60

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

	strlcpy(*dest, value.addr, BUFFERSIZE);
	switch(sscanf(value.addr, "#%6x%2x", &rgb, &a)) {
		case 1:
			snprintf(*dest, BUFFERSIZE, "#%.6x", rgb);
			return 1;
		case 2:
			snprintf(*dest, BUFFERSIZE, "#%.6x", rgb);
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
loadxrdbconfig(XrmDatabase xrdb, char *name, enum resource_type rtype, void *dst, int dst_size)
{
	char *sdst = NULL;
	int *idst = NULL;
	float *fdst = NULL;

	sdst = dst;
	idst = dst;
	fdst = dst;

	char *type;
	XrmValue ret;

	if (!dst_size) {
		dst_size = BUFFERSIZE;
	}

	XrmGetResource(xrdb, name, "*", &type, &ret);
	if (!(ret.addr == NULL || strncmp("String", type, 64)))
	{
		switch (rtype) {
		case STRING:
			strlcpy(sdst, ret.addr, dst_size);
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
loadxrdb(void)
{
	Display *display;
	char * resm;
	XrmDatabase xrdb;
	const ResourcePref *p;

	int s, c, colorscheme;
	char resource[40];
	const char *resource_prefix;
	char *pattern = "dusk.%s.%s.%s";

	char fg[BUFFERSIZE] = {0};
	char bg[BUFFERSIZE] = {0};
	char bd[BUFFERSIZE] = {0};
	unsigned int alpha[] = { 0, 0, 0 };
	char *clrnames[3] = { fg, bg, bd };
	const char *columns[] = { "fg", "bg", "border" };

	char tnfg[BUFFERSIZE] = {0};
	char tnbg[BUFFERSIZE] = {0};
	char tnbd[BUFFERSIZE] = {0};
	char *titlenorm[3] = { tnfg, tnbg, tnbd };

	char tsfg[BUFFERSIZE] = {0};
	char tsbg[BUFFERSIZE] = {0};
	char tsbd[BUFFERSIZE] = {0};
	char *titlesel[3] = { tsfg, tsbg, tsbd };

	if (disabled(Xresources))
		return;

	display = XOpenDisplay(NULL);

	if (display != NULL) {
		resm = XResourceManagerString(display);

		if (resm != NULL) {
			xrdb = XrmGetStringDatabase(resm);

			if (xrdb != NULL) {
				for (s = 0; s < SchemeLast; s++) {
					resource_prefix = _cfg_colors[s][ColCount] ? _cfg_colors[s][ColCount] : default_resource_prefixes[s];
					/* Skip schemes that do not specify a resource string */
					if (!resource_prefix || resource_prefix[0] == '\0') {
						continue;
					}
					for (c = 0; c < ColCount; c++) {
						snprintf(resource, sizeof resource, pattern, resource_prefix, columns[c], "alpha");
						if (!loadxrdbalpha(xrdb, &alpha[c], resource))
							alpha[c] = default_alphas[c];
						snprintf(resource, sizeof resource, pattern, resource_prefix, columns[c], "color");
						if (!loadxrdbcolor(xrdb, &clrnames[c], &alpha[c], resource)) {
							colorscheme = s;
							/* Fall back to SchemeTitleNorm / Sel for SchemeFlex colors if not defined. */
							if (!_cfg_colors[s][0]) {
								colorscheme = (s >= SchemeFlexSelTTB ? SchemeTitleSel : SchemeTitleNorm);
							}
							if (colorscheme == SchemeTitleNorm && titlenorm[0][0]) {
								strlcpy(clrnames[c], titlenorm[c], BUFFERSIZE);
							} else if (colorscheme == SchemeTitleSel && titlesel[0][0]) {
								strlcpy(clrnames[c], titlesel[c], BUFFERSIZE);
							} else {
								strlcpy(clrnames[c], _cfg_colors[colorscheme][c], BUFFERSIZE);
							}
						}
					}

					if (s == SchemeTitleNorm) {
						for (c = 0; c < ColCount; c++) {
							strlcpy(titlenorm[c], clrnames[c], BUFFERSIZE);
						}
					} else if (s == SchemeTitleSel) {
						for (c = 0; c < ColCount; c++) {
							strlcpy(titlesel[c], clrnames[c], BUFFERSIZE);
						}
					}

					free(scheme[s]);
					scheme[s] = drw_scm_create(drw, clrnames, alpha, 3);
				}

				/* status2d terminal colours */
				for (s = 0; s < 16; s++) {
					snprintf(resource, sizeof resource, "dusk.color%d", s);
					loadxrdbcolor(xrdb, &termcolor[s], &alpha[0], resource);
				}

				/* other preferences */
				for (p = resources; p < resources + LENGTH(resources); p++)
					loadxrdbconfig(xrdb, p->name, p->type, p->dst, p->dst_size);
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
	if (systray)
		XMoveWindow(dpy, systray->win, -10000, -10000);
	arrange(NULL);
	focus(NULL);
}
