/*
Handles all printing and output to terminal, cursor positioning on terminal, etc using ANSI escape sequences.
*/

/* -------------------- Includes -------------------- */
#include "op.h"
#include "salte.h"

/* -------------------- Function Definitions -------------------- */

/* Error logger function for THIS file ONLY. */
#ifdef DEBUG_MODE
static void error_log(char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    dprintf(STDERR_FILENO, "\n");
    dprintf(STDERR_FILENO, "OP : ");
    vdprintf(STDERR_FILENO, fmt, args);
    dprintf(STDERR_FILENO, "\n");
    fflush(stderr);

    va_end(args);
}
#else
#define error_log(...) ;
#endif


void hprintf(char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    printf(STARTH);
    vprintf(fmt, args);
    printf(ENDH);

    va_end(args);
}


char *centerify(char *str) {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    // w.ws_row = lines in terminal
    // w.ws_col = columns in terminal
    error_log("Lines = %d, Columns = %d", w.ws_row, w.ws_col);
    
    int len = strlen(str), i, j;
    char *ret = NULL;

    if(len > w.ws_col) {
        error_log("Overflowing string");

        len = w.ws_col - (len % w.ws_col);   // get length of remaining space on last line!

        error_log("Remaining : %d", len);

        int temp = strlen(str) + len + 1;
        ret = (char *)malloc(temp);
        ret[temp-1] = 0;
        
        strcpy(ret, str);

        for(i = temp - 2, j = 0 ; j < len ; i--, j++)
            ret[i] = ' ';
        
        return ret;
    }

    error_log("Normal string");

    int padding = w.ws_col - len;
    ret = (char *)malloc(sizeof(char) * (w.ws_col + 1));
    ret[w.ws_col] = 0;

    for(i = 0 ; i < padding / 2; i++)
            ret[i] = ' ';
    
    error_log("padding : %d", padding);

    if( (padding % 2) )
        ret[i] = ' ';

    for(j = i, i = 0 ; i < len ; i++, j++)
            ret[j] = str[i];
    
    for( ; j <= w.ws_col-1 ; j++)
        ret[j] = ' ';

    error_log("Returning with : %s", ret);
    return ret;
}


void gotoxy(uint32_t x, uint32_t y) {
    printf("\x1b[%d;%dH", (x), (y));
    fflush(stdout);
    te.x = y;
    te.y = x;
}


void print_banners() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    gotoxy(1, 1);
    hprintf(centerify("SAL-TE : The Simple Af Line-based Text Editor!"));
    hprintf(centerify(currFile));

    gotoxy(w.ws_row, 1);
    hprintf(centerify("; ^Q Quit ; ^X Quit without save ; ^K Clear Line ; ^G Goto Line ;"));

    gotoxy(w.ws_row-2, 1);
    hprintf(centerify("Control Section"));
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
            else {
                if(te.currentOffset)
                    te.currentOffset -= te.cols;
            }
            break;
        
        case ARROW_DOWN:
            if(te.y < (te.rows - BOTTOM_BANNER_LINES)) {
                te.manualY = te.y + 1;
                te.manualX = te.x;
                te.pos += te.cols;
            }
            else {
                te.currentOffset += te.cols;
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


void renderData(int fromLine) {
    error_log("Entered renderedData with %d", fromLine);

    int i, j;
    int rowsUsed = 0, spaceLeft = 0, temp;
    char *lineNo = NULL;

    for(i = fromLine ; i < te.lines && rowsUsed < (te.rows - TOP_BANNER_LINES - BOTTOM_BANNER_LINES) ; i++) {
        lineNo = itoa(i+1);
        temp = ((te.len[i] + strlen(lineNo) + 1) / te.cols) + 1;
        // len + n to account for line number printing
        if((rowsUsed + temp) >= (te.rows - TOP_BANNER_LINES - BOTTOM_BANNER_LINES))
            break;
        else
            rowsUsed += temp; // rows that will be used by this line

        printf("%s %s\n\r", lineNo, te.buf[i]);
    }

    if(rowsUsed < (te.rows - TOP_BANNER_LINES - BOTTOM_BANNER_LINES) && i != te.lines) {
        printf("%s ", lineNo);
        spaceLeft = ((te.rows - TOP_BANNER_LINES - BOTTOM_BANNER_LINES) - rowsUsed + 1) * te.cols;
        spaceLeft -= (1 + strlen(lineNo));
        error_log("Space left : %d", spaceLeft);
        for(j = 0 ; j < te.len[i] && j < spaceLeft ; j++) {
            printf("%c", te.buf[i][j]);
        }
    }
    fflush(stdout);
}