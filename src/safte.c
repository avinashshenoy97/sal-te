/*
Text editor implementation.
*/

/* -------------------- Includes -------------------- */
#include "safte.h"
#include "op.h"


/* -------------------- Function Definitions -------------------- */

void print_banners() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    hprintf(centerify("SAF-TE : The Simple AF Text Editor!"));

    gotoxy(w.ws_row, 0);
    hprintf(centerify("; ^X Quit ; ^C Exit to Shell ;"));
}


void init_safte() {
    fresh_term();
    print_banners();
    gotoxy(2, 0);
}