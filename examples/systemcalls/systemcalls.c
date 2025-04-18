#include "systemcalls.h"
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <syslog.h>
#include <stdio.h>
#include <fcntl.h>

/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
*/
bool do_system(const char *cmd)
{

    int flag = system(cmd);

    if (flag < 0)
        return false;
    
    if (flag == 127)
        return false;
    
    if (flag == 0)
        return true;
    else
        return false;

/*
 * TODO  add your code here
 *  Call the system() function with the command set in the cmd
 *   and return a boolean true if the system() call completed with success
 *   or false() if it returned a failure
*/
}

/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*/

bool do_exec(int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    //command[count] = command[count];

    pid_t pid = fork();

    if (pid > 0) //parent
    {
        int status;
        pid_t flag = wait(&status);
        //printf("status = %d, flag = %d\n", status, flag);
        if (status != 0 || flag == -1)
        {
            syslog(LOG_ERR, "fail\n");
            return false;
        }
        return true;
    }
    else if (pid == 0) //child
    {
        execv(command[0], &command[0]);
        
        printf("fail to execv %s, errno = %d\n", command[0], errno);
        syslog(LOG_ERR, "fail to execv %s\n", command[0]);
        exit(-1);
    }
    else //error
    {
         syslog(LOG_ERR, "fail to do_exec\n");
         return false;
    }

    va_end(args);

    return true;
}

/**
* @param outputfile - The full path to the file to write with command output.
*   This file will be closed at completion of the function call.
* All other parameters, see do_exec above
*/
bool do_exec_redirect(const char *outputfile, int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];
    pid_t pid = fork();

    if (pid > 0) //parent
    {
        int status;
        pid_t flag = wait(&status);
        //printf("status = %d, flag = %d\n", status, flag);
        if (status != 0 || flag == -1)
        {
            syslog(LOG_ERR, "fail\n");
            return false;
        }
        return true;
    }
    else if (pid == 0) //child
    {
        int fd = open(outputfile, O_WRONLY);

        if (fd == -1)
            exit(-1);

        dup2(fd, STDOUT_FILENO);

        execv(command[0], &command[0]);
        
        //printf("fail to execv %s, errno = %d\n", command[0], errno);

        syslog(LOG_ERR, "fail to execv %s\n", command[0]);
        closelog();
        exit(-1);
    }
    else //error
    {
        syslog(LOG_ERR, "fail to do_exec\n");
        closelog();
        return false;
    }

/*
 * TODO
 *   Call execv, but first using https://stackoverflow.com/a/13784315/1446624 as a refernce,
 *   redirect standard out to a file specified by outputfile.
 *   The rest of the behaviour is same as do_exec()
 *
*/

    va_end(args);

    return true;
}
