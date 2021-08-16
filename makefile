build: main.c
	mkdir -p built
	gcc main.c -o built/waiddaemon -lX11

run: build
	./built/waiddaemon

install: build
	-killall waiddaemon
	# Yes, sudo in a makefile is probably not a good practice
	sudo -S cp ./built/waiddaemon /bin/waiddaemon