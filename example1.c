#include <stdlib.h>
#include <readline/readline.h>

int main(void)
{
    char *input;

    for (;;) {
        input = readline("Myshell$ ");
        if (input == 0)
            break;
        printf("%s\n", input);
        if (strcmp(input, "exit") == 0)
            break;
        free(input);
    }
}