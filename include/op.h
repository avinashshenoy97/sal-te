#ifndef OP_H
#define OP_H

/* -------------------- Standard Libraries -------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>


/* -------------------- System Libraries -------------------- */
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>


/* -------------------- Macros -------------------- */
#define STARTH "\033[7m"
#define ENDH "\033[27m"
#define CLEAR "\033[2J"
#define RESETTERM "\e[3J"
#define TOPLEFT "\033[1;1H"
#define UP printf("\x1b[1A")
#define DOWN printf("\x1b[1B")
#define RIGHT printf("\x1b[1C")
#define LEFT printf("\x1b[1D")

#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

#define clearScreen() printf(CLEAR RESETTERM TOPLEFT)

#define TOP_BANNER_LINES 3
#define BOTTOM_BANNER_LINES 3


/* -------------------- Globals -------------------- */
int startPrint;


/* -------------------- Function Prototypes -------------------- */

/*
Prints output just like printf, but highlighted!
*/
void hprintf(char *fmt, ...);

/*
Return padded string, padded such that the text is at the center of the screen.
NOTE : if string is longer than terminal width, the string is right-padded such that the entire sting is printed on the terminal and the cursor will be positioned on a fresh line.
*/
char *centerify(char *str);

/*
Clear screen and position cursor at top left corner.
*/
void fresh_term();

/*
Goto x, y on terminal.
*/
void gotoxy(uint32_t x, uint32_t y);

/*
Print SAL-TE banners on first and last rows of terminal.
*/
void print_banners();

/*
Move the cursor in the direction `dir` as defined by macros : ARROW_UP ARROW_DOWN ARROW_LEFT ARROW_RIGHT.
*/
void moveCursor(char dir);

/*
Render the text content from line `fromLine`.
*/
void renderData(int fromLine);


#endif