Author:
	Zack Arnett | arnett.zackary@gmail.com 

Contents:
	zshell.c:
		This is where I implemented my shell code including my main, Change_Directory, CommandHandler, and Setenv functions.

	Makefile:
		This will compile and build an executable file named shell

	README.txt:
		This File.

	zshell.h
		This is the provided header file for the shell.c and the parser.c

	parser.c
		This the the file that runs the parse and free command functions given by instructor

Running:
	This shell program is ran by first using the make function to make an execuable function to run, then run ./shell. This will prompt the user for input and then the user can input any function that they want to use. You can also run ./shell input_file this will read from the input_file and print what the output should be. It will not print the prompt or ask the user for an input.

Implementation Notes:
	In my main function, It checks for Empty/Whitespace, then exit, then goes into a function that checks to see if CD or SETENV were called if they weren't then it continues to the EXECVP, but if it does catch CD or SETENV then it calls the function that it corresponds to. SET_ENV is the function for setting the enviroment and CHANGE_DIRECTORY is the function for changing the enviroment

Limitations:
	I limited the CHAR LENGTH of the command to 100. I had some issues with the Command-Exit test case but with an email to Neil Moore he explained what it was wanting and then I implemented the WIFEXITED signal code. 

