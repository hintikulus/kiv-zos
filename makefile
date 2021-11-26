OBJ = main.o commands.o filesystem.o inode.o linkedlist.o
BIN = main.exe
CMD = gcc -c
xCFLAGS = -Wall -Wextra -pedantic -ansi -g
CFLAGS = -g -Wall

$(BIN): $(OBJ)
	gcc $(OBJ) -o $(BIN) $(CFLAGS) -lm -static 

main.o: main.c
	$(CMD) main.c $(CFLAGS)

commands.o: commands.c commands.h
	$(CMD) commands.c $(CFLAGS)

filesystem.o: filesystem.c filesystem.h
	$(CMD) filesystem.c $(CFLAGS)

inode.o: inode.c inode.h
	$(CMD) inode.c $(CFLAGS)

linkedlist.o: linkedlist.c linkedlist.h
	$(CMD) linkedlist.c $(CFLAGS)

clean:
	rm *.o
