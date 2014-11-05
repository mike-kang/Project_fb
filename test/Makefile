$(warning os: $(shell uname -s))
$(info current dir is ${CURDIR})

MACHINE=$(shell uname -m)

CPPFLAGS =  -g -I.. 
EXE = main

ifeq ($(MACHINE), x86_64)
  CPPFLAGS += -DSIMULATOR
else
  CAMERA_LIB= ../camera/libcamera.so
  CAMERA_LFLAGS= -lcamera -lopenmaxil -lbcm_host -L.. -L/opt/vc/lib -L../camera
  CPPFLAGS += -DCAMERA
endif
INI_LIB= ../inih_r29/arch/$(MACHINE)/libinih.a
INI_LFLAGS = -L../inih_r29/arch/$(MACHINE) -linih

OBJS = main.o



all : $(EXE)

$(CAMERA_LIB) :
	make -C ../camera

$(EXE) : ${OBJS} ../tools/libtool.so $(CAMERA_LIB) $(INI_LIB)
	g++ -o $@ ${OBJS} -L.. -lfid -L../tools -ltool -lpthread -lrt $(CAMERA_LFLAGS) $(INI_LFLAGS) 

main.o : ../maindelegator.h
 .PHONY : clean
clean :
	-rm *.o main


