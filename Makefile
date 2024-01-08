CFLAGS=-Wall -Werror -Wextra

maze: maze.c
	gcc -std=c11 $(CFLAGS) maze.c -o maze

clean:
	rm -f maze