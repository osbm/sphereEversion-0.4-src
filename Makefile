
# use -g for debugging, -w to suppress warnings
# -Wall for lots of warnings, -O3 for highly optimized code

# full debug
#CFLAGS = -Wall -g -DDEBUG

# fast debug
CFLAGS = -Wall -O3 -DDEBUG

# fully optimized
#CFLAGS = -Wall -O3


CCXX=g++

LIBS=-lglut -lGLU -lGL -lm -L/usr/X11R6/lib -lXi -lXmu

all: sphereEversion

clean:
	rm -f core *.o sphereEversion

fontdata.o : fontdata.cpp fontdata.h fontDefinition.h global.h
	$(CCXX) $(CFLAGS) -c fontdata.cpp

drawutil2D.o : drawutil2D.cpp drawutil2D.h fontdata.h global.h
	$(CCXX) $(CFLAGS) -c drawutil2D.cpp

mathutil.o : mathutil.cpp mathutil.h global.h
	$(CCXX) $(CFLAGS) -c mathutil.cpp

drawutil.o : drawutil.cpp drawutil.h mathutil.h fontdata.h global.h
	$(CCXX) $(CFLAGS) -c drawutil.cpp

Camera.o : Camera.cpp Camera.h mathutil.h global.h
	$(CCXX) $(CFLAGS) -c Camera.cpp

generateGeometry.o : generateGeometry.cpp generateGeometry.h
	$(CCXX) $(CFLAGS) -c generateGeometry.cpp

main.o : main.cpp generateGeometry.h Camera.h drawutil.h mathutil.h drawutil2D.h global.h
	$(CCXX) $(CFLAGS) -c main.cpp

sphereEversion : fontdata.o drawutil2D.o mathutil.o drawutil.o Camera.o generateGeometry.o main.o
	$(CCXX) $(CFLAGS) -o sphereEversion \
	fontdata.o drawutil2D.o mathutil.o drawutil.o Camera.o generateGeometry.o main.o \
	$(LIBS)

