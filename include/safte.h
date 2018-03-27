/* -------------------- Standard Libraries -------------------- */
#include <stdio.h>
#include <string.h>


/* -------------------- System Libraries -------------------- */
#include <unistd.h>
#include <fcntl.h>


/* -------------------- Macros -------------------- */
#define BUFF_SIZE 1024      // data buffer in bytes


/* -------------------- Globals -------------------- */
char buf[BUFF_SIZE];


/* -------------------- Function Prototypes -------------------- */

void print_banners();

/*
Initalize the text editor, clear screen, print the header and position cursor on line 2 of terminal.
*/
void init_safte();

