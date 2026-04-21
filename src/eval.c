#include "eval.h"

#include <stdio.h>

#include <unistd.h>
#include <sys/wait.h>

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
    case COMMAND_LIST:
        break;

    default:
        run_simple_command(root);
    }
}
