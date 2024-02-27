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

	char *type;
	XrmValue ret;

	XrmGetResource(xrdb, name, "*", &type, &ret);
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

	int s, c, colorscheme;
	char resource[40];
	const char *resource_prefix;
	char *pattern = "dusk.%s.%s.%s";

	char fg[20] = {0};
	char bg[20] = {0};
	char bd[20] = {0};
	unsigned int alpha[] = { 0, 0, 0 };
	char *clrnames[3] = { fg, bg, bd };
	const char *columns[] = { "fg", "bg", "border" };

	char tnfg[20] = {0};
	char tnbg[20] = {0};
	char tnbd[20] = {0};
	char *titlenorm[3] = { tnfg, tnbg, tnbd };

	char tsfg[20] = {0};
	char tsbg[20] = {0};
	char tsbd[20] = {0};
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
					resource_prefix = colors[s][ColCount] ? colors[s][ColCount] : default_resource_prefixes[s];
					/* Skip schemes that do not specify a resource string */
					if (!resource_prefix || resource_prefix[0] == '\0') {
						continue;
					}
					for (c = 0; c < ColCount; c++) {
						sprintf(resource, pattern, resource_prefix, columns[c], "alpha");
						if (!loadxrdbalpha(xrdb, &alpha[c], resource))
							alpha[c] = default_alphas[c];
						sprintf(resource, pattern, resource_prefix, columns[c], "color");
						if (!loadxrdbcolor(xrdb, &clrnames[c], &alpha[c], resource)) {
							colorscheme = s;
							/* Fall back to SchemeTitleNorm / Sel for SchemeFlex colors if not defined. */
							if (!colors[s][0]) {
								colorscheme = (s >= SchemeFlexSelTTB ? SchemeTitleSel : SchemeTitleNorm);
							}
							if (colorscheme == SchemeTitleNorm && titlenorm[0][0]) {
								strcpy(clrnames[c], titlenorm[c]);
							} else if (colorscheme == SchemeTitleSel && titlesel[0][0]) {
								strcpy(clrnames[c], titlesel[c]);
							} else {
								strcpy(clrnames[c], colors[colorscheme][c]);
							}
						}
					}

					if (s == SchemeTitleNorm) {
						for (c = 0; c < ColCount; c++) {
							strcpy(titlenorm[c], clrnames[c]);
						}
					} else if (s == SchemeTitleSel) {
						for (c = 0; c < ColCount; c++) {
							strcpy(titlesel[c], clrnames[c]);
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
	arrange(NULL);
	focus(NULL);
}
