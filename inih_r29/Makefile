# Simple makefile to build inih as a static library using g++

OBJ = ini.o INIReader.o
OUT = libinih.so 
CPPFLAGS = -g -O2 -fPIC
CC = g++

default: $(OUT)



$(OUT): $(OBJ)
	$(CC) --shared -o $(OUT) $(OBJ) $(EXTRAARFLAGS)

clean:
	rm -f $(OBJ) $(OUT)
