#ifndef SALTE_CONTENT_H
#define SALTE_CONTENT_H

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
Initialise content mode.
*/
void initContentMode();

/*
Process I/O -- line editing - the heart of SAL-TE!
*/
void processContent();


#endif