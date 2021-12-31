VERSION = 1.0
THREADS = 4

CPPFLAGS = -D_DEFAULT_SOURCE -DVERSION=\"${VERSION}\" -DTHREADS=${THREADS}
CFLAGS   = -ansi -pedantic -Wextra -Wall ${CPPFLAGS} -g
LDFLAGS  = -lcrypto -lssl -lsqlite3 -lpthread

SRC = dedup.c arg.c rcdir.c sha256.c util.c sql.c queue.c
OBJ = ${SRC:.c=.o}

all: options dedup

options:
	@echo dedup build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	${CC} -c ${CFLAGS} $<

dedup: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	rm -f dedup ${OBJ}

.PHONY: all options clean
