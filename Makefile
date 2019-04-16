PROG=		chip-dis
SRCS=		chip-dis.c
NOMAN=

CLFAGS+=	-g
CFLAGS+=	-Wall -Werror -Wextra -std=c99 -Wcast-qual -Wformat=2
CFLAGS+=	-Wmissing-declarations -pedantic -Wstrict-prototypes
CFLAGS+=	-Wpointer-arith -Wuninitialized -Wmissing-prototypes
CFLAGS+=	-Wsign-compare -Wshadow -Wdeclaration-after-statement
CFLAGS+=	-Wfloat-equal -Wcast-align -Wundef -Wstrict-aliasing=2

all: ${PROG}

${PROG}: ${SRCS}
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ ${SRCS}

clean:
	rm -f a.out [Ee]rrs mklog *.core y.tab.h ${PROG} ${PROG}.o
