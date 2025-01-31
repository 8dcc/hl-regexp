
#include <errno.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <regex.h>

#define LENGTH(ARR) (sizeof(ARR) / sizeof((ARR)[0]))

#define ERR(...)                                                               \
    do {                                                                       \
        fputs("hl-regexp: ", stderr);                                          \
        fprintf(stderr, __VA_ARGS__);                                          \
        fputc('\n', stderr);                                                   \
    } while (0)

#define ENSURE_ALLOCATION(PTR)                                                 \
    do {                                                                       \
        if ((PTR) == NULL) {                                                   \
            ERR("Error allocating '%s': %s", #PTR, strerror(errno));           \
            return NULL;                                                       \
        }                                                                      \
    } while (0)

/*----------------------------------------------------------------------------*/

#define USAGE_FP stdout
#define USAGE                                                                  \
    "Usage:\n"                                                                 \
    "  %s [OPTION...] REGEXP < INPUT\n\n"                                      \
    "Options:\n"                                                               \
    "  -h, --help\n"                                                           \
    "      Show this help and exit the program.\n\n"                           \
    "  -b STR\n"                                                               \
    "  --before STR\n"                                                         \
    "      Specify the string that should be printed before each\n"            \
    "      regular expression match.\n\n"                                      \
    "  -a STR\n"                                                               \
    "  --after STR\n"                                                          \
    "      Specify the string that should be printed after each\n"             \
    "      regular expression match.\n\n"                                      \
    "  -e, --extended-regexp\n"                                                \
    "      Interpret REGEXP as an extended regular expression (ERE).\n\n"      \
    "  -i, --ignore-case\n"                                                    \
    "      Do not differentiate case.\n"

#define COL_BEFORE "\x1B[7m"
#define COL_AFTER  "\x1B[0m"

/* Globals modified by command-line arguments */
static const char* g_regexp   = NULL;
static const char* g_before   = COL_BEFORE;
static const char* g_after    = COL_AFTER;
static bool g_ignore_case     = false;
static bool g_extended_regexp = false;

/*----------------------------------------------------------------------------*/

/*
 * TODO: Use 'getopt_long', see:
 * https://www.gnu.org/software/libc/manual/html_node/Getopt.html
 */
static int parse_args(int argc, char** argv) {
    if (argc <= 1) {
        ERR("Not enough arguments, expected a REGEXP.");
        return 2;
    }

    g_regexp = argv[argc - 1];
    if (!strcmp(g_regexp, "-h") || !strcmp(g_regexp, "--help"))
        goto print_usage;

    const int last_argc = argc - 1;
    for (int i = 1; i < last_argc; i++) {
        const char* arg = argv[i];
        if (arg[0] != '-') {
            ERR("Invalid argument '%s'.", arg);
            return 2;
        }

        const int j = (arg[1] == '-') ? 2 : 1;
        switch (arg[j]) {
            case 'h': { /* Help */
print_usage:
                fprintf(USAGE_FP, USAGE, argv[0]);
                return 1;
            }

            case 'b': { /* Before */
                i++;
                if (i >= last_argc) {
                    ERR("Not enough arguments for option '%s'.", arg);
                    return 2;
                }

                g_before = argv[i];
            } break;

            case 'a': { /* After */
                i++;
                if (i >= last_argc) {
                    ERR("Not enough arguments for option '%s'.", arg);
                    return 2;
                }

                g_after = argv[i];
            } break;

            case 'e': { /* Extended regexps */
                g_extended_regexp = true;
            } break;

            case 'i': { /* Ignore case */
                g_ignore_case = true;
            } break;

            default: {
                ERR("Invalid argument '%s'.", arg);
                return 2;
            }
        }
    }

    return 0;
}

static char* get_line(FILE* fp) {
    /* NOTE: This method only works if we read lines from a single file */
    static bool got_eof = false;
    if (got_eof)
        return NULL;

    size_t line_sz  = 100;
    size_t line_pos = 0;
    char* line      = malloc(line_sz);
    ENSURE_ALLOCATION(line);

    int c = 0;
    while (c != '\n') {
        c = fgetc(fp);
        if (c == EOF) {
            got_eof = true;
            break;
        }

        if (line_pos + 1 >= line_sz) {
            line_sz *= 2;
            line = realloc(line, line_sz);
            ENSURE_ALLOCATION(line);
        }

        line[line_pos++] = c;
    }

    line[line_pos] = '\0';
    return line;
}

static bool get_match(const char* regexp, const char* str, size_t* start_idx,
                      size_t* end_idx) {
    static regex_t r;
    static regmatch_t matches[1];

    /* Reset destination pointers */
    *start_idx = 0;
    *end_idx   = 0;

    int cflags = 0;
    if (g_extended_regexp)
        cflags |= REG_EXTENDED;
    if (g_ignore_case)
        cflags |= REG_ICASE;

    if (regcomp(&r, regexp, cflags) != REG_NOERROR) {
        ERR("Failed to compile regular expression \"%s\".", regexp);
        return false;
    }

    const int code = regexec(&r, str, LENGTH(matches), matches, 0);
    regfree(&r);

    if (code != REG_NOERROR)
        return false;

    *start_idx = matches[0].rm_so;
    *end_idx   = matches[0].rm_eo;
    return true;
}

static void print_highlighted_line(FILE* dst, const char* line,
                                   const char* regexp) {
    for (;;) {
        size_t match_start, match_end;
        if (!get_match(regexp, line, &match_start, &match_end)) {
            /*
             * No more matches left, print the remainder of the line without
             * highlighting anything and stop.
             */
            fprintf(dst, "%s", line);
            break;
        }

        /*
         * We got a match, print the text before the match, the text for
         * highlighting it, and the match text itself.
         */
        size_t i = 0;
        while (i < match_start)
            fputc(line[i++], dst);
        printf("%s", g_before);
        while (i < match_end)
            fputc(line[i++], dst);
        printf("%s", g_after);

        /*
         * Since the there might be more that one match per line, continue
         * checking the remainder of the line on the next iteration.
         */
        line = &line[match_end];
    }
}

/*----------------------------------------------------------------------------*/

int main(int argc, char** argv) {
    const int args_code = parse_args(argc, argv);
    if (args_code != 0)
        return args_code;
    assert(g_regexp != NULL);

    FILE* src = stdin;
    FILE* dst = stdout;

    char* line;
    while ((line = get_line(src)) != NULL) {
        print_highlighted_line(dst, line, g_regexp);
        free(line);
    }

    return 0;
}
