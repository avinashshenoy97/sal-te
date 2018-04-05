srcfiles = ./src/
includes = ./include/
files = $(srcfiles)main.c $(srcfiles)salte.c $(srcfiles)op.c $(srcfiles)salte_command.c $(srcfiles)salte_content.c
log = ./tests/log1


all: run

run: compile
	./te $(input)

compile:
	cc -o te -I$(includes) -Wall $(files)

# Debug mode

d: dcompile
	./te $(input) 2> $(log)

dcompile:
	cc -g -DDEBUG_MODE -o te -I$(includes) -Wall $(files)