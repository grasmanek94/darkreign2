OBJS = main.o game.o matchmaker.o
SRCS = main.cpp game.cpp matchmaker.cpp

CFLAGS= -D_LINUX -D_USE_CURSES -D_NO_TOPLEVEL_CATCH -I. -g
CPPFLAGS = $(CFLAGS) -I../..

all: main.o game.o matchmaker.o
	g++ -g -I../.. -o matchmaker -lpthread -lncurses main.o game.o matchmaker.o ../../libTitan.a

main.o: main.cpp
game.o: game.cpp
matchmaker.o: matchmaker.cpp

clean:
	rm -f $(OBJS) matchmaker

depend:
	makedepend -- $(CFLAGS) -- $(SRCS)

# DO NOT DELETE

main.o: /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h
main.o: /usr/include/gnu/stubs.h
main.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stddef.h
main.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stdarg.h
main.o: /usr/include/bits/types.h /usr/include/libio.h
main.o: /usr/include/_G_config.h /usr/include/bits/stdio_lim.h
main.o: /usr/include/termios.h /usr/include/bits/termios.h
main.o: /usr/include/sys/ttydefaults.h main.h OutputQueue.h matchmaker.h
main.o: game.h
game.o: main.h game.h
matchmaker.o: /usr/include/sys/types.h /usr/include/features.h
matchmaker.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
matchmaker.o: /usr/include/bits/types.h
matchmaker.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stddef.h
matchmaker.o: /usr/include/time.h /usr/include/endian.h
matchmaker.o: /usr/include/bits/endian.h /usr/include/sys/select.h
matchmaker.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
matchmaker.o: /usr/include/sys/sysmacros.h /usr/include/sys/stat.h
matchmaker.o: /usr/include/bits/stat.h matchmaker.h game.h main.h
