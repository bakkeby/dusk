# dusk - a dwm fork
# See LICENSE file for copyright and license details.

include config.mk

SRC = drw.c dusk.c util.c
OBJ = ${SRC:.c=.o}

# FreeBSD users, prefix all ifdef, else and endif statements with a . for this to work (e.g. .ifdef)

all: options dusk duskc

options:
	@echo dusk build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	${CC} -c ${CFLAGS} $<

${OBJ}: config.h config.mk

config.h:
	cp config.def.h $@

dusk: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

duskc:
	${CC} -o $@ lib/ipc/duskc.c ${LDFLAGS}

clean:
	rm -f dusk ${OBJ} dusk-${VERSION}.tar.gz
	rm -f duskc

dist: clean
	mkdir -p dusk-${VERSION}
	cp -R LICENSE Makefile README config.def.h config.mk\
		dusk.1 drw.h util.h ${SRC} dusk.png transient.c dusk-${VERSION}
	tar -cf dusk-${VERSION}.tar dusk-${VERSION}
	gzip dusk-${VERSION}.tar
	rm -rf dusk-${VERSION}

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f dusk ${DESTDIR}${PREFIX}/bin
	cp -f duskc ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/dusk
	chmod 755 ${DESTDIR}${PREFIX}/bin/duskc
	mkdir -p ${DESTDIR}${MANPREFIX}/man1
	sed "s/VERSION/${VERSION}/g" < dusk.1 > ${DESTDIR}${MANPREFIX}/man1/dusk.1
	chmod 644 ${DESTDIR}${MANPREFIX}/man1/dusk.1
	mkdir -p /usr/share/xsessions
	cp -n dusk.desktop /usr/share/xsessions/
	chmod 644 /usr/share/xsessions/dusk.desktop

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/dusk\
		${DESTDIR}${MANPREFIX}/man1/dusk.1\
		/usr/share/xsessions/dusk.desktop

.PHONY: all options clean dist install uninstall
