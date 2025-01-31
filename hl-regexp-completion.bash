#!/usr/bin/env bash

_hl_regexp_completion() {
    local opts
    opts=(
        -h --help
        -b --before
        -a --after
        -i --ignore-case
    )

    # Check the the previous option ('$3') for special values or options.
    case "$3" in
        '2>' | '>' | '<')
            # If it was a redirector, show the default file completion.
            compopt -o bashdefault -o default
            return
            ;;

        '-b' | '--before' | '-a' | '--after')
            # These options expect an extra parameter, so don't show completion.
            return
            ;;
    esac

    # If the current option ('$2') starts with a dash, return (in '$COMPREPLY')
    # the possible completions for the current option using 'compgen'.
    if [[ "$2" =~ -* ]]; then
        mapfile -t COMPREPLY < <(compgen -W "${opts[*]}" -- "$2")
    fi
}

complete -F _hl_regexp_completion hl-regexp
