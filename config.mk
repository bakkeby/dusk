# dusk version
VERSION = 1.0

# Customize below to fit your system

# Paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

PKG_CONFIG = pkg-config

# Optional dependency on libxi for mouse related features
#HAVE_LIBXI = -DHAVE_LIBXI=1
#XINPUTLIBS = `$(PKG_CONFIG) --libs xi xfixes`

# Optional dependency on fribidi for RTL languages
#HAVE_FRIBIDI = -DHAVE_FRIBIDI=1
#FRIBIDILIBS = `$(PKG_CONFIG) --libs fribidi`
#FRIBIDIINC = `$(PKG_CONFIG) --cflags fribidi`

# Optionally set to 1 to use key codes rather than keysyms for keybindings
USE_KEYCODES = 0

# Optional dependency on Xinerama for multi-monitor support
XINERAMALIBS  = `$(PKG_CONFIG) --libs xinerama`
XINERAMAFLAGS = -DXINERAMA

# freetype
# FREETYPELIBS = -lfontconfig -lXft
FREETYPELIBS = `$(PKG_CONFIG) --libs fontconfig xft`
FREETYPEINC = `$(PKG_CONFIG) --cflags fontconfig`

# Needed for alpha and window icons
XRENDER = `$(PKG_CONFIG) --libs xrender`

## Optional dependency on dbus (used for ipc / duskc)
HAVE_DBUS = -DHAVE_DBUS=1
DBUSLIBS = `$(PKG_CONFIG) --libs dbus-1`
DBUSINC = `$(PKG_CONFIG) --cflags dbus-1`

# Optional dependency on yajl, needed for ipc information dumps
YAJLLIBS = `$(PKG_CONFIG) --libs yajl`
YAJLINC = `$(PKG_CONFIG) --cflags yajl`

# Needed for window swallowing functionality
XCBLIBS = `$(PKG_CONFIG) --libs x11-xcb xcb-res`

# Needed for window icons
IMLIB2LIBS = `$(PKG_CONFIG) --libs imlib2`

# Includes and libs
INCS = ${FREETYPEINC} ${YAJLINC} ${DBUSINC} ${XINPUTINC} ${FRIBIDIINC}
LIBS = ${XINERAMALIBS} ${FREETYPELIBS} ${XRENDER} ${XCBLIBS} ${KVMLIB} ${YAJLLIBS} ${DBUSLIBS} ${IMLIB2LIBS} ${XINPUTLIBS} ${FRIBIDILIBS}

# Optional host flag for computer specific configuration
#HOSTFLAGS = -DHOST=$(shell command -v cksum > /dev/null && hostname | cksum | tr -d ' ')

# Optional compiler optimisations may create smaller binaries and
# faster code, but increases compile time.
# See https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html
#OPTIMISATIONS = -march=native -flto=auto -O3

# flags
CPPFLAGS = -D_DEFAULT_SOURCE ${HAVE_LIBXI} ${HAVE_FRIBIDI} ${HAVE_DBUS} -D_BSD_SOURCE -D_POSIX_C_SOURCE=200809L -DUSE_KEYCODES=${USE_KEYCODES} -DVERSION=\"${VERSION}\" ${XINERAMAFLAGS} ${HOSTFLAGS}
CFLAGS   = ${OPTIMISATIONS} -std=c99 -pedantic -Wall -Wno-unused-function -Wno-deprecated-declarations ${INCS} ${CPPFLAGS}
LDFLAGS  = ${LIBS}

# Compiler and linker
# CC = cc
