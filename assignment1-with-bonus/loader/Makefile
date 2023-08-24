#Create lib_simpleloader.so from loader.c
all: lib_simpleloader.so

lib_simpleloader.so: loader.c loader.h
	gcc -fPIC -m32 -shared -I. loader.c -o lib_simpleloader.so

#Provide the command for cleanup
clean:
	-@rm -f lib_simpleloader.so
