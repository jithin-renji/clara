#include "jobs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <sys/wait.h>

Job_t *jobs = NULL;
static size_t next_job_id = 0;

static void job_add(Job_t *job)
{
    if (!jobs) {
        jobs = job;
    } else {
        /* Add job to the beginning of list */
        job->next = jobs;
        jobs = job;
    }
}

void job_create(Pipeline_t *pipeline, int is_foreground)
{
    int read_fd = STDIN_FILENO;
    int write_fd = STDOUT_FILENO;
    pid_t pgrp = 0;
    for (Proc_t *proc = pipeline; proc; proc = proc->next) {
        int pfd[2];
        if (proc->next) {
            if (pipe(pfd) == -1) {
                perror("pipe");
                break;
            }

            write_fd = pfd[1];
        } else {
            /* Reset write_fd since it may have been overwritten by
             * previous processes. */
            write_fd = STDOUT_FILENO;
        }

        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            break;
        }

        switch (pid) {
        case 0:
            proc_exec(proc, pgrp, read_fd, write_fd);
            break;

        default:
            /* The first process should be the session leader. */
            if (pgrp == 0) {
                pgrp = pid;
            }

            setpgid(pid, pgrp);
        }

        if (read_fd != STDIN_FILENO) {
            close(read_fd);
        }

        if (write_fd != STDOUT_FILENO) {
            close(write_fd);
        }

        /* The next process in the pipeline should read from *this* pipe. */
        read_fd = pfd[0];
    }


    int wstatus;
    if (waitpid(-pgrp, &wstatus, WUNTRACED) == -1 && errno != ECHILD) {
        perror("waitpid");
    }

    if (WIFSTOPPED(wstatus)) {
        Job_t *job = malloc(sizeof(Job_t));
        job->id = next_job_id;
        job->cmdline = strdup(pipeline->argv->v[0]);
        job->pipeline = pipeline;
        job->pgrp = pgrp;
        job->is_foreground = 0;
        job->is_running = 0;
        job->next = NULL;

        job_add(job);
        fprintf(stderr, "\n[%ld] Stopped.\n"
                "Run 'bg' to send this job to the background or 'fg' to bring it back to the foreground.\n", job->id);

        next_job_id++;
    }

    tcsetpgrp(STDIN_FILENO, getpgrp());
}

void jobs_free(void)
{
    Job_t *cur = jobs;
    while (cur) {
        free(cur->cmdline);
        pipeline_free(cur->pipeline);

        Job_t *next = cur->next;
        free(cur);

        cur = next;
    }
}
