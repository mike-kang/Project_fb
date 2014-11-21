$(warning os: $(shell uname -s))
$(info current dir is ${CURDIR})

SRCS = filesystem.cpp log.cpp logservice.cpp serial.cpp asyncserial.cpp timer.cpp network.cpp date.cpp base64.cpp utils.cpp wavplayer.cpp datetime.cpp
OBJS = $(SRCS:.cpp=.o)

LIB = libtool.so

CPPFLAGS =  -fPIC -g -I.

all : ${LIB} 

${LIB} : ${OBJS} 
	g++ -shared -o ${LIB} ${OBJS} 

depend : $(SRCS)
	g++ -MM $(CPPFLAGS) $^ > $@
	

.PHONY : clean
clean :
	-rm *.o ${LIB}

-include depend