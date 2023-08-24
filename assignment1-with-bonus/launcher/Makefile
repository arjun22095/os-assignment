#Compile the launch.c by linking it with the lib_simpleloader.so
all:  launch

launch: launch.c
	gcc -m32 -I../loader -L../bin -o $@ $< -Wl,-rpath=../bin -l_simpleloader

#Provide the command for cleanup
clean:
	-@rm -f launch
