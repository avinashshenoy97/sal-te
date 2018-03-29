/*
The file that contains the main function and delegates work to other modules.
*/

/* -------------------- Includes -------------------- */
#include "main.h"
#include "salte.h"
#include "salte_command.h"
#include "salte_content.h"
#include "op.h"


#ifdef DEBUG_MODE
static void error_log(char *fmt, ...);
#else
#define error_log(...) ;
#endif

/* ==================== Main ==================== */

int main(int argc, char **argv) {
    clearScreen();
    enter_raw();

    char *temp = NULL;
    if(argc > 1) {
        temp = strdup(argv[1]);
    }
    else {
        time_t rawtime;
        struct tm * timeinfo;

        time(&rawtime);
        timeinfo = localtime(&rawtime);
        
        temp = strdup(asctime(timeinfo));
        temp[strlen(temp) - 1] = 0;
    }
    init_salte(temp);


    // Register Exit Handler
    atexit(exit_handler);

    // being TE I/O
    error_log("Starting process loop");
    fflush(stdout); fflush(stdin); fflush(stderr);  // flush for correct functionality in CANON mode
    while(1) {
        fflush(stdout);
        clearScreen();
        print_banners();
        gotoxy(4, 1);

        switch(te.mode) {
            case COMMAND_MODE:
                renderData(te.renderOffset);
                gotoxy(te.prompt_row, 1);
                printf("%s : %s", te.prompt[te.p], te.command); fflush(stdout);
                processesCommand();
                break;
            
            case CONTENT_MODE:
                processContent();
                break;

            default:
                fprintf(stderr, "INVALID MODE");
        }
    }

    free(temp);
    return 0;
}


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
#endif