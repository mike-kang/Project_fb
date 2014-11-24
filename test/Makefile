$(warning os: $(shell uname -s))
$(info current dir is ${CURDIR})

MACHINE=$(shell uname -m)

CPPFLAGS =  -g -I.. 
EXE = main

INI_LIB= ../inih_r29/libinih.so
INI_LFLAGS = -L../inih_r29 -linih

OBJS = main.o

all :  $(EXE)

$(EXE) : ${OBJS}  $(CAMERA_LIB) 
	g++ -o $@ ${OBJS} -L.. -lfid -L../tools -ltool -lpthread -lrt -lsqlite3 $(INI_LFLAGS) 

main.o : ../maindelegator.h



 .PHONY : clean
clean :
	-rm *.o main

-include depend

