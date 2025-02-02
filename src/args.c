
#include <stdbool.h>
#include <stddef.h>

#include <argp.h>

#include "args.h"

/*----------------------------------------------------------------------------*/

#ifndef COL_BEFORE
#define COL_BEFORE "\x1B[7m"
#endif /* COL_BEFORE */

#ifndef COL_AFTER
#define COL_AFTER "\x1B[0m"
#endif /* COL_AFTER */

#ifndef VERSION
#define VERSION NULL
#endif /* VERSION */

const char* argp_program_version     = VERSION;
const char* argp_program_bug_address = "<8dcc.git@gmail.com>";

/*
 * Names of the mandatory command-line arguments.
 */
static char args_doc[] = "REGEXP";

/*
 * Command-line options. See:
 * https://www.gnu.org/software/libc/manual/html_node/Argp-Option-Vectors.html
 */
static struct argp_option options[] = {
    {
      "before",
      'b',
      "STR",
      0,
      "Print STR before each regular expression match, instead changing the "
      "background color.",
      0,
    },
    {
      "after",
      'a',
      "STR",
      0,
      "Print STR after each regular expression match, instead resetting the "
      "background color.",
      0,
    },
    {
      "extended-regexp",
      'e',
      0,
      0,
      "Interpret REGEXP as an extended regular expression (ERE).",
      0,
    },
    {
      "ignore-case",
      'i',
      0,
      0,
      "Do not differentiate case.",
      0,
    },
    { NULL, 0, NULL, 0, NULL, 0 }
};

/*----------------------------------------------------------------------------*/

static error_t parse_opt(int key, char* arg, struct argp_state* state) {
    /*
     * Get the 'input' argument from 'argp_parse', which we know is a pointer to
     * our 'Args' structure.
     */
    struct Args* args = state->input;

    switch (key) {
        case 'b':
            args->before = arg;
            break;
        case 'a':
            args->after = arg;
            break;
        case 'e':
            args->extended_regexp = true;
            break;
        case 'i':
            args->ignore_case = true;
            break;

        case ARGP_KEY_ARG:
            /* Too many arguments? */
            if (state->arg_num >= 1)
                argp_usage(state);

            args->regexp = arg;
            break;

        case ARGP_KEY_END:
            /* Not enough arguments? */
            if (state->arg_num < 1)
                argp_usage(state);
            break;

        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

/*----------------------------------------------------------------------------*/

void args_init(struct Args* args) {
    args->regexp          = NULL;
    args->before          = COL_BEFORE;
    args->after           = COL_AFTER;
    args->extended_regexp = false;
    args->ignore_case     = false;
}

void args_parse(int argc, char** argv, struct Args* args) {
    static struct argp argp = {
        options, parse_opt, args_doc, NULL, NULL, NULL, NULL,
    };

    argp_parse(&argp, argc, argv, 0, 0, args);
}
