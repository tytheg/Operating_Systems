CFLAGS= -W -Wall -g
PROGS= uspsv1 uspsv2 uspsv3 uspsv4
OBJECTS=uspsv1.o uspsv2.o uspsv3.o uspsv4.o p1fxns.o

all: $(PROGS)

uspsv1: uspsv1.o p1fxns.o
	cc -o uspsv1 $^

uspsv2: uspsv2.o p1fxns.o
	cc -o uspsv2 $^

uspsv3: uspsv3.o p1fxns.o
	cc -o uspsv3 $^

uspsv4: uspsv4.o p1fxns.o
	cc -o uspsv4 $^

p1fxns.o: p1fxns.c p1fxns.h 

clean:
	rm -f *~ $(OBJECTS) $(PROGS)
