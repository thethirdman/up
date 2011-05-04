# qotd - quote of the day server
# See LICENSE file for copyright and license details.

include config.mk

SRC = qotd.c
OBJ = ${SRC:.c=.o}

all: options qotd

options:
	@echo qotd build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

config.mk:
	cp config.def.mk config.mk

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${OBJ}: config.mk

qotd: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f qotd ${OBJ} qotd-${VERSION}.tar.gz

dist: clean
	@echo creating dist tarball
	@mkdir -p qotd-${VERSION}
	@cp -R LICENSE Makefile README config.def.mk qotd.1 ${SRC} qotd-${VERSION}
	@tar -cf qotd-${VERSION}.tar qotd-${VERSION}
	@gzip qotd-${VERSION}.tar
	@rm -rf qotd-${VERSION}

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f qotd ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/qotd
	@echo installing manual page to ${DESTDIR}${MANPREFIX}/man1
	@mkdir -p ${DESTDIR}${MANPREFIX}/man1
	@sed "s/VERSION/${VERSION}/g" < qotd.1 > ${DESTDIR}${MANPREFIX}/man1/qotd.1
	@chmod 644 ${DESTDIR}${MANPREFIX}/man1/qotd.1

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/qotd
	@echo removing manual page from ${DESTDIR}${MANPREFIX}/man1
	@rm -f ${DESTDIR}${MANPREFIX}/man1/qotd.1

.PHONY: all options clean dist install uninstall
