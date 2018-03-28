/*
The file that contains the main function and delegates work to other modules.
*/

/* -------------------- Includes -------------------- */
#include "main.h"
#include "safte.h"
#include "op.h"


/* ==================== Main ==================== */

int main(int argc, char **argv) {
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
    init_safte(temp);


    // Register Exit Handler
    atexit(exit_handler);

    // being TE I/O
    
    fflush(stdout); fflush(stdin); fflush(stderr);  // flush for correct functionality in CANON mode
    while(1) {
        clearScreen();
        print_banners();
        gotoxy(4, 0);
        renderData(0);
        gotoxy(te.prompt_row, 0);
        printf("Line to edit : %s", te.command); fflush(stdout);
        switch(te.mode) {
            case COMMAND_MODE:
                processesCommand();
                break;
            
            case CONTENT_MODE:
                processContent();
                break;

            default:
                error_log("INVALID MODE");
        }
    }

    free(temp);
    return 0;
}
