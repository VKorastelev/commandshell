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


#define COMMAND_BUF_SIZE 256
#define MAX_NUM_ARGS 5
#define MAX_SIZE_ARGS 6

int parsing_command(
        char *str,
        char **const args,
        size_t num_args);

int get_command(
        char *str,
        size_t str_size);

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
        printf("\ncommand> ");
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

            if (0 == strncasecmp(args[0], "exit", 4))
            {
                puts("Exiting the commandshell.");
                break;
            }

            ret = exec_prog(args, &child);
            printf("ret = %d\n", ret);
            if (true == child)
            {
                break;
            }
            else if (0 != ret)
            {
                break;
            }
        }
    }

    return ret;
}


int get_command(
        char *const str,
        size_t const str_size)
{
    int ret = 0;

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
    printf("args[%d] = %p\n", i, args[i]);

    i++;

    while (NULL != (pstr = strtok(NULL, " ")) && i < (num_args - 1))
    {
        args[i] = pstr;
        printf("args[%d] = %p\n", i, args[i]);
        i++;
    }

    if (NULL != pstr && (num_args - 1) == i)
    {
        fprintf(stderr, "Too much arguments! The number of arguments must not exceed"
                " %ld\n", num_args - 2);
    }

    args[i] = NULL;
    printf("args[%d] = %p\n", i, args[i]);

 finally:

    return ret;
}


int exec_prog(
        char **args,
        bool *const child)
{
    int ret = 0;
    int errsv = 0;

    pid_t pid, ret_pid;
    int wstatus = 0;

    errno = 0;
    switch (pid = fork())
    {
        case -1: // Error in fork()
        {
            *child = false;
            errsv = errno;
            perror("Error in fork()");
            printf("errno = %d\n", errsv);
            ret = EXIT_FAILURE;
            goto finally;
        }
        case 0: // Child process
        {
            *child = true;
            printf("Child %d, parent pid = %d\n", getpid(), getppid());
            
            errno = 0;
            if (-1 == execvp(args[0], args))
            {
                ret = errno;
                perror("Error in command");
            }
            break;
        }
        default: // Parent process
        {
            *child = false;
            printf("Parent %d, child pid = %d\n", getpid(), pid);
            ret_pid = waitpid(pid, &wstatus, 0);

            printf("ret_pid = %d\n", ret_pid);

            if (-1 == ret_pid)
            {
                errsv = errno;
                perror("Error in wait(...)");
                printf("errno = %d\n", errsv);
                ret = EXIT_FAILURE;
                goto finally;
            }
            
            if (WIFEXITED(wstatus))
            {
                printf("Child pid = %d exited, WEXITSTATUS(wstatus) = %d\n",
                        pid,
                        WEXITSTATUS(wstatus));
            }
            else
            {
                printf("Child pid = %d exited, not WEXITSTATUS\n", pid);
            }

            break;
        }
    }

 finally:
    
    return ret;
}
