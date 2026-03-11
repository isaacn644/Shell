// minishell.c
// Compile: gcc -Wall -Wextra -std=c11 minishell.c -o minishell
// Run:     ./minishell

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#define MAX_LINE 1024
#define MAX_TOKENS 128
#define HISTORY_MAX 100

// ---------------------- History ----------------------
static char *history[HISTORY_MAX];
static int history_count = 0;

static void add_history(const char *line) {
    if (history_count >= HISTORY_MAX) return;
    history[history_count++] = strdup(line);
}

static void print_history(void) {
    for (int i = 0; i < history_count; i++) {
        printf("%d %s\n", i + 1, history[i]);
    }
}

static const char* get_history_item(int n) {
    if (n < 1 || n > history_count) return NULL;
    return history[n - 1];
}

// ---------------------- SIGCHLD handler ----------------------
static void sigchld_handler(int sig) {
    (void)sig;
    int saved_errno = errno;

    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;

    errno = saved_errno;
}

// ---------------------- Helpers ----------------------
static void trim_newline(char *s) {
    if (!s) return;
    size_t len = strlen(s);
    if (len > 0 && s[len - 1] == '\n')
        s[len - 1] = '\0';
}

static int tokenize(char *line, char *tokens[], int max_tokens) {
    int count = 0;
    char *tok = strtok(line, " \t");
    while (tok && count < max_tokens - 1) {
        tokens[count++] = tok;
        tok = strtok(NULL, " \t");
    }
    tokens[count] = NULL;
    return count;
}

static void remove_two_tokens(char *tokens[], int *ntokens, int i) {
    for (int j = i; j + 2 < *ntokens; j++) {
        tokens[j] = tokens[j + 2];
    }
    *ntokens -= 2;
    tokens[*ntokens] = NULL;
}

static int handle_redirection(char *tokens[], int *ntokens) {
    for (int i = 0; i < *ntokens; ) {
        if (strcmp(tokens[i], "<") == 0 || strcmp(tokens[i], ">") == 0) {
            if (i + 1 >= *ntokens) {
                fprintf(stderr, "Redirection error: missing filename\n");
                return -1;
            }

            if (strcmp(tokens[i], "<") == 0) {
                if (!freopen(tokens[i + 1], "r", stdin)) {
                    perror("freopen");
                    return -1;
                }
            } else {
                if (!freopen(tokens[i + 1], "w", stdout)) {
                    perror("freopen");
                    return -1;
                }
            }

            remove_two_tokens(tokens, ntokens, i);
            continue;
        }
        i++;
    }
    return 0;
}

static int is_background(char *tokens[], int *ntokens) {
    if (*ntokens > 0 && strcmp(tokens[*ntokens - 1], "&") == 0) {
        tokens[--(*ntokens)] = NULL;
        return 1;
    }
    return 0;
}

static void print_prompt(void) {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)))
        printf("%s> ", cwd);
    else
        printf("> ");
    fflush(stdout);
}

// ---------------------- Execution ----------------------
static void run_external(char *tokens[], int ntokens, int background) {
    pid_t pid = fork();

    if (pid == 0) {
        if (handle_redirection(tokens, &ntokens) != 0)
            exit(1);

        execvp(tokens[0], tokens);
        perror("execvp");
        exit(1);
    } 
    else if (pid > 0) {
        if (!background)
            waitpid(pid, NULL, 0);
    } 
    else {
        perror("fork");
    }
}

// ---------------------- Main ----------------------
int main(void) {
    struct sigaction sa = {0};
    sa.sa_handler = sigchld_handler;
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, NULL);

    char line[MAX_LINE];

    while (1) {
        print_prompt();

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }

        trim_newline(line);
        if (*line == '\0') continue;

        /* -------- FIX: handle !N BEFORE adding to history -------- */
        if (line[0] == '!' && line[1]) {
            int n = atoi(line + 1);
            const char *cmd = get_history_item(n);
            if (!cmd) {
                fprintf(stderr, "No such history item\n");
                continue;
            }
            strncpy(line, cmd, sizeof(line) - 1);
            line[sizeof(line) - 1] = '\0';
        }

        add_history(line);

        char line_copy[MAX_LINE];
        strncpy(line_copy, line, sizeof(line_copy));
        char *tokens[MAX_TOKENS];
        int ntokens = tokenize(line_copy, tokens, MAX_TOKENS);

        if (ntokens == 0) continue;

        if (strcmp(tokens[0], "exit") == 0)
            break;

        if (strcmp(tokens[0], "history") == 0) {
            print_history();
            continue;
        }

        if (strcmp(tokens[0], "cd") == 0) {
            if (ntokens < 2)
                fprintf(stderr, "cd: missing operand\n");
            else if (chdir(tokens[1]) != 0)
                perror("cd");
            continue;
        }

        int background = is_background(tokens, &ntokens);
        run_external(tokens, ntokens, background);
    }

    for (int i = 0; i < history_count; i++)
        free(history[i]);

    return 0;
}
