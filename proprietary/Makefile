OBJ = compareVIMG.o
OUT = libcompareVIMG.so 
CFLAGS = -g -O2 -fPIC
CC = gcc

default: $(OUT)

$(OUT): $(OBJ)
	$(CC) --shared -o $(OUT) $(OBJ) $(CFLAGS)

test:	
	$(CC) --shared -o libcompareVIMG_default.so  $(CFLAGS) compareVIMG_default.c 

clean:
	rm -f $(OBJ) $(OUT)


