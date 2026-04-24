#ifndef PROC_H
#define PROC_H

#include "vec.h"

typedef struct Proc
{
    Vec_t *argv;
    pid_t pid;
    int completed;
    struct Proc *next;
} Proc_t;

typedef Proc_t Pipeline_t;

Pipeline_t *pipeline_create(void);
void pipeline_append(Pipeline_t *pipeline, Proc_t *proc);
Proc_t *proc_create(Vec_t *argv);
Proc_t *proc_find(Proc_t *pipeline, pid_t pid);
void proc_exec(Proc_t *proc, pid_t pgrp, int read_fd, int write_fd);
void pipeline_free(Pipeline_t *pipeline);

#endif
