#include "systemcalls.h"
#include <stdlib.h>   //Adding this header for calling system()
#include <unistd.h>   //Adding this header for execv and fork
#include <stdbool.h>  //Adding this header for bool data type
#include <wait.h>     //Adding this header for waitpid
#include <fcntl.h>    //Adding this header ofr open
/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
*/
bool do_system(const char *cmd)
{

/*  Call the system() function with the command set in the cmd
 *   and return a boolean true if the system() call completed with success
 *   or false() if it returned a failure
*/
    int returnCode = system(cmd);  //Calling the system() with cmd as input argument
    return (returnCode == 0);	    // Returns exit status 0 if system executed succiessfully
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
    command[count] = command[count];

/*
 * TODO:
 *   Execute a system command by calling fork, execv(),
 *   and wait instead of system (see LSP page 161).
 *   Use the command[0] as the full path to the command to execute
 *   (first argument to execv), and use the remaining arguments
 *   as second argument to the execv() command.
 *
*/

    va_end(args);

    //Fork a new process
    pid_t pid = fork();
    if(pid == -1){
	return false;
    }else if(pid == 0){
	//Child process using execv()
	execv(command[0],command);
	//Exit if execv() fails
	exit(EXIT_FAILURE);
    }else{
	//Parent process
	int statusProcess;
	if(waitpid(pid, &statusProcess,0) == -1){
	    //Resturning false on pid fail
            return false;
	}
	return WIFEXITED(statusProcess) && WEXITSTATUS(statusProcess) == 0;
    }
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


/*
 * TODO
 *   Call execv, but first using https://stackoverflow.com/a/13784315/1446624 as a refernce,
 *   redirect standard out to a file specified by outputfile.
 *   The rest of the behaviour is same as do_exec()
 *
*/

    va_end(args);
    //Fork a new process
    pid_t pid = fork();
    if(pid == -1){
	return false;
    }else if(pid == 0){
	//Processing for the child process
	//Redirect the output to the output file
	int fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	//Checking if file open failed
	if(fd == -1){
            exit(EXIT_FAILURE);
	}
	if(dup2(fd, STDOUT_FILENO) == -1){
	    close(fd);
	    exit(EXIT_FAILURE);
	}
	close(fd);

	//execv() command
	execv(command[0],command);
	//Exit if execv() fails
	exit(EXIT_FAILURE);
    }else{
	int pStatus;
	if(waitpid(pid, &pStatus,0) == -1){
	    //pid failed
            return false;
	}
	return WIFEXITED(pStatus) && WEXITSTATUS(pStatus) == 0;
    }
}
