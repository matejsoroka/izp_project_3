project=proj3
CFLAGS=-std=c99 -Wall -Wextra
$(project): -lm $(project).o
clean:
	-rm $(project) $(project).o
