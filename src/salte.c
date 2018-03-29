/*
General text editor implementation -- the delegator.
*/

/* -------------------- Includes -------------------- */
#include "salte.h"
#include "op.h"


/* -------------------- Function Definitions -------------------- */

/* Error logger function for THIS file ONLY. */
#ifdef DEBUG_MODE
static void error_log(char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    dprintf(STDERR_FILENO, "\n");
    dprintf(STDERR_FILENO, "SALTE : ");
    vdprintf(STDERR_FILENO, fmt, args);
    dprintf(STDERR_FILENO, "\n");
    fflush(stderr);

    va_end(args);
}
#else
#define error_log(...) ;
#endif


void init_salte(char *fileName) {
    struct winsize w;

    void *check = NULL;
    char temp;
    int currAlloc = 0;


    // Open the file
    fileFD = open(fileName, O_RDWR | O_CREAT, 0644);
    if(fileFD > 0) {
        currFile = (char *)malloc(sizeof(char) * (strlen(fileName) + 1 + 7));
        strcpy(currFile, "File : ");
        strcat(currFile, fileName);
    }
    else {
        perror("Error operning file");
        exit(0);
    }

    // Draw terminal
    clearScreen();
    print_banners();
    gotoxy(4, 1);

    // Get terminal window size
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    te.rows = w.ws_row;         // rows available for drawing
    te.cols = w.ws_col;         // columns available for drawing
    te.renderOffset = 0;        // line number to start displaying

    // command prompts
    te.prompt = (char **)malloc(sizeof(char *) * 2);
    te.prompt[0] = strdup("Line to edit");
    te.prompt[1] = strdup("Goto line");
    te.p = 0;   // current prompt
    
    te.prompt_row = w.ws_row - 1;       // where to print prompt
    te.cmd = 0;                         // length of command
    te.command[0] = 0;                  // command itself
    te.mode = COMMAND_MODE;             // initialise mode to command mode

    te.flush = 0;                       // turn of automatic flushing
    te.lines = 1;                       // lines of text present in file (minimum 1 line - could be empty line)
    te.len = (uint16_t *)malloc(sizeof(int) * te.lines);        // lengths of each line of text
    te.buf = (char **)malloc(sizeof(char *) * te.lines);        // the lines of text
    te.buf[0] = NULL;

    // Load file data to memory
    while(read(fileFD, &temp, sizeof(char)) > 0 && temp != EOF) {
        // Make space for new character read in
        te.len[te.lines - 1] += 1;
        if(te.len[te.lines - 1] > (currAlloc - 1)) {
            check = realloc(te.buf[te.lines - 1], currAlloc + 1024);
            if(check) {
                currAlloc += 1024;
                te.buf[te.lines - 1] = check;
            }
            else perror("REALLOC FAIL (while buf)");
        }

        // If newline, make space for the new line and add nul char to end of prev line
        if(temp == '\n' || temp == '\r') {
            te.buf[te.lines - 1][te.len[te.lines - 1]] = 0;
            error_log("Line %d done : %s", te.lines, te.buf[te.lines - 1]);

            currAlloc = 0;      // new line to read
            te.lines += 1;

            check = realloc(te.len, sizeof(int) * te.lines);        // extend len array
            if(check) {
                te.len = check;
                te.len[te.lines - 1] = 0;
            }
            else perror("REALLOC FAIL (while creating line in len)");

            check = realloc(te.buf, sizeof(char *) * te.lines);     // extend data array
            if(check) {
                te.buf = check;
                te.buf[te.lines - 1] = NULL;
            }
            else perror("REALLOC FAIL (while creating line in buf)");
        }
        else
            te.buf[te.lines - 1][te.len[te.lines - 1] - 1] = temp;      // if not new line, add to current line of data
    }

    // Add nul char to end of last line read in
    if(te.len[te.lines - 1] > (currAlloc - 1)) {
        check = realloc(te.buf[te.lines - 1], currAlloc + 1024);
        if(check) {
            currAlloc += 1024;
            te.buf[te.lines - 1] = check;
        }
        else perror("REALLOC FAIL (while buf)");
    }
    te.buf[te.lines - 1][te.len[te.lines - 1]] = 0;

    error_log("File read! Lines = %d", te.lines);

    // Manual mode within content mode (when arrow keys are pressed while editing a line)
    te.manual = 0;
    te.manualX = 0;
    te.manualY = 0;
}


void exit_handler(void) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &(te.old));
    
    if(te.flush) {
        ftruncate(fileFD, 0);
        if(lseek(fileFD, 0, SEEK_SET) < 0)
            perror("LSEEK error while flushing!");

        int i;
        for(i = 0 ; i < te.lines ; i++) {
            error_log("Writing line %d : %d : %s", i+1, te.len[i], te.buf[i]);
            write(fileFD, te.buf[i], (te.len[i]) * sizeof(char));
            if(i != (te.lines - 1))
                write(fileFD, "\n", sizeof(char));
        }
    }

    close(fileFD);
    clearScreen();
}


void enter_raw() {
    // get current terminal properties
    if (tcgetattr(STDIN_FILENO , &(te.old)) != 0) {
        perror("Couldn't get terminal properties!");
        exit(0);
    }

    // set raw properties
    struct termios new = te.old;
    
    new.c_iflag &= ~(IXON | ICRNL); 
    // disable ctrl-S and ctrl-Q (software data flow control), ctrl-m, 

    new.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG); 
    // disable echoing, canonical mode (so we can read byte-by-byte), ctrl-v and ctrl-o, ctrl-C

    new.c_oflag &= ~(OPOST);
    // disable output processing

    new.c_oflag &= ~(BRKINT | INPCK | ISTRIP | CS8);
    // for compatibility apparently
    
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &new) != 0) {
        perror("Couldn't set terminal properties!");
        exit(0);
    }
}


char getPressedKey() {
    char c;

    if(read(STDIN_FILENO, &c, 1) < 1) {
        perror("Read error");
        exit(0);
    }

    return c;
}


char *itoa(int i) {
    error_log("Entered ITOA with %d", i);

    char *ret = (char *)malloc(sizeof(char));
    int curr = 0;
    while(i) {
        ret = (char *)realloc(ret, (curr + 2) * sizeof(char));
        ret[curr++] = digit_to_char(i % 10);
        i /= 10;
    }
    ret[curr] = 0;

    error_log("Reversing %d; %d; %s", curr, curr/2, ret);
    // reverse
    int j; 
    char temp;
    for(i = 0, j = curr-1 ; i < (curr / 2) ; i++, j--) {
        temp = ret[i];
        ret[i] = ret[j];
        ret[j] = temp;
    }
    
    error_log("Returning ITOA : %s", ret);
    return ret;
}
