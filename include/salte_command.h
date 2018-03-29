#ifndef SALTE_COMMAND_H
#define SALTE_COMMAND_H

/* -------------------- Standard Libraries -------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


/* -------------------- System Libraries -------------------- */
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>


/* -------------------- Function Prototypes -------------------- */

/*
Process keys pressed in command mode. Handles scrolling (with arrow keys), store command in buffer for interpretation. Handle control sequences.
*/
void processesCommand();


#endif