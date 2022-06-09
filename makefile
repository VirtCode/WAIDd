release: main.c
	@echo "-> Building WAIDd"
	mkdir -p built
	gcc main.c -o built/waidd -lX11
	@echo "-> Build completed successfully."

debug: main.c
	@echo "-> Building WAIDd debug binary"
	mkdir -p built
	gcc main.c -o built/waidd -lX11 -g
	@echo "-> Debug build completed successfully."

run: debug
	@echo "-> Running new debug build"
	./built/waidd

install: release
	-killall waidd
	cp ./built/waidd ~/.local/bin/waidd
	@echo "-> Successfully installed the WAIDd binary for this user."

install-global: release
	@echo "WARNING: This target must be run by root in order to work!"
	-killall waidd
	cp ./built/waidd /bin/waidd
	@echo "-> Successfully installed the WAIDd binary globally on this machine."

install-global-debug: debug
	@echo "WARNING: This target must be run by root in order to work!"
	-killall waidd
	cp ./built/waidd /bin/waidd
	@echo "-> Successfully installed the a debug build of the WAIDd binary globally on this machine."