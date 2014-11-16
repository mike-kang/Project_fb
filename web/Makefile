$(warning os: $(shell uname -s))
$(info current dir is ${CURDIR})


OBJS = main.o webservice.o safemanwebservice.o

EXE = webservice
all : ${EXE}

CPPFLAGS =   -g -DDEBUG -I..

${EXE} :  ${OBJS}
	g++  -o ${EXE}  ${OBJS} -L../tools -ltool -lrt -lpthread
	

main.o : webservice.h
webservice.o : webservice.h
safemanwebservice.o : safemanwebservice.h webservice.h

.PHONY : clean
clean :
	-rm *.o  ${EXE}


