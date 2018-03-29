srcfiles = ./src/
includes = ./include/
files = $(srcfiles)main.c $(srcfiles)safte.c $(srcfiles)op.c
input = ./tests/test1
log = ./tests/log1


all: run

run: compile
	./te $(input)

compile:
	cc -o te -I$(includes) -Wall $(files)

# Debug mode

d: dcompile
	./te $(input) $(log)

dcompile:
	cc -g -DDEBUG_MODE -o te -I$(includes) -Wall $(files)