
# This makefile is usable on Mac OS X.
# To make use of it on other platforms, you should just need to
# replace -framework with -l, remove some libraries and add some others.


CXXFLAGS=-arch i386
all: demo_simple

demo_simple: demo_simple.o
	g++ demo_simple.o -framework xal -framework OpenAL -framework hltypes -framework AudioToolbox -framework CoreServices -framework Ogg -framework Vorbis -framework xalutil -o demo_simple -arch i386

clean:
	rm demo_simple.o demo_simple
