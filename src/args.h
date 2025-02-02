
#include <stdbool.h>

/*
 * Used by 'main' to communicate with 'parse_opt'.
 */
struct Args {
    /* Mandatory arguments */
    const char* regexp;

    /* Mandatory arguments */
    const char* before;
    const char* after;
    bool extended_regexp, ignore_case;
};

/*----------------------------------------------------------------------------*/

/*
 * Initialize an 'Args' structure.
 */
void args_init(struct Args* args);

/*
 * Parse the arguments in the 'argv' array, of length 'argc', storing the
 * results in the 'Args' structure pointed to by 'args'.
 */
void args_parse(int argc, char** argv, struct Args* args);
