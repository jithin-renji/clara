%{

#include "vec.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define YYDEBUG     1

extern char *cur_cmd;
extern char *cur_ch;

int yylex(void);
void yyerror(char const *);

%}

%union {
    char *word;
    Vec_t *words;
}

%token <word> WORD
%type <words> simple_command

%left '|'

%%
input:
    %empty
    | command_list
    | pipeline
    | pipeline ';' command_list
    ;

pipeline:
    simple_command '|' simple_command {
        printf("simple_command | simple_command:\n");
        vec_print($1);
        vec_print($3);
    }
    | pipeline '|' simple_command {
        printf("pipeline | simple_command:\n");
        vec_print($3);
    }

command_list:
    simple_command {
        printf("simple_command:\n");
        vec_print($1);
        vec_free($1);
    }
    | command_list ';' simple_command {
        printf("command_list ; simple_command:\n");
        vec_print($3);
        vec_free($3);
    }
    | command_list ';' pipeline
    | command_list ';'
    ;

simple_command:
    WORD {
        $$ = vec_create();
        $$ = vec_append($$, $1);
    }
    | simple_command WORD {
        $$ = vec_append($1, $2);
    }
    ;

%%

static int isreserved(char c)
{
    switch (c) {
    case '|':
    case '<':
    case '>':
    case '&':
    case ';':
        return 1;

    default:
    }

    return 0;
}

int yylex(void)
{
    while (*cur_ch == ' ' || *cur_ch == '\t') {
        cur_ch++;
    }

    if (*cur_ch && !isspace(*cur_ch) && !isreserved(*cur_ch)) {
        size_t n_char = 0;
        while (*cur_ch && !isspace(*cur_ch) && !isreserved(*cur_ch)) {
            n_char++;
            cur_ch++;
        }

        yylval.word = strndup(cur_ch - n_char, n_char + 1);
        yylval.word[n_char] = '\0';

        return WORD;
    }

    return *cur_ch++;
}

void yyerror(char const *e)
{
    fprintf(stderr, "%s\n", e);
}
