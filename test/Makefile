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
INI_LIB= ../inih_r29/libinih.so
INI_LFLAGS = -L../inih_r29 -linih

OBJS = main.o

OBJS_FBSERVICE = ../fbservice.o ../fbprotocolCMSerial.o ../fb_protocol.o test_fbservice.o


all :  test_fbservice 

$(CAMERA_LIB) :
	make -C ../camera

$(EXE) : ${OBJS}  $(CAMERA_LIB) 
	g++ -o $@ ${OBJS} -L.. -lfid -L../tools -ltool -lpthread -lrt $(CAMERA_LFLAGS) $(INI_LFLAGS) 

main.o : ../maindelegator.h


test_fbservice : ${OBJS_FBSERVICE} 
	g++ -o $@ ${OBJS_FBSERVICE} -L..  -L../tools -ltool -lpthread -lrt


 .PHONY : clean
clean :
	-rm *.o main


