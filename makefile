OBJ = main.o commands.o filesystem.o inode.o linkedlist.o
BIN = main.exe
CMD = gcc -c

$(BIN): $(OBJ)
	gcc $(OBJ) -o $(BIN) -g -lm

main.o: main.c
	$(CMD) main.c -g

commands.o: commands.c commands.h
	$(CMD) commands.c -g

filesystem.o: filesystem.c filesystem.h
	$(CMD) filesystem.c -g

inode.o: inode.c inode.h
	$(CMD) inode.c -g

linkedlist.o: linkedlist.c linkedlist.h
	$(CMD) linkedlist.c -g

clean:
	rm *.o
