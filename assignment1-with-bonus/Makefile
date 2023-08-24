#invoke make inside following directories and in this order: loader, launch, fib
#move the lib_simpleloader.so and launch binaries inside bin directory
#Provide the command for cleanup
all:
	make -C loader
	mv loader/lib_simpleloader.so bin/
	make -C launcher
	mv launcher/launch bin/
	make -C test
	mv test/fib bin/

clean:
	-@rm -f  bin/lib_simpleloader.so bin/launch bin/fib
