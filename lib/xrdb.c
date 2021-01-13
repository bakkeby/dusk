void
loadxrdb()
{
	Display *display;
	char * resm;
	XrmDatabase xrdb;
	char *type;
	XrmValue value;

	if (disabled(Xresources))
		return;

	display = XOpenDisplay(NULL);

	if (display != NULL) {
		resm = XResourceManagerString(display);

		if (resm != NULL) {
			xrdb = XrmGetStringDatabase(resm);

			if (xrdb != NULL) {
				XRDB_LOAD_COLOR("dusk.normfgcolor", normfgcolor);
				XRDB_LOAD_COLOR("dusk.normbgcolor", normbgcolor);
				XRDB_LOAD_COLOR("dusk.normbordercolor", normbordercolor);
				XRDB_LOAD_COLOR("dusk.selfgcolor", selfgcolor);
				XRDB_LOAD_COLOR("dusk.selbgcolor", selbgcolor);
				XRDB_LOAD_COLOR("dusk.selbordercolor", selbordercolor);
				XRDB_LOAD_COLOR("dusk.titlenormfgcolor", titlenormfgcolor);
				XRDB_LOAD_COLOR("dusk.titlenormbgcolor", titlenormbgcolor);
				XRDB_LOAD_COLOR("dusk.titlenormbordercolor", titlenormbordercolor);
				XRDB_LOAD_COLOR("dusk.titleselfgcolor", titleselfgcolor);
				XRDB_LOAD_COLOR("dusk.titleselbgcolor", titleselbgcolor);
				XRDB_LOAD_COLOR("dusk.titleselbordercolor", titleselbordercolor);
				XRDB_LOAD_COLOR("dusk.wsnormfgcolor", wsnormfgcolor);
				XRDB_LOAD_COLOR("dusk.wsnormbgcolor", wsnormbgcolor);
				XRDB_LOAD_COLOR("dusk.wsvisfgcolor", wsvisfgcolor);
				XRDB_LOAD_COLOR("dusk.wsvisbgcolor", wsvisbgcolor);
				XRDB_LOAD_COLOR("dusk.wsselfgcolor", wsselfgcolor);
				XRDB_LOAD_COLOR("dusk.wsselbgcolor", wsselbgcolor);
				XRDB_LOAD_COLOR("dusk.hidfgcolor", hidfgcolor);
				XRDB_LOAD_COLOR("dusk.hidbgcolor", hidbgcolor);
				XRDB_LOAD_COLOR("dusk.hidbordercolor", hidbordercolor);
				XRDB_LOAD_COLOR("dusk.urgfgcolor", urgfgcolor);
				XRDB_LOAD_COLOR("dusk.urgbgcolor", urgbgcolor);
				XRDB_LOAD_COLOR("dusk.urgbordercolor", urgbordercolor);
				XRDB_LOAD_COLOR("dusk.normTTBbgcolor", normTTBbgcolor);
				XRDB_LOAD_COLOR("dusk.normLTRbgcolor", normLTRbgcolor);
				XRDB_LOAD_COLOR("dusk.normMONObgcolor", normMONObgcolor);
				XRDB_LOAD_COLOR("dusk.normGRIDbgcolor", normGRIDbgcolor);
				XRDB_LOAD_COLOR("dusk.normGRD1bgcolor", normGRD1bgcolor);
				XRDB_LOAD_COLOR("dusk.normGRD2bgcolor", normGRD2bgcolor);
				XRDB_LOAD_COLOR("dusk.normGRDMbgcolor", normGRDMbgcolor);
				XRDB_LOAD_COLOR("dusk.normHGRDbgcolor", normHGRDbgcolor);
				XRDB_LOAD_COLOR("dusk.normDWDLbgcolor", normDWDLbgcolor);
				XRDB_LOAD_COLOR("dusk.normSPRLbgcolor", normSPRLbgcolor);
				XRDB_LOAD_COLOR("dusk.normfloatbgcolor", normfloatbgcolor);
				XRDB_LOAD_COLOR("dusk.actTTBbgcolor", actTTBbgcolor);
				XRDB_LOAD_COLOR("dusk.actLTRbgcolor", actLTRbgcolor);
				XRDB_LOAD_COLOR("dusk.actMONObgcolor", actMONObgcolor);
				XRDB_LOAD_COLOR("dusk.actGRIDbgcolor", actGRIDbgcolor);
				XRDB_LOAD_COLOR("dusk.actGRD1bgcolor", actGRD1bgcolor);
				XRDB_LOAD_COLOR("dusk.actGRD2bgcolor", actGRD2bgcolor);
				XRDB_LOAD_COLOR("dusk.actGRDMbgcolor", actGRDMbgcolor);
				XRDB_LOAD_COLOR("dusk.actHGRDbgcolor", actHGRDbgcolor);
				XRDB_LOAD_COLOR("dusk.actDWDLbgcolor", actDWDLbgcolor);
				XRDB_LOAD_COLOR("dusk.actSPRLbgcolor", actSPRLbgcolor);
				XRDB_LOAD_COLOR("dusk.actfloatbgcolor", actfloatbgcolor);
				XRDB_LOAD_COLOR("dusk.selTTBbgcolor", selTTBbgcolor);
				XRDB_LOAD_COLOR("dusk.selLTRbgcolor", selLTRbgcolor);
				XRDB_LOAD_COLOR("dusk.selMONObgcolor", selMONObgcolor);
				XRDB_LOAD_COLOR("dusk.selGRIDbgcolor", selGRIDbgcolor);
				XRDB_LOAD_COLOR("dusk.selGRD1bgcolor", selGRD1bgcolor);
				XRDB_LOAD_COLOR("dusk.selGRD2bgcolor", selGRD2bgcolor);
				XRDB_LOAD_COLOR("dusk.selGRDMbgcolor", selGRDMbgcolor);
				XRDB_LOAD_COLOR("dusk.selHGRDbgcolor", selHGRDbgcolor);
				XRDB_LOAD_COLOR("dusk.selDWDLbgcolor", selDWDLbgcolor);
				XRDB_LOAD_COLOR("dusk.selSPRLbgcolor", selSPRLbgcolor);
				XRDB_LOAD_COLOR("dusk.selfloatbgcolor", selfloatbgcolor);
				XRDB_LOAD_COLOR("color0", termcol0);
				XRDB_LOAD_COLOR("color1", termcol1);
				XRDB_LOAD_COLOR("color2", termcol2);
				XRDB_LOAD_COLOR("color3", termcol3);
				XRDB_LOAD_COLOR("color4", termcol4);
				XRDB_LOAD_COLOR("color5", termcol5);
				XRDB_LOAD_COLOR("color6", termcol6);
				XRDB_LOAD_COLOR("color7", termcol7);
				XRDB_LOAD_COLOR("color8", termcol8);
				XRDB_LOAD_COLOR("color9", termcol9);
				XRDB_LOAD_COLOR("color10", termcol10);
				XRDB_LOAD_COLOR("color11", termcol11);
				XRDB_LOAD_COLOR("color12", termcol12);
				XRDB_LOAD_COLOR("color13", termcol13);
				XRDB_LOAD_COLOR("color14", termcol14);
				XRDB_LOAD_COLOR("color15", termcol15);
			}
		}
	}

	XCloseDisplay(display);
}

void
xrdb(const Arg *arg)
{
	loadxrdb();
	int i;
	for (i = 0; i < LENGTH(colors); i++)
		scheme[i] = drw_scm_create(drw, colors[i], alphas[i], ColCount);
	focus(NULL);
	arrange(NULL);
}