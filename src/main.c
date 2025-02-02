/*
 * Copyright 2025 8dcc
 *
 * This file is part of hl-regexp.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <errno.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>

#include "util.h"
#include "args.h"

/*----------------------------------------------------------------------------*/

#define ENSURE_ALLOCATION(PTR)                                                 \
    do {                                                                       \
        if ((PTR) == NULL) {                                                   \
            ERR("Error allocating '%s': %s", #PTR, strerror(errno));           \
            return NULL;                                                       \
        }                                                                      \
    } while (0)

/*----------------------------------------------------------------------------*/

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

static bool get_match(size_t* start_idx, size_t* end_idx, const char* regexp,
                      const char* str, bool extended_regexp, bool ignore_case) {
    static regex_t r;
    static regmatch_t matches[1];

    /* Reset destination pointers */
    *start_idx = 0;
    *end_idx   = 0;

    int cflags = 0;
    if (extended_regexp)
        cflags |= REG_EXTENDED;
    if (ignore_case)
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
                                   struct Args* args) {
    for (;;) {
        size_t match_start, match_end;
        if (!get_match(&match_start,
                       &match_end,
                       args->regexp,
                       line,
                       args->extended_regexp,
                       args->ignore_case)) {
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
        printf("%s", args->before);
        while (i < match_end)
            fputc(line[i++], dst);
        printf("%s", args->after);

        /*
         * Since the there might be more that one match per line, continue
         * checking the remainder of the line on the next iteration.
         */
        line = &line[match_end];
    }
}

/*----------------------------------------------------------------------------*/

int main(int argc, char** argv) {
    struct Args args;
    args_init(&args);
    args_parse(argc, argv, &args);
    assert(args.regexp != NULL);

    FILE* src = stdin;
    FILE* dst = stdout;

    char* line;
    while ((line = get_line(src)) != NULL) {
        print_highlighted_line(dst, line, &args);
        free(line);
    }

    return 0;
}
