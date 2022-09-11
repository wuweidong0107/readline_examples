#include <stdlib.h>
#include <readline/readline.h>
#include <termios.h>
#include <unistd.h>
#include <wordexp.h>
#include <errno.h>
#include <stdbool.h>

typedef int (*cmd_fn_t)(int argc, char *argv[]);
typedef struct {
    char *name;
    cmd_fn_t func;
    char *doc;
} command_t;

static struct termios term;
static tcflag_t old_lflag;
static cc_t old_vtime;

static int cmd_help(int argc, char *argv[]);
static int cmd_quit(int argc, char *argv[]);

static command_t commands[] = {
    { "help", cmd_help, "Disply help info" },
    { "quit", cmd_quit, "Quit this menu" }, 
    { NULL, NULL, NULL},
};

static int cmd_help(int argc, char *argv[])
{
    int i=0;
    printf("Avaliable command:\n");
    for (; commands[i].name; i++) {
        printf("%s: %s\n", commands[i].name, commands[i].doc);
    }
}

static int cmd_quit(int argc, char *argv[])
{
    term.c_cflag = old_lflag;
    term.c_cc[VTIME] = old_vtime;

    if (tcsetattr(STDIN_FILENO, TCSANOW, &term) < 0) {
        perror("tcsetattr");
        exit(1);
    }
    exit(0);
}

static command_t *find_command(const char *name)
{
    int i;
    for (i=0; commands[i].name; i++) {
        if(strcmp(name, commands[i].name) == 0)
            return (&commands[i]);
    }
    for (i=0; commands[i].name; i++) {
        if(strcmp("help", commands[i].name) == 0)
            return (&commands[i]);
    }
    return NULL;
}

static int shell_exec(int argc, char *argv[])
{
    int i=0;
    command_t *cmd = find_command(argv[0]);
    if (cmd) {
        return cmd->func(argc, argv);
    }
    return -ENOENT;
}

static void process_line(char *line)
{
    if (line == NULL) {
        fprintf(stderr, "line is NULL\n");
        term.c_cflag = old_lflag;
        term.c_cc[VTIME] = old_vtime;
        if (tcsetattr(STDIN_FILENO, TCSANOW, &term) < 0) {
            perror("tcsetattr");
            exit(1);
        }
        exit(0);
    }

    wordexp_t w;
	if (wordexp(line, &w, WRDE_NOCMD))
		return;

	if (w.we_wordc == 0) {
		wordfree(&w);
		return;
	}
    shell_exec(w.we_wordc, w.we_wordv);
    wordfree(&w);
    free(line);
}

int main(void)
{
    if (tcgetattr(STDIN_FILENO, &term) < 0) {
        perror("tcgetattr");
        exit(1);
    }
    old_lflag = term.c_cflag;
    old_vtime = term.c_cc[VTIME];
    term.c_cflag &= ~ICANON;
    term.c_cc[VTIME] = 1;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &term) < 0) {
        perror("tcsetattr");
        exit(1);
    }

    rl_callback_handler_install("Myshell$ ", process_line);

    fd_set fds;
    while(1) {
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
            
        if (select(STDIN_FILENO+1, &fds, NULL, NULL, NULL) < 0) {
            perror("select");
            exit(1);
        }

        if (FD_ISSET(STDIN_FILENO, &fds)) {
            rl_callback_read_char();
        }
    }
}