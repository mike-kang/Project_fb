$(warning os: $(shell uname -s))
$(info current dir is ${CURDIR})

MACHINE=$(shell uname -m)

CPPFLAGS =  -g -I. -fPIC 
LIB = libfid.so

ifeq ($(MACHINE), x86_64)
  CPPFLAGS += -DSIMULATOR
else
  CPPFLAGS += -DCAMERA
endif


SRCS =  maindelegator.cpp    web/webservice.cpp hardware/gpio.cpp hardware/switchgpio.cpp settings.cpp employeeinfomgr.cpp timesheetmgr.cpp
OBJS = $(SRCS:.cpp=.o)

all : $(LIB)
 

$(LIB) : ${OBJS} 
	g++ -shared -o $@ ${OBJS} 
	
depend : $(SRCS)
	g++ -MM $(CPPFLAGS) $^ > $@

.PHONY : clean
clean :
	-rm *.o web/*.o hardware/*.o inih_r29/*.o

INSTALL_DIR = /home/pi/acu
install:
	cp libfid.so $(INSTALL_DIR)/libs/
	cp tools/libtool.so $(INSTALL_DIR)/libs/
	-cp camera/libcamera.so $(INSTALL_DIR)/libs/
	cp inih_r29/libinih.so $(INSTALL_DIR)/libs/
	cp qt/acu/acu $(INSTALL_DIR)
	cp qt/acu/start.sh $(INSTALL_DIR)
	cp qt/acu/start_sa.sh $(INSTALL_DIR)
	cp config/FID.ini /etc/acu/ 
	
-include depend

