
VERSION="\"$(shell git describe --tags --abbrev=0) ($(shell git rev-parse --short HEAD))\""

CC=gcc
CPPFLAGS=-DVERSION=$(VERSION)
CFLAGS=-std=c99 -Wall -Wextra -Wpedantic -ggdb3
LDLIBS=

SRC=main.c args.c
OBJ=$(addprefix obj/, $(addsuffix .o, $(SRC)))

BIN=hl-regexp
COMPLETION=hl-regexp-completion.bash

PREFIX=/usr/local
BINDIR=$(PREFIX)/bin
COMPLETIONDIR=$(PREFIX)/share/bash-completion/completions

#-------------------------------------------------------------------------------

.PHONY: all clean install install-bin install-completion

all: $(BIN)

clean:
	rm -f $(OBJ)
	rm -f $(BIN)

install: install-bin install-completion

install-bin: $(BIN)
	install -D -m 755 $^ -t $(DESTDIR)$(BINDIR)

install-completion: $(COMPLETION)
	install -D -m 644 $^ $(DESTDIR)$(COMPLETIONDIR)/$(BIN)

#-------------------------------------------------------------------------------

$(BIN): $(OBJ)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^ $(LDLIBS)

obj/%.c.o : src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c $<
