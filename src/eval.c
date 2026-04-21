#include "eval.h"

#include <stdio.h>

#include <unistd.h>
#include <sys/wait.h>

static void run_pipeline(ASTNode_t *root, ASTNode_t *parent)
{
    if (root->type != PIPELINE) {
        fprintf(stderr, "run_pipeline() called for root->type = %d\n", root->type);
        exit(EXIT_FAILURE);
    }

    if (root->left->type == PIPELINE) {
        run_pipeline(root->left, root);
    }

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
            perror("dup2");
            exit(EXIT_FAILURE);
        }

        close(pipefd[0]);
        close(pipefd[1]);

        ASTNode_t *writer = root->left;

        execvp(writer->argv->v[0], writer->argv->v);

        perror("execvp");
        exit(EXIT_FAILURE);
    }

    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        if (dup2(pipefd[0], STDIN_FILENO) == -1) {
            perror("dup2");
            exit(EXIT_FAILURE);
        }

        close(pipefd[0]);
        close(pipefd[1]);

        ASTNode_t *reader = root->right;

        execvp(reader->argv->v[0], reader->argv->v);

        perror("execvp");
        exit(EXIT_FAILURE);
    }


    close(pipefd[0]);
    close(pipefd[1]);

    int status;
    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid");
    }
}

static void run_simple_command(ASTNode_t *cmd)
{
    if (cmd->type != SIMPLE_COMMAND) {
        fprintf(stderr, "run_simple_command() called for cmd->type = %d\n", cmd->type);
        abort();
    }

    pid_t pid = fork();
    pid_t w;
    int wstatus;
    switch (pid) {
    case -1:
        perror("fork");
        break;

    case 0:
        if (execvp(cmd->argv->v[0], cmd->argv->v) == -1) {
            perror("execvp");
            exit(EXIT_FAILURE);
        }

        break;

    default:
        w = waitpid(pid, &wstatus, 0);
    }
}

void eval(ASTNode_t *root)
{
    if (!root) {
        return;
    }

    if (root->type == PIPELINE) {
        run_pipeline(root, NULL);
        return;
    }

    if (root->left) {
        eval(root->left);
    }

    if (root->right) {
        eval(root->right);
    }

    switch (root->type) {
    /* A COMMAND_LIST node will always be above a SIMPLE_COMMAND node.
     * Since all of it's children have already been executed, there's
     * nothing to do here. */
    case PIPELINE:
    case COMMAND_LIST:
        break;

    default:
        run_simple_command(root);
    }
}
