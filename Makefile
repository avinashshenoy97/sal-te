srcfiles = ./src/
includes = ./include/
files = $(srcfiles)main.c $(srcfiles)safte.c $(srcfiles)op.c

all: drun

run: compile
	./te

compile:
	cc -o te -I$(includes) -Wall $(files)

# Debug mode

drun: dcompile
	./te

dcompile:
	cc -g -DDEBUG_MODE -o te -I$(includes) -Wall $(files)