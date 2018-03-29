/*
Text editor implementation -- content mode.
*/

/* -------------------- Includes -------------------- */
#include "salte.h"
#include "salte_command.h"
#include "salte_content.h"
#include "op.h"


/* -------------------- Function Definitions -------------------- */

/* Error logger function for THIS file ONLY. */
#ifdef DEBUG_MODE
static void error_log(char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    dprintf(STDERR_FILENO, "\n");
    dprintf(STDERR_FILENO, "SALTE CONTENT : ");
    vdprintf(STDERR_FILENO, fmt, args);
    dprintf(STDERR_FILENO, "\n");
    fflush(stderr);

    va_end(args);
}
#else
#define error_log(...) ;
#endif


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
    te.currentOffset = 0;

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

    printf("%d %s", te.currentLineNo, te.currentLine + te.currentOffset); fflush(stdout);
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