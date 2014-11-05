
all : test_network  test_base64  test_timer

LFLAGS = -L.. -ltool -lpthread -lrt

test_network : ../network.o test_network.o
	g++ -g -o $@ $^ $(LFLAGS)
	
test_timer : test_timer.o
	g++ -g -o $@ $^ $(LFLAGS)

test_base64 : test_base64.o
	g++ -g -o $@ $^ $(LFLAGS)

test_network.o : ../network.h
test_media.o : ../timer.h
test_base64.o : ../base64.h

.PHONY : clean
clean:
	rm *.o test_network test_timer
