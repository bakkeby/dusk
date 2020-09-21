# dawn - a dwm fork
# See LICENSE file for copyright and license details.

include config.mk

SRC = drw.c dawn.c util.c
OBJ = ${SRC:.c=.o}

# FreeBSD users, prefix all ifdef, else and endif statements with a . for this to work (e.g. .ifdef)

ifdef YAJLLIBS
all: options dawn dawnc
else
all: options dawn
endif

options:
	@echo dawn build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	${CC} -c ${CFLAGS} $<

${OBJ}: config.h config.mk patches.h

config.h:
	cp config.def.h $@

patches.h:
	cp patches.def.h $@

dawn: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

ifdef YAJLLIBS
dawnc:
	${CC} -o $@ patch/ipc/dawnc.c ${LDFLAGS}
endif

clean:
	rm -f dawn ${OBJ} dawn-${VERSION}.tar.gz
	rm -f dawnc

dist: clean
	mkdir -p dawn-${VERSION}
	cp -R LICENSE Makefile README config.def.h config.mk\
		dawn.1 drw.h util.h ${SRC} dawn.png transient.c dawn-${VERSION}
	tar -cf dawn-${VERSION}.tar dawn-${VERSION}
	gzip dawn-${VERSION}.tar
	rm -rf dawn-${VERSION}

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f dawn ${DESTDIR}${PREFIX}/bin
ifdef YAJLLIBS
	cp -f dawnc ${DESTDIR}${PREFIX}/bin
endif
	chmod 755 ${DESTDIR}${PREFIX}/bin/dawn
ifdef YAJLLIBS
	chmod 755 ${DESTDIR}${PREFIX}/bin/dawnc
endif
	mkdir -p ${DESTDIR}${MANPREFIX}/man1
	sed "s/VERSION/${VERSION}/g" < dawn.1 > ${DESTDIR}${MANPREFIX}/man1/dawn.1
	chmod 644 ${DESTDIR}${MANPREFIX}/man1/dawn.1

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/dawn\
		${DESTDIR}${MANPREFIX}/man1/dawn.1

.PHONY: all options clean dist install uninstall
