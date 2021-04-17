/* See LICENSE file for copyright and license details. */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

unsigned long settings = 0;

void *
ecalloc(size_t nmemb, size_t size)
{
	void *p;

	if (!(p = calloc(nmemb, size)))
		die("calloc:");
	return p;
}

void
die(const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	if (fmt[0] && fmt[strlen(fmt)-1] == ':') {
		fputc(' ', stderr);
		perror(NULL);
	} else {
		fputc('\n', stderr);
	}

	exit(1);
}

int
enabled(const long functionality)
{
	return settings & functionality;
}

int
disabled(const long functionality)
{
	return !(settings & functionality);
}

void
enablefunc(const long functionality)
{
	settings |= functionality;
}

void
disablefunc(const long functionality)
{
	settings &= ~functionality;
}

void
togglefunc(const long functionality)
{
	settings ^= functionality;
}

const unsigned long
getfuncbyname(const char *name)
{
	if (strcmp(name, "SmartGaps") == 0)
		return SmartGaps;
	else if (strcmp(name, "SmartGapsMonocle") == 0)
		return SmartGapsMonocle;
	else if (strcmp(name, "Swallow") == 0)
		return Swallow;
	else if (strcmp(name, "SwallowFloating") == 0)
		return SwallowFloating;
	else if (strcmp(name, "CenteredWindowName") == 0)
		return CenteredWindowName;
	else if (strcmp(name, "BarActiveGroupBorderColor") == 0)
		return BarActiveGroupBorderColor;
	else if (strcmp(name, "SpawnCwd") == 0)
		return SpawnCwd;
	else if (strcmp(name, "ColorEmoji") == 0)
		return ColorEmoji;
	else if (strcmp(name, "Status2DNoAlpha") == 0)
		return Status2DNoAlpha;
	else if (strcmp(name, "Systray") == 0)
		return Systray;
	else if (strcmp(name, "BarBorder") == 0)
		return BarBorder;
	else if (strcmp(name, "NoBorders") == 0)
		return NoBorders;
	else if (strcmp(name, "Warp") == 0)
		return Warp;
	else if (strcmp(name, "FocusedOnTop") == 0)
		return FocusedOnTop;
	else if (strcmp(name, "DecorationHints") == 0)
		return DecorationHints;
	else if (strcmp(name, "FocusOnNetActive") == 0)
		return FocusOnNetActive;
	else if (strcmp(name, "AllowNoModifierButtons") == 0)
		return AllowNoModifierButtons;
	else if (strcmp(name, "CenterSizeHintsClients") == 0)
		return CenterSizeHintsClients;
	else if (strcmp(name, "ResizeHints") == 0)
		return ResizeHints;
	else if (strcmp(name, "SortScreens") == 0)
		return SortScreens;
	else if (strcmp(name, "ViewOnWs") == 0)
		return ViewOnWs;
	else if (strcmp(name, "Xresources") == 0)
		return Xresources;
	else if (strcmp(name, "Debug") == 0)
		return Debug;
	else if (strcmp(name, "AltWorkspaceIcons") == 0)
		return AltWorkspaceIcons;
	else if (strcmp(name, "GreedyMonitor") == 0)
		return GreedyMonitor;
	else if (strcmp(name, "SmartLayoutConvertion") == 0)
		return SmartLayoutConvertion;
	else if (strcmp(name, "AutoHideScratchpads") == 0)
		return AutoHideScratchpads;
	else if (strcmp(name, "RioDrawIncludeBorders") == 0)
		return RioDrawIncludeBorders;
	else if (strcmp(name, "RioDrawSpawnAsync") == 0)
		return RioDrawSpawnAsync;
	else if (strcmp(name, "BarPadding") == 0)
		return BarPadding;
	else if (strcmp(name, "FuncPlaceholder0x80000000") == 0)
		return FuncPlaceholder0x80000000;
	else if (strcmp(name, "FuncPlaceholder4294967296") == 0)
		return FuncPlaceholder4294967296;
	else if (strcmp(name, "FuncPlaceholder8589934592") == 0)
		return FuncPlaceholder8589934592;
	else if (strcmp(name, "FuncPlaceholder17179869184") == 0)
		return FuncPlaceholder17179869184;
	else if (strcmp(name, "FuncPlaceholder34359738368") == 0)
		return FuncPlaceholder34359738368;
	else if (strcmp(name, "FuncPlaceholder68719476736") == 0)
		return FuncPlaceholder68719476736;
	else if (strcmp(name, "FuncPlaceholder137438953472") == 0)
		return FuncPlaceholder137438953472;
	else if (strcmp(name, "FuncPlaceholder274877906944") == 0)
		return FuncPlaceholder274877906944;
	else if (strcmp(name, "FuncPlaceholder549755813888") == 0)
		return FuncPlaceholder549755813888;
	else if (strcmp(name, "FuncPlaceholder1099511627776") == 0)
		return FuncPlaceholder1099511627776;
	else if (strcmp(name, "FuncPlaceholder2199023255552") == 0)
		return FuncPlaceholder2199023255552;
	else if (strcmp(name, "FuncPlaceholder4398046511104") == 0)
		return FuncPlaceholder4398046511104;
	else if (strcmp(name, "FuncPlaceholder8796093022208") == 0)
		return FuncPlaceholder8796093022208;
	else if (strcmp(name, "FuncPlaceholder17592186044416") == 0)
		return FuncPlaceholder17592186044416;
	else if (strcmp(name, "FuncPlaceholder35184372088832") == 0)
		return FuncPlaceholder35184372088832;
	else if (strcmp(name, "FuncPlaceholder70368744177664") == 0)
		return FuncPlaceholder70368744177664;
	else if (strcmp(name, "FuncPlaceholder140737488355328") == 0)
		return FuncPlaceholder140737488355328;
	else if (strcmp(name, "FuncPlaceholder281474976710656") == 0)
		return FuncPlaceholder281474976710656;
	else if (strcmp(name, "FuncPlaceholder562949953421312") == 0)
		return FuncPlaceholder562949953421312;
	else if (strcmp(name, "FuncPlaceholder1125899906842624") == 0)
		return FuncPlaceholder1125899906842624;
	else if (strcmp(name, "FuncPlaceholder2251799813685248") == 0)
		return FuncPlaceholder2251799813685248;
	else if (strcmp(name, "FuncPlaceholder4503599627370496") == 0)
		return FuncPlaceholder4503599627370496;
	else if (strcmp(name, "FuncPlaceholder9007199254740992") == 0)
		return FuncPlaceholder9007199254740992;
	else if (strcmp(name, "FuncPlaceholder18014398509481984") == 0)
		return FuncPlaceholder18014398509481984;
	else if (strcmp(name, "FuncPlaceholder36028797018963968") == 0)
		return FuncPlaceholder36028797018963968;
	else if (strcmp(name, "FuncPlaceholder72057594037927936") == 0)
		return FuncPlaceholder72057594037927936;
	else if (strcmp(name, "FuncPlaceholder144115188075855872") == 0)
		return FuncPlaceholder144115188075855872;
	else if (strcmp(name, "FuncPlaceholder288230376151711744") == 0)
		return FuncPlaceholder288230376151711744;
	else if (strcmp(name, "FuncPlaceholder576460752303423488") == 0)
		return FuncPlaceholder576460752303423488;
	else if (strcmp(name, "FuncPlaceholder1152921504606846976") == 0)
		return FuncPlaceholder1152921504606846976;
	else if (strcmp(name, "FuncPlaceholder2305843009213693952") == 0)
		return FuncPlaceholder2305843009213693952;
	else if (strcmp(name, "FuncPlaceholder4611686018427387904") == 0)
		return FuncPlaceholder4611686018427387904;
	else if (strcmp(name, "FuncPlaceholder9223372036854775808") == 0)
		return FuncPlaceholder9223372036854775808;

	return 0;
}
