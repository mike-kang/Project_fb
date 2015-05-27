OBJ = compareVIMG.o
OUT = libcompareVIMG.so 
CPPFLAGS = -g -O2 -fPIC
CC = gcc

default: $(OUT)

$(OUT): $(OBJ)
	$(CC) --shared -o $(OUT) $(OBJ) 

test:	
	$(CC) --shared -o libcompareVIMG_default.so  compareVIMG_default.c

clean:
	rm -f $(OBJ) $(OUT)


