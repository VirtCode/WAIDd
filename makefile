WAID: main.c
	mkdir -p built
	gcc main.c -o built/WAID -lX11

run: WAID
	./built/WAID