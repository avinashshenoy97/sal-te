/*
Text editor implementation.
*/

/* -------------------- Includes -------------------- */
#include "safte.h"
#include "op.h"


/* -------------------- Function Definitions -------------------- */

/* Error logger function for THIS file ONLY. */
#ifdef DEBUG_MODE
static void error_log(char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    dprintf(STDERR_FILENO, "\n");
    dprintf(STDERR_FILENO, "SAFTE : ");
    vdprintf(STDERR_FILENO, fmt, args);
    dprintf(STDERR_FILENO, "\n");
    fflush(stderr);

    va_end(args);
}
#else
#define error_log(...) ;
#endif


void print_banners() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    gotoxy(0, 0);
    hprintf(centerify("SAF-TE : The Simple AF Text Editor!"));
    hprintf(centerify(currFile));

    gotoxy(w.ws_row, 0);
    hprintf(centerify("; ^Q Quit ; ^K Clear Line ;"));

    gotoxy(w.ws_row-2, 0);
    hprintf(centerify("Control Section"));
}


void updateOnScreen() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    maxOnScreen = (w.ws_row - 4) * w.ws_col;
}


void init_safte(char *fileName) {
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
    gotoxy(4, 0);

    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    te.rows = w.ws_row - 6;         // rows available for drawing
    te.cols = w.ws_col;

    te.prompt_row = w.ws_row - 1;
    te.cmd = 0;
    te.command[0] = 0;
    te.mode = COMMAND_MODE;

    // Load file data to memory
    void *check = NULL;
    char temp;
    int currAlloc = 0;

    te.lines = 1;
    te.len = (uint16_t *)malloc(sizeof(int) * te.lines);
    te.buf = (char **)malloc(sizeof(char *) * te.lines);
    te.buf[0] = NULL;

    error_log("Reading file...");

    while(read(fileFD, &temp, sizeof(char)) > 0 && temp != EOF) {
        // Make space for new character read in
        te.len[te.lines - 1] += 1;
        if(te.len[te.lines - 1] > currAlloc) {
            check = realloc(te.buf[te.lines - 1], currAlloc + 1024);
            if(check) {
                currAlloc += 1024;
                te.buf[te.lines - 1] = check;
            }
            else {
                perror("REALLOC FAIL (while buf)");
            }
        }

        // If newline, make space for the new line and add nul char to prev line
        if(temp == '\n' || temp == '\r') {
            te.buf[te.lines - 1][te.len[te.lines - 1]] = 0;

            error_log("Line %d done : %s", te.lines, te.buf[te.lines - 1]);

            currAlloc = 0;
            te.lines += 1;
            check = realloc(te.len, sizeof(int) * te.lines);
            
            if(check) {
                te.len = check;
                te.len[te.lines - 1] = 0;
            }
            else
                perror("REALLOC FAIL (while creating line in len)");

            check = realloc(te.buf, sizeof(char *) * te.lines);
            if(check) {
                te.buf = check;
                te.buf[te.lines - 1] = NULL;
            }
            else
                perror("REALLOC FAIL (while creating line in buf)");
        }
        else
            te.buf[te.lines - 1][te.len[te.lines - 1] - 1] = temp;
    }

    te.len[te.lines - 1] += 1;
    if(te.len[te.lines - 1] > currAlloc) {
        check = realloc(te.buf[te.lines - 1], currAlloc + 1024);
        if(check) {
            currAlloc += 1024;
            te.buf[te.lines - 1] = check;
        }
        else {
            perror("REALLOC FAIL (while buf)");
        }
    }
    te.buf[te.lines - 1][te.len[te.lines - 1]] = 0;

    error_log("File read! Lines = %d", te.lines);

    te.x = 0;
    te.y = 0;
}


void exit_handler(void) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &(te.old));
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


void moveCursor(char dir) {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    switch(dir) {
        case ARROW_UP:
            if(te.y > 0) {
                te.y--;
                //UP;
            }
            break;
        
        case ARROW_DOWN:
            if(te.y < w.ws_col) {
                te.y++;
                //DOWN;
            }
            break;

        case ARROW_LEFT:
            if(te.x > 0) {
                te.x--;
                //LEFT;
            }
            break;
        
        case ARROW_RIGHT:
            if(te.x < w.ws_row) {
                te.x++;
                //RIGHT;
            }
            break;

        default:
            fprintf(stderr, "Invalid direction");
    }
    gotoxy(te.x + 4, te.y);
}


char getPressedKey() {
    char c;

    if(read(STDIN_FILENO, &c, 1) < 1) {
        perror("Read error");
        exit(0);
    }

    return c;
}


void processesCommand() {
    char fc[4];
    fc[0] = getPressedKey();
    int processed = 1;

    switch(fc[0]) {
        case IS_CTRL_KEY('q'):
            exit(0);
            break;

        case ESC:
            fc[1] = getPressedKey();
            fc[2] = getPressedKey();
            processed += 2;
            if(fc[1] == '[') {
                /*switch(fc[2]) {
                    case 'A':
                        moveCursor(ARROW_UP);
                        break;
                    
                    case 'B':
                        moveCursor(ARROW_DOWN);
                        break;
                    
                    case 'C':
                        moveCursor(ARROW_RIGHT);
                        break;

                    case 'D':
                        moveCursor(ARROW_LEFT);
                        break;
                }*/
                break;
            }
            break;

        default:
            if(te.mode == COMMAND_MODE) {
                if(fc[0] == '\n' || fc[0] == '\r') {
                    error_log("Found newline");
                    te.mode = CONTENT_MODE;
                    break;
                }
                else {
                    te.command[te.cmd++] = fc[0];
                }
                te.command[te.cmd] = 0;
            }
            break;
    }
}


void renderData(int fromLine) {
    error_log("Entered renderedData with %d", fromLine);

    int i;
    int rowsUsed = 0;
    char *lineNo = NULL;

    for(i = 0 ; i < te.lines && rowsUsed < te.rows ; i++) {
        lineNo = itoa(i+1);
        rowsUsed += ((te.len[i] + strlen(lineNo) + 1) / te.cols) + 1; // rows that will be used by this line
        // len + n to account for line number printing
        
        printf("%s %s\n\r", lineNo, te.buf[i]);
        fflush(stdout);
    }
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


void processContent() {


    char c = getPressedKey();
    //int pos = strlen(te.buf[]);

    switch(c) {
        case IS_CTRL_KEY('k'):
            //pos = 0;
            break;

        case IS_CTRL_KEY('q'):
            exit(0);
            break;
    }
}