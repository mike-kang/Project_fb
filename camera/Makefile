$(warning os: $(shell uname -s))
$(info current dir is ${CURDIR})

#TEST = true

OBJS = omxilComponent.o dump.o camerastill.o  
LIB = libcamera.so
ifeq ($(TEST), true)
  EXE = camera
  all : ${LIB}  ${EXE}
  CAMERA_LFLAGS=  -lopenmaxil -lbcm_host -lrt -L. -L/opt/vc/lib -lvcos -lvchiq_arm
else  
  all : ${LIB} 
endif

CPPFLAGS =  -fPIC -g -I/opt/vc/include -I/opt/vc/include/interface/vcos/pthreads \
		-I/opt/vc/include/interface/vmcs_host/linux -I../tools -DOMX_SKIP64BIT

${LIB} : ${OBJS} 
	g++ -shared -o ${LIB} ${OBJS} 

${EXE} : ${OBJS} main.o
	g++  -o ${EXE} ${OBJS} main.o -L../tools -ltool -lpthread ${CAMERA_LFLAGS} -lcamera
	
camerastill.o : ../tools/log.h ../tools/logservice.h camerastill.h omxilComponent.h ../tools/Semaphore.h
omxilComponent.o : ../tools/log.h ../tools/logservice.h omxilComponent.h dump.h
main.o : camerastill.h omxilComponent.h 

.PHONY : clean
clean :
	-rm *.o ${LIB} ${EXE}


