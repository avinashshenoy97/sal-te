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

    gotoxy(1, 1);
    hprintf(centerify("SAF-TE : The Simple AF Text Editor!"));
    hprintf(centerify(currFile));

    gotoxy(w.ws_row, 1);
    hprintf(centerify("; ^Q Quit ; ^X Quit without save ; ^K Clear Line ;"));

    gotoxy(w.ws_row-2, 1);
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
    gotoxy(4, 1);

    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    te.rows = w.ws_row;         // rows available for drawing
    te.cols = w.ws_col;
    te.renderOffset = 0;

    // command prompts
    te.prompt = (char **)malloc(sizeof(char *) * 2);
    te.prompt[0] = strdup("Line to edit");
    te.prompt[1] = strdup("Goto line");
    te.p = 0;
    
    te.prompt_row = w.ws_row - 1;
    te.cmd = 0;
    te.command[0] = 0;
    te.mode = COMMAND_MODE;

    // Load file data to memory
    void *check = NULL;
    char temp;
    int currAlloc = 0;

    te.flush = 0;
    te.lines = 1;
    te.len = (uint16_t *)malloc(sizeof(int) * te.lines);
    te.buf = (char **)malloc(sizeof(char *) * te.lines);
    te.buf[0] = NULL;

    error_log("Reading file...");

    while(read(fileFD, &temp, sizeof(char)) > 0 && temp != EOF) {
        // Make space for new character read in
        te.len[te.lines - 1] += 1;
        if(te.len[te.lines - 1] > (currAlloc - 1)) {
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

    error_log("check1");

    if(te.len[te.lines - 1] > (currAlloc - 1)) {
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


void moveCursor(char dir) {
    if(te.manual == 0) {
        te.manual = 1;
        te.pos = te.currentLen;
    }

    switch(dir) {
        case ARROW_UP:
            if(te.y > (TOP_BANNER_LINES + 1)) {
                te.manualY = te.y - 1;
                te.manualX = te.x;
                te.pos -= te.cols;
            }
            break;
        
        case ARROW_DOWN:
            if(te.y < (te.rows - BOTTOM_BANNER_LINES)) {
                te.manualY = te.y + 1;
                te.manualX = te.x;
                te.pos += te.cols;
            }
            break;

        case ARROW_LEFT:
            if(te.x > 1) {
                te.manualX = te.x - 1;
                te.manualY = te.y;
            }
            else {
                te.x = te.cols;
                moveCursor(ARROW_UP);
            }
            te.pos -= 1;
            break;
        
        case ARROW_RIGHT:
            if(te.x < te.cols) {
                te.manualX = te.x + 1;
                te.manualY = te.y;
            }
            else {
                te.x = 1;
                moveCursor(ARROW_DOWN);
            }
            te.pos += 1;
            break;

        default:
            error_log("MOVE CURSOR Invalid direction");
            return;
    }
    
    error_log("MOVE CURSOR applied (%d, %d) %d", te.manualX, te.manualY, te.pos);
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
        case 127:
            te.cmd--;
            te.command[te.cmd] = 0;
            break;
            
        case IS_CTRL_KEY('q'):
            te.flush = 1;
            exit(0);
            break;

        case IS_CTRL_KEY('x'):
            error_log("CMD Control x found!");
            te.flush = 0;
            exit(0);
            break;

        case IS_CTRL_KEY('g'):
            error_log("CMD Control G found!");
            te.p = 1;
            break;

        case ESC:
            fc[1] = getPressedKey();
            fc[2] = getPressedKey();
            processed += 2;
            if(fc[1] == '[') {
                error_log("Arrow key in command mode");
                break;
            }
            break;

        default:
            if(te.mode == COMMAND_MODE) {
                if(fc[0] == '\n' || fc[0] == '\r') {
                    if(te.cmd > 0) {
                        error_log("Found newline");
                        if(te.p == 0) {
                            te.mode = CONTENT_MODE;
                            initContentMode();
                        }
                        else if(te.p == 1) {
                            te.command[te.cmd] = 0;
                            te.renderOffset = atoi(te.command) - 1;
                            error_log("Going to %d", te.renderOffset);
                            te.cmd = 0;
                            te.command[0] = 0;
                            te.p = 0;
                        }
                    }
                    else {
                        error_log("Simple enter");
                    }
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

    for(i = fromLine ; i < te.lines && rowsUsed < (te.rows - TOP_BANNER_LINES - BOTTOM_BANNER_LINES) ; i++) {
        lineNo = itoa(i+1);
        rowsUsed += ((te.len[i] + strlen(lineNo) + 1) / te.cols) + 1; // rows that will be used by this line
        // len + n to account for line number printing
        if(rowsUsed >= (te.rows - TOP_BANNER_LINES - BOTTOM_BANNER_LINES))
            break;
        
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


void initContentMode() {
    error_log("Initialising content mode");
    
    uint32_t command = atoi(te.command);
    error_log("ATOI command = %d", command);

    void *temp;
    if(command > te.lines) {
        error_log("Larger command line %d vs %d", command, te.lines);
        int i = te.lines;
        temp = realloc(te.buf, command * sizeof(char *));
        if(temp) {
            te.buf = temp;
            temp = realloc(te.len, command * sizeof(int));
            if(temp) {
                te.len = temp;
                for(i = te.lines ; i < command ; i++) {
                    te.buf[i] = (char *)calloc(1, sizeof(char));
                    te.len[i] = 0;
                }
                te.lines = command;
            }
            else {
                perror("REALLOC error adding lines to len");
                exit(0);
            }
        }
        else {
            perror("REALLOC error adding lines");
            exit(0);
        }
    }
    
    te.currentLineNo = command;
    te.currentLen = strlen(te.buf[command-1]);
    te.currentLine = strdup(te.buf[command-1]);
    te.currentAlloc = te.currentLen + 1024;

    error_log("INIT CURRLEN %d", te.currentLen);

    temp = realloc(te.currentLine, te.currentAlloc * sizeof(char));
    if(temp)
        te.currentLine = temp;
    else {
        perror("REALLOC currentLine");
        exit(0);
    }
}


void processContent() {
    error_log("Entered processContent");

    int temp = te.currentLineNo;
    while(temp) {
        te.x++;
        temp /= 10;
    }
    error_log("BEFORE adding strings (%d, %d)", te.x, te.y);

    te.x++;
    te.x += te.currentLen;
    error_log("BEFORE adjusting cols (%d, %d)", te.x, te.y);

    while(te.x > te.cols) {
        te.x -= te.cols;
        te.y++;
    }

    printf("%d %s", te.currentLineNo, te.currentLine); fflush(stdout);
    error_log("(x, y) = (%d, %d) ; %d %s", te.x, te.y, te.currentLineNo, te.currentLine);
    
    if(te.manual) {
        gotoxy(te.manualY, te.manualX);
        te.x = te.manualX;
        te.y = te.manualY;
        error_log("POS : %d", te.pos);
    }

    char c = getPressedKey();
    error_log("Content key : %d", c);
    switch(c) {
        case IS_CTRL_KEY('k'):
            error_log("Control K found!");
            te.currentLen = 0;
            te.currentLine[te.currentLen] = 0;
            break;

        case IS_CTRL_KEY('q'):
            error_log("Control Q found!");
            te.flush = 1;
            exit(0);
            break;

        case IS_CTRL_KEY('x'):
            error_log("Control x found!");
            te.flush = 0;
            exit(0);
            break;

        case ESC:
            c = getPressedKey();
            if(c == '[') {
                c = getPressedKey();
                switch(c) {
                    case 'A':
                        error_log("UP");
                        moveCursor(ARROW_UP);
                        break;

                    case 'B':
                        error_log("DOWN");
                        moveCursor(ARROW_DOWN);
                        break;

                    case 'C':
                        error_log("RIGHT");
                        moveCursor(ARROW_RIGHT);
                        break;

                    case 'D':
                        error_log("LEFT");
                        moveCursor(ARROW_LEFT);
                        break;

                    default:
                        error_log("Invalid escape sequence!");
                }
            }
            break;
        
        case 127:
            if(te.currentLen) {
                error_log("Content bck : %c", te.currentLine[te.currentLen]);
                te.currentLen--;
                te.currentLine[te.currentLen] = 0;
            }
            break;

        case '\n': case '\r':
            free(te.buf[te.currentLineNo - 1]);
            te.buf[te.currentLineNo - 1] = te.currentLine;
            te.len[te.currentLineNo - 1] = te.currentLen;
            te.currentAlloc = 0;
            te.currentLen = 0;
            te.currentLine = NULL;
            te.currentLineNo = 0;
            te.command[0] = 0;
            te.cmd = 0;
            te.manual = 0;
            te.mode = COMMAND_MODE;
            break;

        default:
            if(te.manual) {
                if(te.pos >= te.currentLen) {
                    error_log("pos >= len");
                    while((te.pos+1) > te.currentAlloc) {
                        te.currentAlloc += 1024;
                        te.currentLine = realloc(te.currentLine, te.currentAlloc * sizeof(char));
                    }
                    int i;
                    for(i = te.currentLen ; i < te.pos ; i++)
                        te.currentLine[i] = ' ';
                    te.currentLen = te.pos + 1;
                }
                else {
                    te.currentLen += 1;
                    if(te.currentLen > (te.currentAlloc - 1)) {
                        te.currentAlloc += 1024;
                        te.currentLine = realloc(te.currentLine, te.currentAlloc);
                    }
                    int i;
                    for(i = te.currentLen ; i > te.pos ; i--)
                        te.currentLine[i] = te.currentLine[i-1];
                }
                moveCursor(ARROW_RIGHT);
            }
            else{
                te.currentLen += 1;
                if(te.currentLen > (te.currentAlloc - 1)) {
                    te.currentAlloc += 1024;
                    te.currentLine = realloc(te.currentLine, te.currentAlloc);
                }
                te.pos = te.currentLen;
            }
            te.currentLine[te.pos - 1] = c;
            error_log("ADDED Content key : %d", c);
    }
}