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

char *currFile;
int fileFD;

typedef struct salteConfig {
    // About the terminal
    struct termios old;    // terminal properties
    uint16_t rows;       // available on terminal for drawing
    uint16_t cols;
    
    // About the data
    u_int32_t lines;        // lines of data
    uint16_t *len;          // length of lines
    char **buf;             // data
    uint8_t flush;          // auto save or not
    uint16_t renderOffset;  // which line to start printing

    // About the cursor
    uint16_t x;          // Cursor position
    uint16_t y;
    uint16_t manual;        // manual cursor movement = True or False
    uint16_t manualX;       // cursor position
    uint16_t manualY;
    uint16_t pos;       // cursor position within a line (in buf) (if cursor is moved with arrows)
    
    // Current mode of execution == command / content mode
    uint8_t mode;

    // About the prompt
    char **prompt;
    uint8_t p;
    uint16_t prompt_row;    // row to print prompt
    char command[25];       // command
    uint8_t cmd;            // length of command

    // About the content (i.e the chosen line)
    char *currentLine;          // CONTENT MODE
    uint16_t currentLen;
    uint16_t currentAlloc;
    uint16_t currentLineNo;
    uint16_t currentOffset;
    
}salteConfig;

salteConfig te;


/* -------------------- Function Prototypes -------------------- */

/* ---------- Signal Handlers ---------- */
void handle_signals(int signo);

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
Integer to (dynamically allocated) string.
*/
char *itoa(int i);


#endif