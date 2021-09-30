build: main.c
	mkdir -p built
	gcc main.c -o built/waidd -lX11

run: build
	./built/waidd

install: build
	-killall waidd
	cp ./built/waidd ~/.local/bin/waidd