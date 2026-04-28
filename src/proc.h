#ifndef PROC_H
#define PROC_H

#include "cmd_ast.h"
#include "vec.h"

typedef struct Proc
{
    Vec_t *argv;
    char *in_fname;
    char *out_fname;
    int outfile_append;
    pid_t pid;
    int completed;
    struct Proc *next;
} Proc_t;

typedef Proc_t Pipeline_t;

Pipeline_t *pipeline_create(void);
void pipeline_append(Pipeline_t *pipeline, Proc_t *proc);
Proc_t *proc_create(ASTNode_t *cmd);
Proc_t *proc_find(Proc_t *pipeline, pid_t pid);
void proc_exec(Proc_t *proc, pid_t pgrp, 
        int read_fd, int write_fd, int is_foreground);
void pipeline_free(Pipeline_t *pipeline);

#endif
