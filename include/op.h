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
#define TOPLEFT "\033[1;1H"

#define gotoxy(x,y) printf("\033[%d;%dH", (x), (y))

/* -------------------- Globals -------------------- */


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