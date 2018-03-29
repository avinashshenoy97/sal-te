/*
Text editor implementation -- command mode.
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
    dprintf(STDERR_FILENO, "SALTE CMD : ");
    vdprintf(STDERR_FILENO, fmt, args);
    dprintf(STDERR_FILENO, "\n");
    fflush(stderr);

    va_end(args);
}
#else
#define error_log(...) ;
#endif


void processesCommand() {
    char fc[4];
    fc[0] = getPressedKey();

    switch(fc[0]) {
        case 127:                       // process backspace
            te.cmd--;
            te.command[te.cmd] = 0;
            break;

        case IS_CTRL_KEY('q'):          // process control-q to save content and quit
            te.flush = 1;
            exit(0);
            break;

        case IS_CTRL_KEY('x'):                      // process control-x to simply quit (without saving)
            error_log("CMD Control x found!");
            te.flush = 0;
            exit(0);
            break;

        case IS_CTRL_KEY('g'):                      // switch command prompt to go-to prompt
            error_log("CMD Control G found!");
            te.p = 1;
            break;

        case ESC:                               // process escape character-sequences (arrow keys)
            fc[1] = getPressedKey();
            if(fc[1] == '[') {
                fc[2] = getPressedKey();
                switch(fc[2]) {
                    case 'A':
                        error_log("CMD UP");
                        if(te.renderOffset > 0)
                            te.renderOffset--;
                        break;

                    case 'B':
                        error_log("CMD DOWN");
                        if(te.renderOffset < (te.lines-1))
                            te.renderOffset++;
                        break;

                    case 'C':
                        error_log("CMD RIGHT");
                        break;

                    case 'D':
                        error_log("CMD LEFT");
                        break;

                    default:
                        error_log("Invalid escape sequence!");
                }
            }
            break;

        default:                                            // default action to append to command buffer till newline is found
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