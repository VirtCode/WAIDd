build: main.c
	@echo "-> Building WAIDd"
	mkdir -p built
	gcc main.c -o built/waidd -lX11
	@echo "-> Build completed successfully."

run: build
	@echo "-> Running new build"
	./built/waidd

install: build
	-killall waidd
	cp ./built/waidd ~/.local/bin/waidd
	@echo "\n-> Successfully installed the WAIDd binary for this user."

install-global: build
	@echo "WARNING: This target must be run by root in order to work!"
	-killall waidd
	cp ./built/waidd /bin/waidd
	@echo "\n-> Successfully installed the WAIDd binary globally on this machine."