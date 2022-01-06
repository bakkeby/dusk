int
loadxrdbcolor(XrmDatabase xrdb, char **dest, char *resource)
{
	XrmValue value;
	char *type;
	if (XrmGetResource(xrdb, resource, NULL, &type, &value) == True) {
		if (value.addr != NULL && strnlen(value.addr, 8) == 7 && value.addr[0] == '#') {
			int i = 1;
			for (; i <= 6; i++) {
				if (value.addr[i] < 48) break;
				if (value.addr[i] > 57 && value.addr[i] < 65) break;
				if (value.addr[i] > 70 && value.addr[i] < 97) break;
				if (value.addr[i] > 102) break;
			}
			if (i == 7) {
				strncpy(*dest, value.addr, 7);
				return 1;
			}
		}
	}
	return 0;
}

void
loadxrdb()
{
	Display *display;
	char * resm;
	XrmDatabase xrdb;

	int s;
	char resource[40];
	char *pattern = "dusk.%s%scolor";

	char fg[] = "#000000";
	char bg[] = "#000000";
	char bd[] = "#000000";
	char *clrnames[3] = { fg, bg, bd };
	char *dmenunames[4] = { dmenunormfgcolor, dmenunormbgcolor, dmenuselfgcolor, dmenuselbgcolor };

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
					if (colors[s][ColResource][0] == '\0')
						continue;

					sprintf(resource, pattern, colors[s][ColResource], "fg");
					if (!loadxrdbcolor(xrdb, &clrnames[ColFg], resource))
						strcpy(clrnames[ColFg], colors[s][ColFg]);

					sprintf(resource, pattern, colors[s][ColResource], "bg");
					if (!loadxrdbcolor(xrdb, &clrnames[ColBg], resource))
						strcpy(clrnames[ColBg], colors[s][ColBg]);

					sprintf(resource, pattern, colors[s][ColResource], "border");
					if (!loadxrdbcolor(xrdb, &clrnames[ColBorder], resource))
						strcpy(clrnames[ColBorder], colors[s][ColBorder]);

					scheme[s] = drw_scm_create(drw, clrnames, alphas[s], 3);
				}

				/* status2d terminal colours */
				for (s = 0; s < 16; s++) {
					sprintf(resource, "dusk.color%d", s);
					loadxrdbcolor(xrdb, &termcolor[s], resource);
				}

				/* dmenu colours */
				sprintf(resource, pattern, "dmenunorm", "fg");
				loadxrdbcolor(xrdb, &dmenunames[0], resource);
				sprintf(resource, pattern, "dmenunorm", "bg");
				loadxrdbcolor(xrdb, &dmenunames[1], resource);

				sprintf(resource, pattern, "dmenusel", "fg");
				loadxrdbcolor(xrdb, &dmenunames[2], resource);
				sprintf(resource, pattern, "dmenusel", "bg");
				loadxrdbcolor(xrdb, &dmenunames[3], resource);

			}
		}
	}

	XCloseDisplay(display);
}

void
xrdb(const Arg *arg)
{
	loadxrdb();
	focus(NULL);
	arrange(NULL);
}
