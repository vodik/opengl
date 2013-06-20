VERSION = $(shell git describe --tags)

CXX := ${CXX} -std=c++11

CXXFLAGS := -Wall -Wextra -pedantic \
	-D_GNU_SOURCE -D_REENTRANT \
	${CFLAGS}

LDLIBS = -lGL -lSDL2 -lpthread

all: sdl2

sdl2: sdl2.o sdl2.hh vbo.hh shader.hh texture.hh
	${CXX} ${CXXFLAGS} ${LDFLAGS} $< ${LDLIBS} -o $@

clean:
	${RM} sdl2 *.o

.PHONY: clean install uninstall
