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
				XRDB_LOAD_COLOR("dusk.normfgcolor", colors[SchemeNorm][ColFg]);
				XRDB_LOAD_COLOR("dusk.normbgcolor", colors[SchemeNorm][ColBg]);
				XRDB_LOAD_COLOR("dusk.normbordercolor", colors[SchemeNorm][ColBorder]);
				XRDB_LOAD_COLOR("dusk.selfgcolor", colors[SchemeSel][ColFg]);
				XRDB_LOAD_COLOR("dusk.selbgcolor", colors[SchemeSel][ColBg]);
				XRDB_LOAD_COLOR("dusk.selbordercolor", colors[SchemeSel][ColBorder]);
				XRDB_LOAD_COLOR("dusk.titlenormfgcolor", colors[SchemeTitleNorm][ColFg]);
				XRDB_LOAD_COLOR("dusk.titlenormbgcolor", colors[SchemeTitleNorm][ColBg]);
				XRDB_LOAD_COLOR("dusk.titlenormbordercolor", colors[SchemeTitleNorm][ColBorder]);
				XRDB_LOAD_COLOR("dusk.titleselfgcolor", colors[SchemeTitleSel][ColFg]);
				XRDB_LOAD_COLOR("dusk.titleselbgcolor", colors[SchemeTitleSel][ColBg]);
				XRDB_LOAD_COLOR("dusk.titleselbordercolor", colors[SchemeTitleSel][ColBorder]);
				XRDB_LOAD_COLOR("dusk.wsnormfgcolor", colors[SchemeWsNorm][ColFg]);
				XRDB_LOAD_COLOR("dusk.wsnormbgcolor", colors[SchemeWsNorm][ColBg]);
				XRDB_LOAD_COLOR("dusk.wsvisfgcolor", colors[SchemeWsVisible][ColFg]);
				XRDB_LOAD_COLOR("dusk.wsvisbgcolor", colors[SchemeWsVisible][ColBg]);
				XRDB_LOAD_COLOR("dusk.wsselfgcolor", colors[SchemeWsSel][ColFg]);
				XRDB_LOAD_COLOR("dusk.wsselbgcolor", colors[SchemeWsSel][ColBg]);
				XRDB_LOAD_COLOR("dusk.wsoccfgcolor", colors[SchemeWsOcc][ColFg]);
				XRDB_LOAD_COLOR("dusk.wsoccbgcolor", colors[SchemeWsOcc][ColBg]);
				XRDB_LOAD_COLOR("dusk.hidnormfgcolor", colors[SchemeHidNorm][ColFg]);
				XRDB_LOAD_COLOR("dusk.hidnormbgcolor", colors[SchemeHidNorm][ColBg]);
				XRDB_LOAD_COLOR("dusk.hidnormbordercolor", colors[SchemeHidNorm][ColBorder]);
				XRDB_LOAD_COLOR("dusk.hidselfgcolor", colors[SchemeHidSel][ColFg]);
				XRDB_LOAD_COLOR("dusk.hidselbgcolor", colors[SchemeHidSel][ColBg]);
				XRDB_LOAD_COLOR("dusk.hidselbordercolor", colors[SchemeHidSel][ColBorder]);
				XRDB_LOAD_COLOR("dusk.urgfgcolor", colors[SchemeUrg][ColFg]);
				XRDB_LOAD_COLOR("dusk.urgbgcolor", colors[SchemeUrg][ColBg]);
				XRDB_LOAD_COLOR("dusk.urgbordercolor", colors[SchemeUrg][ColBorder]);
				XRDB_LOAD_COLOR("dusk.markedfgcolor", colors[SchemeMarked][ColFg]);
				XRDB_LOAD_COLOR("dusk.markedbgcolor", colors[SchemeMarked][ColBg]);
				XRDB_LOAD_COLOR("dusk.markedbordercolor", colors[SchemeMarked][ColBorder]);
				XRDB_LOAD_COLOR("dusk.scratchnormfgcolor", colors[SchemeScratchNorm][ColFg]);
				XRDB_LOAD_COLOR("dusk.scratchnormbgcolor", colors[SchemeScratchNorm][ColBg]);
				XRDB_LOAD_COLOR("dusk.scratchnormbordercolor", colors[SchemeScratchNorm][ColBorder]);
				XRDB_LOAD_COLOR("dusk.scratchselfgcolor", colors[SchemeScratchSel][ColFg]);
				XRDB_LOAD_COLOR("dusk.scratchselbgcolor", colors[SchemeScratchSel][ColBg]);
				XRDB_LOAD_COLOR("dusk.scratchselbordercolor", colors[SchemeScratchSel][ColBorder]);

				/* flexwintitle background colours */
				XRDB_LOAD_COLOR("dusk.normTTBbgcolor", colors[SchemeFlexInaTTB][ColBg]);
				XRDB_LOAD_COLOR("dusk.normLTRbgcolor", colors[SchemeFlexInaLTR][ColBg]);
				XRDB_LOAD_COLOR("dusk.normMONObgcolor", colors[SchemeFlexInaMONO][ColBg]);
				XRDB_LOAD_COLOR("dusk.normGRIDbgcolor", colors[SchemeFlexInaGRID][ColBg]);
				XRDB_LOAD_COLOR("dusk.normGRIDCbgcolor", colors[SchemeFlexInaGRIDC][ColBg]);
				XRDB_LOAD_COLOR("dusk.normGRD1bgcolor", colors[SchemeFlexInaGRD1][ColBg]);
				XRDB_LOAD_COLOR("dusk.normGRD2bgcolor", colors[SchemeFlexInaGRD2][ColBg]);
				XRDB_LOAD_COLOR("dusk.normGRDMbgcolor", colors[SchemeFlexInaGRDM][ColBg]);
				XRDB_LOAD_COLOR("dusk.normHGRDbgcolor", colors[SchemeFlexInaHGRD][ColBg]);
				XRDB_LOAD_COLOR("dusk.normDWDLbgcolor", colors[SchemeFlexInaDWDL][ColBg]);
				XRDB_LOAD_COLOR("dusk.normDWDLCbgcolor", colors[SchemeFlexInaDWDLC][ColBg]);
				XRDB_LOAD_COLOR("dusk.normSPRLbgcolor", colors[SchemeFlexInaSPRL][ColBg]);
				XRDB_LOAD_COLOR("dusk.normSPRLCbgcolor", colors[SchemeFlexInaSPRLC][ColBg]);
				XRDB_LOAD_COLOR("dusk.normTTMIbgcolor", colors[SchemeFlexInaTTMI][ColBg]);
				XRDB_LOAD_COLOR("dusk.normTTMICbgcolor", colors[SchemeFlexInaTTMIC][ColBg]);
				XRDB_LOAD_COLOR("dusk.normfloatbgcolor", colors[SchemeFlexInaFloat][ColBg]);
				XRDB_LOAD_COLOR("dusk.actTTBbgcolor", colors[SchemeFlexActTTB][ColBg]);
				XRDB_LOAD_COLOR("dusk.actLTRbgcolor", colors[SchemeFlexActLTR][ColBg]);
				XRDB_LOAD_COLOR("dusk.actMONObgcolor", colors[SchemeFlexActMONO][ColBg]);
				XRDB_LOAD_COLOR("dusk.actGRIDbgcolor", colors[SchemeFlexActGRID][ColBg]);
				XRDB_LOAD_COLOR("dusk.actGRIDCbgcolor", colors[SchemeFlexActGRIDC][ColBg]);
				XRDB_LOAD_COLOR("dusk.actGRD1bgcolor", colors[SchemeFlexActGRD1][ColBg]);
				XRDB_LOAD_COLOR("dusk.actGRD2bgcolor", colors[SchemeFlexActGRD2][ColBg]);
				XRDB_LOAD_COLOR("dusk.actGRDMbgcolor", colors[SchemeFlexActGRDM][ColBg]);
				XRDB_LOAD_COLOR("dusk.actHGRDbgcolor", colors[SchemeFlexActHGRD][ColBg]);
				XRDB_LOAD_COLOR("dusk.actDWDLbgcolor", colors[SchemeFlexActDWDL][ColBg]);
				XRDB_LOAD_COLOR("dusk.actDWDLCbgcolor", colors[SchemeFlexActDWDLC][ColBg]);
				XRDB_LOAD_COLOR("dusk.actSPRLbgcolor", colors[SchemeFlexActSPRL][ColBg]);
				XRDB_LOAD_COLOR("dusk.actSPRLCbgcolor", colors[SchemeFlexActSPRLC][ColBg]);
				XRDB_LOAD_COLOR("dusk.actTTMIbgcolor", colors[SchemeFlexActTTMI][ColBg]);
				XRDB_LOAD_COLOR("dusk.actTTMICbgcolor", colors[SchemeFlexActTTMIC][ColBg]);
				XRDB_LOAD_COLOR("dusk.actfloatbgcolor", colors[SchemeFlexActFloat][ColBg]);
				XRDB_LOAD_COLOR("dusk.selTTBbgcolor", colors[SchemeFlexSelTTB][ColBg]);
				XRDB_LOAD_COLOR("dusk.selLTRbgcolor", colors[SchemeFlexSelLTR][ColBg]);
				XRDB_LOAD_COLOR("dusk.selMONObgcolor", colors[SchemeFlexSelMONO][ColBg]);
				XRDB_LOAD_COLOR("dusk.selGRIDbgcolor", colors[SchemeFlexSelGRID][ColBg]);
				XRDB_LOAD_COLOR("dusk.selGRIDCbgcolor", colors[SchemeFlexSelGRIDC][ColBg]);
				XRDB_LOAD_COLOR("dusk.selGRD1bgcolor", colors[SchemeFlexSelGRD1][ColBg]);
				XRDB_LOAD_COLOR("dusk.selGRD2bgcolor", colors[SchemeFlexSelGRD2][ColBg]);
				XRDB_LOAD_COLOR("dusk.selGRDMbgcolor", colors[SchemeFlexSelGRDM][ColBg]);
				XRDB_LOAD_COLOR("dusk.selHGRDbgcolor", colors[SchemeFlexSelHGRD][ColBg]);
				XRDB_LOAD_COLOR("dusk.selDWDLbgcolor", colors[SchemeFlexSelDWDL][ColBg]);
				XRDB_LOAD_COLOR("dusk.selDWDLCbgcolor", colors[SchemeFlexSelDWDLC][ColBg]);
				XRDB_LOAD_COLOR("dusk.selSPRLbgcolor", colors[SchemeFlexSelSPRL][ColBg]);
				XRDB_LOAD_COLOR("dusk.selSPRLCbgcolor", colors[SchemeFlexSelSPRLC][ColBg]);
				XRDB_LOAD_COLOR("dusk.selTTMIbgcolor", colors[SchemeFlexSelTTMI][ColBg]);
				XRDB_LOAD_COLOR("dusk.selTTMICbgcolor", colors[SchemeFlexSelTTMIC][ColBg]);
				XRDB_LOAD_COLOR("dusk.selfloatbgcolor", colors[SchemeFlexSelFloat][ColBg]);

				/* flexwintitle foreground colours */
				XRDB_LOAD_COLOR("dusk.normTTBfgcolor", colors[SchemeFlexInaTTB][ColFg]);
				XRDB_LOAD_COLOR("dusk.normLTRfgcolor", colors[SchemeFlexInaLTR][ColFg]);
				XRDB_LOAD_COLOR("dusk.normMONOfgcolor", colors[SchemeFlexInaMONO][ColFg]);
				XRDB_LOAD_COLOR("dusk.normGRIDfgcolor", colors[SchemeFlexInaGRID][ColFg]);
				XRDB_LOAD_COLOR("dusk.normGRIDCfgcolor", colors[SchemeFlexInaGRIDC][ColFg]);
				XRDB_LOAD_COLOR("dusk.normGRD1fgcolor", colors[SchemeFlexInaGRD1][ColFg]);
				XRDB_LOAD_COLOR("dusk.normGRD2fgcolor", colors[SchemeFlexInaGRD2][ColFg]);
				XRDB_LOAD_COLOR("dusk.normGRDMfgcolor", colors[SchemeFlexInaGRDM][ColFg]);
				XRDB_LOAD_COLOR("dusk.normHGRDfgcolor", colors[SchemeFlexInaHGRD][ColFg]);
				XRDB_LOAD_COLOR("dusk.normDWDLfgcolor", colors[SchemeFlexInaDWDL][ColFg]);
				XRDB_LOAD_COLOR("dusk.normDWDLCfgcolor", colors[SchemeFlexInaDWDLC][ColFg]);
				XRDB_LOAD_COLOR("dusk.normSPRLfgcolor", colors[SchemeFlexInaSPRL][ColFg]);
				XRDB_LOAD_COLOR("dusk.normSPRLCfgcolor", colors[SchemeFlexInaSPRLC][ColFg]);
				XRDB_LOAD_COLOR("dusk.normTTMIfgcolor", colors[SchemeFlexInaTTMI][ColFg]);
				XRDB_LOAD_COLOR("dusk.normTTMICfgcolor", colors[SchemeFlexInaTTMIC][ColFg]);
				XRDB_LOAD_COLOR("dusk.normfloatfgcolor", colors[SchemeFlexInaFloat][ColFg]);
				XRDB_LOAD_COLOR("dusk.actTTBfgcolor", colors[SchemeFlexActTTB][ColFg]);
				XRDB_LOAD_COLOR("dusk.actLTRfgcolor", colors[SchemeFlexActLTR][ColFg]);
				XRDB_LOAD_COLOR("dusk.actMONOfgcolor", colors[SchemeFlexActMONO][ColFg]);
				XRDB_LOAD_COLOR("dusk.actGRIDfgcolor", colors[SchemeFlexActGRID][ColFg]);
				XRDB_LOAD_COLOR("dusk.actGRIDCfgcolor", colors[SchemeFlexActGRIDC][ColFg]);
				XRDB_LOAD_COLOR("dusk.actGRD1fgcolor", colors[SchemeFlexActGRD1][ColFg]);
				XRDB_LOAD_COLOR("dusk.actGRD2fgcolor", colors[SchemeFlexActGRD2][ColFg]);
				XRDB_LOAD_COLOR("dusk.actGRDMfgcolor", colors[SchemeFlexActGRDM][ColFg]);
				XRDB_LOAD_COLOR("dusk.actHGRDfgcolor", colors[SchemeFlexActHGRD][ColFg]);
				XRDB_LOAD_COLOR("dusk.actDWDLfgcolor", colors[SchemeFlexActDWDL][ColFg]);
				XRDB_LOAD_COLOR("dusk.actDWDLCfgcolor", colors[SchemeFlexActDWDLC][ColFg]);
				XRDB_LOAD_COLOR("dusk.actSPRLfgcolor", colors[SchemeFlexActSPRL][ColFg]);
				XRDB_LOAD_COLOR("dusk.actSPRLCfgcolor", colors[SchemeFlexActSPRLC][ColFg]);
				XRDB_LOAD_COLOR("dusk.actTTMIfgcolor", colors[SchemeFlexActTTMI][ColFg]);
				XRDB_LOAD_COLOR("dusk.actTTMICfgcolor", colors[SchemeFlexActTTMIC][ColFg]);
				XRDB_LOAD_COLOR("dusk.actfloatfgcolor", colors[SchemeFlexActFloat][ColFg]);
				XRDB_LOAD_COLOR("dusk.selTTBfgcolor", colors[SchemeFlexSelTTB][ColFg]);
				XRDB_LOAD_COLOR("dusk.selLTRfgcolor", colors[SchemeFlexSelLTR][ColFg]);
				XRDB_LOAD_COLOR("dusk.selMONOfgcolor", colors[SchemeFlexSelMONO][ColFg]);
				XRDB_LOAD_COLOR("dusk.selGRIDfgcolor", colors[SchemeFlexSelGRID][ColFg]);
				XRDB_LOAD_COLOR("dusk.selGRIDCfgcolor", colors[SchemeFlexSelGRIDC][ColFg]);
				XRDB_LOAD_COLOR("dusk.selGRD1fgcolor", colors[SchemeFlexSelGRD1][ColFg]);
				XRDB_LOAD_COLOR("dusk.selGRD2fgcolor", colors[SchemeFlexSelGRD2][ColFg]);
				XRDB_LOAD_COLOR("dusk.selGRDMfgcolor", colors[SchemeFlexSelGRDM][ColFg]);
				XRDB_LOAD_COLOR("dusk.selHGRDfgcolor", colors[SchemeFlexSelHGRD][ColFg]);
				XRDB_LOAD_COLOR("dusk.selDWDLfgcolor", colors[SchemeFlexSelDWDL][ColFg]);
				XRDB_LOAD_COLOR("dusk.selDWDLCfgcolor", colors[SchemeFlexSelDWDLC][ColFg]);
				XRDB_LOAD_COLOR("dusk.selSPRLfgcolor", colors[SchemeFlexSelSPRL][ColFg]);
				XRDB_LOAD_COLOR("dusk.selSPRLCfgcolor", colors[SchemeFlexSelSPRLC][ColFg]);
				XRDB_LOAD_COLOR("dusk.selTTMIfgcolor", colors[SchemeFlexSelTTMI][ColFg]);
				XRDB_LOAD_COLOR("dusk.selTTMICfgcolor", colors[SchemeFlexSelTTMIC][ColFg]);
				XRDB_LOAD_COLOR("dusk.selfloatfgcolor", colors[SchemeFlexSelFloat][ColFg]);

				/* status2d terminal colours */
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
