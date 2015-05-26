OBJ = compareVIMG.o
OUT = libcompareVIMG.so 
CPPFLAGS = -g -O2 -fPIC
CC = g++

default: $(OUT)

$(OUT): $(OBJ)
	$(CC) --shared -o $(OUT) $(OBJ) 

clean:
	rm -f $(OBJ) $(OUT)


