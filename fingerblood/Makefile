$(warning os: $(shell uname -s))
$(info current dir is ${CURDIR})

MACHINE=$(shell uname -m)

CPPFLAGS =  -g -I.. 
EXE = test


SRCS = fbservice.cpp fbprotocolCMSerial.cpp fb_protocol.cpp test_fbservice.cpp
OBJS = $(SRCS:.cpp=.o)

all :  $(EXE)

$(EXE) : ${OBJS} 
	g++ -o $@ ${OBJS} -L.. -L../tools -ltool -lpthread -lrt -ldl



depend : $(SRCS)
	g++ -MM $(CPPFLAGS) $^ > $@


 .PHONY : clean
clean :
	-rm *.o main

-include depend

