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