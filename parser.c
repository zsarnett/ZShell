#include "shell.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void free_command(struct command *cmd)
{
	int i;
	for (i = 0; i < SHELL_MAX_ARGS; i++) {
		// NULL pointer marks the end of arguments.
		if (cmd->args[i] == NULL)
			break;
		free(cmd->args[i]);
	}
	if (cmd->out_redir)
		free(cmd->out_redir);
	if (cmd->in_redir)
		free(cmd->in_redir);
	free(cmd);
}

/* Version of strdup that terminates on allocation failure. */
static char *Strdup(const char *s)
{
	char *result = strdup(s);
	if (result == NULL) {
		fprintf(stderr, "strdup failed to allocate memory\n");
		exit(127);
	}
	return result;
}

/* Allocate an empty command, terminating on allocation failure. */
static struct command *alloc_command(void)
{
	/* Use calloc so it is filled with 0s (null pointers).  The cast
	 * from calloc's "void *" return is needed in C++ but not C. */
	void *result = calloc(1, sizeof(struct command));
	if (result == NULL) {
		fprintf(stderr, "alloc_command failed to allocate memory\n");
		exit(127);
	}
	return (struct command *) result;
}

#define WHITESPACE " \t\n"
struct command *parse_command(const char *cmdline)
{
	if (cmdline == NULL)
	{
		fprintf(stderr, "Cannot parse null cmdline.\n");
		exit(127);
	}

	struct command *result = alloc_command();

	/* Index into result->args of next argument. */
	int argno = 0;

	/* Make a writable copy of the command line for tokenization */
	char *cmdbuf = Strdup(cmdline);
	/* Get first whitespace-delimited token. */
	char *token = strtok(cmdbuf, WHITESPACE);
	while (token != NULL) {
		/* Parse redirections */
		if (token[0] == '>') {
			if (result->out_redir) {
				fprintf(stderr, "Multiple redirection.\n");
				exit(127);
			} else if (token[1] != '\0') {
				/* >file (one word) */
				result->out_redir = Strdup(token + 1);
			} else if ((token = strtok(NULL, WHITESPACE)) != NULL) {
				/* > file: get next argument */
				result->out_redir = Strdup(token);
			} else {
				/* > (end of line): error */
				fprintf(stderr, "Redirection syntax error.\n");
				exit(127);
			}
		} else if (token[0] == '<') {
			if (result->in_redir) {
				fprintf(stderr, "Multiple redirection.\n");
				exit(127);
			} else if (token[1] != '\0') {
				/* <file (one word) */
				result->in_redir = Strdup(token + 1);
			} else if ((token = strtok(NULL, WHITESPACE)) != NULL) {
				/* < file: get next argument */
				result->in_redir = Strdup(token);
			} else {
				/* < (end of line): error */
				fprintf(stderr, "Redirection syntax error.\n");
				exit(127);
			}
		} else {
			/* Not a redirection, must be an argument. Make sure
			 * there is room in the args array for both this arg
			 * and a sentinel NULL pointer. */
			if (argno >= SHELL_MAX_ARGS) {
				fprintf(stderr, "Too many arguments.\n");
				exit(127);
			}
			result->args[argno++] = Strdup(token);
		}
		/* Get next token */
		token = strtok(NULL, WHITESPACE);
	}
	/* Don't need to manually do result->args[argno] = NULL, because
	 * calloc took care of that for us. */
	free(cmdbuf);
	return result;
}


