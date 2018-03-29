#ifndef SALTE_H
#define SALTE_H

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


/* -------------------- Macros -------------------- */
#define MAX_BUF_SIZE 4294967295

#define IS_CTRL_KEY(k) ((k) & 0x1f)
#define ESC '\x1b'

#define ARROW_UP 'w'
#define ARROW_DOWN 's'
#define ARROW_LEFT 'a'
#define ARROW_RIGHT 'd'

#define digit_to_char(d) (48+d)

#define COMMAND_MODE 0
#define CONTENT_MODE 1


/* -------------------- Globals -------------------- */

uint32_t maxOnScreen, onScreen;
uint16_t horizontal, vertical;

char *currFile;
int fileFD;

typedef struct salteConfig {
    struct termios old;    // terminal properties
    
    u_int32_t lines;        // Data
    uint16_t *len;
    char **buf;
    uint8_t flush;

    uint16_t x;          // Cursor position
    uint16_t y;
    uint16_t manual;
    uint16_t manualX;
    uint16_t manualY;
    uint16_t pos;       // cursor position within a line (if cursor is moved with arrows)

    int rows;       // available on terminal
    int cols;

    char **prompt;
    uint8_t p;
    uint16_t prompt_row;    // row to print prompt
    char command[25];       // command
    uint8_t cmd;            // length of command

    uint8_t mode;

    char *currentLine;      // CONTENT MODE
    uint16_t currentLen;
    uint16_t currentAlloc;
    uint16_t currentLineNo;

    uint16_t renderOffset;
}salteConfig;

salteConfig te;


/* -------------------- Function Prototypes -------------------- */

/* ---------- Signal Handlers ---------- */
void handle_signals(int signo);

/*
Print SAL-TE banners on first and last row of terminal.
*/
void print_banners();

/*
Update data maintained about screen size and content!
*/
void updateOnScreen();

/*
Initalize the text editor, clear screen, print the header and position cursor on line 2 of terminal.
*/
void init_salte(char *fileName);

/*
The exit handler for SALTE.
*/
void exit_handler(void);

/*
Raw terminal mode! No cooked I/O.
*/
void enter_raw();

/*
Read one character from STDIN.
*/
char getPressedKey();

/*
Process the key pressed (from getPressedKey). Handle control sequences.
*/
void processesCommand();

/*
Render the text content from line `fromLine`.
*/
void renderData(int fromLine);

/*
Initialise content mode.
*/
void initContentMode();

/*
Integer to string.
*/
char *itoa(int i);

/*
Process I/O
*/
void processContent();


#endif