NAME = SpriteMage
SRCS = tinyfiledialogs.c main.cc
LINKING = -lSDL2main -lSDL2
LIBDIR = lib
INCLUDEDIR = include
CXX = g++
BITS = 64

SpriteMage: main.cc MageLoader.h Makefile
	cp depends/tinyfd/tinyfiledialogs* .
	${CXX} -x c++ -c ${SRCS} -I${INCLUDEDIR} -m${BITS} -no-pie -Os
	${CXX} *.o -L${LIBDIR} -static-libgcc -static-libstdc++ -Wl,-Bdynamic ${LINKING} -o ${NAME}

#appimage:
#	mkdir -p app
#	cp icon app/icon.png
#	linuxdeploy --appdir=app -d SpriteMage.desktop -i icon -e SpriteMage --output appimage
#
#	rm SpriteMage
#	mv SpriteMage-*.AppImage SpriteMage

clean:
	rm -rf ${NAME} *.o
