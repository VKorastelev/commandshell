#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <locale.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include "gettext.h"

#define COMMAND_BUF_SIZE 512
#define MAX_NUM_ARGS 100

int parsing_command(
        char *str,
        char **const args,
        size_t num_args);

int exec_prog(
        char **args,
        bool *child);

int main()
{
    int ret = 0;
    time_t curr_time;
    bool child = false;

    char command_buf[COMMAND_BUF_SIZE];
    char *args[MAX_NUM_ARGS + 2] = {NULL};

    curr_time = time(NULL);
    printf("commandshell 0.1 %s\n", ctime(&curr_time));
    puts("To exit the shell, enter \"exit\", \"Exit\" ...");

    for(;;)
    {
        printf("command> ");
        ret = get_text(
                command_buf,
                sizeof(command_buf));
        if (EOF == ret )
        {
            break;
        }
        else if (EXIT_FAILURE == ret)
        {
            puts("Input error!");
        }
        else if (0 == ret)
        {
            parsing_command(
                    command_buf,
                    args,
                    sizeof(args) / sizeof(args[0]));

            if (0 == strncasecmp(args[0], "exit", 5))
            {
                puts("Exiting the commandshell");
                break;
            }

            ret = exec_prog(args, &child);
            if (true == child || 0 != ret)
            {
                break;
            }
        }
    }

    return ret;
}


int parsing_command(
        char *const str,
        char **const args,
        size_t const num_args)
{
    int ret = 0;
    int i = 0;
    char *pstr = NULL;

    pstr = strtok(str, " ");
    if (NULL == pstr)
    {
        fprintf(stderr, "Error! The command line is empty or has no subdelim: %s\n\n",
                str);
        ret = EXIT_FAILURE;
        goto finally;
    }

    args[i] = pstr;
    i++;

    while (NULL != (pstr = strtok(NULL, " ")) && i < (num_args - 1))
    {
        args[i] = pstr;
        i++;
    }

    if (NULL != pstr && (num_args - 1) == i)
    {
        fprintf(stderr, "Too much arguments! The number of arguments must not exceed"
                " %ld\n", num_args - 2);
    }

    args[i] = NULL;

 finally:

    return ret;
}


int exec_prog(
        char **args,
        bool *const child)
{
    int ret = 0;

    pid_t pid;
    int wstatus = 0;

    errno = 0;
    switch (pid = fork())
    {
        case -1: // Error in fork()
        {
            *child = false;

            perror("Error in fork()");
            ret = -1;
            break;
        }
        case 0: // Child process
        {
            *child = true;
            
            errno = 0;
            if (-1 == execvp(args[0], args) && 0 != errno)
            {
                ret = errno;
                perror("Error in command");
            }
            else
            {
                ret = -1;
            }
            break;
        }
        default: // Parent process
        {
            *child = false;

            if (-1 == waitpid(pid, &wstatus, 0))
            {
                perror("Error in wait(...)");
                ret = -1;
                goto finally;
            }

            if (WIFEXITED(wstatus))
            {
                printf("Shell returned status: %d\n", WEXITSTATUS(wstatus));
            }
            else
            {
                printf("Shell aborted/interrupded with status: %d\n", wstatus);
            }

            break;
        }
    }

 finally:
    
    return ret;
}
