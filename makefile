CC = gcc
FLAGS = -Wall -m64 -g
OPENGL_FLAGS =  -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lglfw -Werror -no-pie

all: main.o x86_function.o glad.o
	$(CC) $(CFLAGS) -o program main.o x86_function.o glad.o $(OPENGL_FLAGS)

x86_function.o: src/x86_function.s
	nasm -f elf64 -o x86_function.o src/x86_function.s

main.o: src/main.c
	$(CC) $(CFLAGS) -c -o main.o src/main.c

glad.o: src/glad.c
	$(CC) $(CFLAGS) -c -o glad.o src/glad.c

clean:
	rm -f *.o

