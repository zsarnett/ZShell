#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "shell.h"
#include <fcntl.h>
#include <signal.h>

void Execute(char** args);
int CommandHandle(char **args);

#define Max_Command_Length 100

//char BckGrnd[Max_Command_Length]; Started Background Process

int main(int argc, char** argv)
{
	char Command[Max_Command_Length+1];
	
	FILE *fp;
	
	//If an Argument is Passed
	if(argc == 2)
        {
		//Open file that was passed through
 	        fp = fopen(argv[1], "rt");
		if(!fp)
		{
			//Error Handling
			perror("Error opening file");
			_exit(1);
		}
        }
	while(1)
	{
		if(argc == 1) //Read from stdin
		{
			//Print Prompt for User
			fprintf(stderr, "shell> ");
                	fflush(stderr);
			fgets(Command, sizeof(Command), stdin);	
		}
		else //Read from File
		{
			if(!fgets(Command, sizeof(Command), fp))
			{
				//Break at the EOF
				break;	
			}
		}

		//Parse Command using the given function
		struct command *cmd = parse_command(Command);
		
		//Case for just new line
		if(cmd->args[0] == NULL) 
		{
			continue; 
		}
		
		//Case for exit status
		if(strcmp(cmd->args[0],"exit") == 0) 
		{ 
			free_command(cmd);
			_exit(0);
		}

		//Call Command handler to check if the built in functions were called
		if(CommandHandle(cmd->args) != 0)
                {
			continue;
                }
	
		int fd;

		//Fork
	        pid_t pid = fork();

        	//Error when Forking
        	if(pid < 0)
	        {
        	        //Error handling
	                perror("Failed to Fork");
			free_command(cmd);
	                _exit(1);
	        }
	        else if(pid == 0) //CHILD Process
	        {
	                if(cmd->in_redir) //If there is an "<" called
        	        {
				//Open the file called
	                        if((fd = open(cmd->in_redir, O_RDONLY)) < 0)
				{
					//Error Handling
					perror("Error redirecting");
					free_command(cmd);
					_exit(3);
				}
				//Duplicate it to stdin
        	                if(dup2(fd, 0) == -1)
				{
					//Error Handling
					perror("Error While Duplicating");
					free_command(cmd);
					_exit(100);
				}
                	        close(fd);
	                }
			if(cmd->out_redir) //If > was called
			{
				//Open or create the file
				if ((fd = open(cmd->out_redir, O_WRONLY | O_TRUNC  | O_CREAT, 0666)) < 0)
				{
					//Error Handling
					perror("Error redirecting");
					free_command(cmd);
                                        _exit(4);				
				}
				//Duplicate to STDout
				if(dup2(fd, 1) == -1)
				{
					//Error Handling
					perror("Error While Duplicating");
                                        free_command(cmd);
                                        _exit(101);
				}
                                close(fd);
			}
			//Call Execvp to run commands
        	        int result = execvp(cmd->args[0],cmd->args);
	                if(result == -1)
        	        {
                	        //Error Handling
                        	perror("Failed to Execute");
				free_command(cmd);
	                        _exit(5);
        	        }
	        }
	        else //PARENT Process
	        {
        	        int status;

			/*if(strcmp(cmd->args[sizeof(cmd->args)/sizeof(cmd->args[0])], "&") == 0)
			{
				continue;
			}*/

			//Ignore Signals		
			signal(SIGINT, SIG_IGN);

			//Wait for the CHILD Process
			if(waitpid(pid, &status, 0) == -1)
        	        {
                	        //Error Handling
                        	perror("Wait Failed");
	                        _exit(6);
        	        }
			
			//Set the signal back to default
			signal(SIGINT, SIG_DFL);
			
			if (WIFEXITED(status)) //If the Wait exited
			{
				if(WEXITSTATUS(status) > 0) //if the exit status was non zero
				{
 					fprintf(stderr,"Command returned %d", WEXITSTATUS(status));
				}				
			}
			if (WIFSIGNALED(status)) //If a signal was called
			{
				fprintf(stderr, "Command Killed : %s", strsignal(WTERMSIG(status)));
			}
	        }
		
		//Free the memory from Command
		free_command(cmd);
	}
	return EXIT_SUCCESS;
}

int Change_Directory(char **args)
{
	//If no directory was specified
	if(args[1] == NULL)
	{
		//Get the home enviroment and change the directory to it
		int CDError = chdir(getenv("HOME"));
		if(CDError == -1)
		{
			//Error Handling
			perror("Error going HOME");
			return -1;
		}
	}
	else if(chdir(args[1]) == -1) //Change to given Directory
	{
		//Error Handling
		perror("No Directory Found");
		return -1;
	}
	return -1;
}

int Set_Enviroment(char **args)
{
	//If 2 arguements are called
	if(args[1] != NULL && args[2] != NULL)
	{
		//Use the two arguements to call setenv
        	int setenvir = setenv(args[1], args[2], 1);
                if(setenvir == -1)
                {
                	//Error Handling
                        perror("Failed Setting Enviroment");
                        return -1;
               	}
		return 1;
	}
	//If only one arguement was called
        else if(args[1] != NULL && args[2] == NULL)
        {
		//Unset the given arguement
        	int unsetenvir = unsetenv(args[1]);
                if(unsetenvir == -1)
                {
                        //Error Handling
                        perror("Failed Setting Enviroment");
                        return -1;
                }
		return 1;
	}
	//If no arguements were called
	else if(args[1] == NULL && args[2] == NULL)
        {
		//Not enough arguements to do anything
        	perror("Not enough Arguements");
		return -1;
        }
	return 1;
}

int CommandHandle(char **args) //Execute External Commands
{	
	//Checking if "cd" was called
	if(strcmp(args[0],"cd") == 0)
	{
		//Call Change_Directory Function
		return Change_Directory(args);
	}
	//Checking if "setenv" was called
	else if(strcmp(args[0],"setenv") == 0)
	{
		return Set_Enviroment(args);
	}
	return 0;
}
