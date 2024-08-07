# dusk version
VERSION = 1.0

# Customize below to fit your system

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

X11INC = /usr/X11R6/include
X11LIB = /usr/X11R6/lib

# Optional dependency on libxi for mouse related features
#HAVE_LIBXI = -DHAVE_LIBXI=1
#XINPUTLIBS = -lXi
#XFIXESLIBS = -lXfixes

# Optional dependency on fribidi for RTL languages
#HAVE_FRIBIDI = -DHAVE_FRIBIDI=1
#FRIBIDIINC = -I/usr/include/fribidi
#FRIBIDILIBS = -lfribidi

# Optionally set to 1 to use key codes rather than keysyms for keybindings
USE_KEYCODES = 0

# Xinerama, comment if you don't want it
XINERAMALIBS  = -lXinerama
XINERAMAFLAGS = -DXINERAMA

# freetype
FREETYPELIBS = -lfontconfig -lXft
FREETYPEINC = /usr/include/freetype2

# Needed for the alpha patch and the winicon patch
XRENDER = -lXrender

# Needed for the ipc patch
YAJLLIBS = -lyajl
YAJLINC = -I/usr/include/yajl

# Needed for the swallow patch
XCBLIBS = -lX11-xcb -lxcb -lxcb-res

# Needed for the winicon patch
IMLIB2LIBS = -lImlib2

# includes and libs
INCS = -I${X11INC} -I${FREETYPEINC} ${YAJLINC} ${XINPUTINC} ${FRIBIDIINC}
LIBS = -L${X11LIB} -lX11 ${XINERAMALIBS} ${FREETYPELIBS}  ${XRENDER} ${XCBLIBS} ${KVMLIB} ${YAJLLIBS} ${IMLIB2LIBS} ${XINPUTLIBS} ${XFIXESLIBS} ${FRIBIDILIBS}

# Optional host flag for computer specific configuration
#HOSTFLAGS = -DHOST=$(shell command -v cksum > /dev/null && hostname | cksum | tr -d ' ')

# Optional compiler optimisations may create smaller binaries and
# faster code, but increases compile time.
# See https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html
#OPTIMISATIONS = -march=native -mtune=native -flto=auto -O3

# flags
CPPFLAGS = -D_DEFAULT_SOURCE ${HAVE_LIBXI} ${HAVE_FRIBIDI} -D_BSD_SOURCE -D_POSIX_C_SOURCE=200809L -DUSE_KEYCODES=${USE_KEYCODES} -DVERSION=\"${VERSION}\" ${XINERAMAFLAGS} ${HOSTFLAGS}
CFLAGS   = ${OPTIMISATIONS} -std=c99 -pedantic -Wall -Wno-unused-function -Wno-deprecated-declarations ${INCS} ${CPPFLAGS}
LDFLAGS  = ${LIBS}

# compiler and linker
CC = cc
