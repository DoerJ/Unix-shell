// Shell starter file
// You may make any changes to any part of this file.

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#define COMMAND_LENGTH 1024
#define NUM_TOKENS (COMMAND_LENGTH / 2 + 1)

int history_count = 0;
char *command_history[100];

/**
 * Command Input and Processing
 */

/*
 * Tokenize the string in 'buff' into 'tokens'.
 * buff: Character array containing string to tokenize.
 *       Will be modified: all whitespace replaced with '\0'
 * tokens: array of pointers of size at least COMMAND_LENGTH/2 + 1.
 *       Will be modified so tokens[i] points to the i'th token
 *       in the string buff. All returned tokens will be non-empty.
 *       NOTE: pointers in tokens[] will all point into buff!
 *       Ends with a null pointer.
 * returns: number of tokens.
 */
int tokenize_command(char *buff, char *tokens[])
{
	int token_count = 0;
	_Bool in_token = false;
	int num_chars = strnlen(buff, COMMAND_LENGTH);
	for (int i = 0; i < num_chars; i++) {
		switch (buff[i]) {
		// Handle token delimiters (ends):
		case ' ':
		case '\t':
		case '\n':
			buff[i] = '\0';
			in_token = false;
			break;

		// Handle other characters (may be start)
		default:
			if (!in_token) {
				tokens[token_count] = &buff[i];
				token_count++;
				in_token = true;
			}
		}
	}
	tokens[token_count] = NULL;
	return token_count;
}

/**
 * Read a command from the keyboard into the buffer 'buff' and tokenize it
 * such that 'tokens[i]' points into 'buff' to the i'th token in the command.
 * buff: Buffer allocated by the calling code. Must be at least
 *       COMMAND_LENGTH bytes long.
 * tokens[]: Array of character pointers which point into 'buff'. Must be at
 *       least NUM_TOKENS long. Will strip out up to one final '&' token.
 *       tokens will be NULL terminated (a NULL pointer indicates end of tokens).
 * in_background: pointer to a boolean variable. Set to true if user entered
 *       an & as their last token; otherwise set to false.
 */
void read_command(char *buff, char *tokens[], _Bool *in_background, _Bool *remove)
{
	*in_background = false;
 
	// Read input
	// return the number of bytes that were read
	// scanf
	int length = read(STDIN_FILENO, buff, COMMAND_LENGTH-1);

	if ((length < 0) && (errno != EINTR)) {
		perror("Unable to read command from keyboard. Terminating.\n");
		exit(-1);
	}
	// Null terminate and strip \n.
	// '\0' stands for null-terminated string
	// add null pointer at the end of string
	buff[length] = '\0';
	if (buff[strlen(buff) - 1] == '\n') {
		buff[strlen(buff) - 1] = '\0';
	}

	// Tokenize (saving original command string)
	// return the number of seperated words 
	int token_count = tokenize_command(buff, tokens);
	// if no word is contained in the buffer, that means user didn't enter any command before return
	if (token_count == 0) {
		return;
	}

	// Extract if running in background:
	// strcmp(): check if chars are identical
	// note: char c = 0 is equivalent to char c = '\0'
	if (token_count > 0 && strcmp(tokens[token_count - 1], "&") == 0) {
		*in_background = true;
		// remove & from token buffer
		// so ls & can be executed by child process
		tokens[token_count - 1] = 0;
		*remove = true;
	}
}
// return a command number that is to be executed from history list
int command_list(char *command, int history_count) {

	int i = 0;
	int j = 0;
	int command_num = 0;
	char *num_temp = (char *)malloc(sizeof(command));
	for(; i < strlen(command) - 1; i++) {
		// ---------------------------------------------- TO BE CHECK! -----------------------------
		// bypass the first char of !command (i.e., '!')
			num_temp[j] = command[i + 1];
			j ++;
	}
	// check if !! command
	if(num_temp[0] == '!') {
		// command_num == history - 1
		command_num = history_count - 1;
		// if there is no previous command
		if(command_num == 0) {
			write(STDOUT_FILENO, "There is no command available!", strlen("There is no command available!"));
			write(STDOUT_FILENO, "\n", strlen("\n"));
		}
	}
	// else not !! command (!num or !invalid)
	else {
		command_num = atoi(num_temp);
		if(command_num == 0 && num_temp[0] != 0) {
			write(STDOUT_FILENO, "Your input is invalid! Please try again!", strlen("Your input is invalid! Please try again!"));
			write(STDOUT_FILENO, "\n", strlen("\n"));
		}
	}
	// free() num_temp
	free(num_temp);
	return command_num;
}

void display_history(int count_history, char *command_history[]) {
	int count = 1;
	char str[5];
	// if history_count is less than or equal to 10
	if(count_history <= 10) {
		while(count <= count_history) {
			// convert count to string
			sprintf(str, "%i", count);
			write(STDOUT_FILENO, str, strlen(str));
			write(STDOUT_FILENO, "\t", strlen("\t")); 
			write(STDOUT_FILENO, command_history[count - 1], strlen(command_history[count - 1]));
			write(STDOUT_FILENO, "\n", strlen("\n"));
			count ++;
		}
	}
	// if history_count > 10
	else {
		// the history count of the first command on the history list
		count = count_history - 9;
		while(count <= count_history) {
			// convert count to string
			sprintf(str, "%i", count);
			write(STDOUT_FILENO, str, strlen(str));
			write(STDOUT_FILENO, "\t", strlen("\t"));
			write(STDOUT_FILENO, command_history[count - 1], strlen(command_history[count - 1]));
			write(STDOUT_FILENO, "\n", strlen("\n"));
			count ++;
		}
	}
}
// user defined signal handler
void signal_handler(int sig) {
	if(sig == SIGINT) {
		write(STDOUT_FILENO, "\n", strlen("\n"));
		display_history(history_count, command_history);
	}
}
/**
 * Main and Execute Commands
 */
int main(int argc, char* argv[])
{
	char input_buffer[COMMAND_LENGTH];
	char *tokens[NUM_TOKENS];
	int status;

	while (true) {

		// set up signal handler
		// register signal and signal handler
		signal(SIGINT, signal_handler);

		// Get command
		// Use write because we need to use read() to work with
		// signals, and read() is incompatible with printf().
		// write() is a system call that writes the data out of string buffer
		// the first parameter passed to write() is file descriptor
		// second parameter: the buffer (char sequence) written from
		// third parameter: the size of text in buffer
		// change the prompt
	
		char prompt_buffer[100];
		char *prompt_dir;
		char *prompt;
		prompt_dir = getcwd(prompt_buffer, sizeof(prompt_buffer));
		// change prompt
		prompt = strcat(prompt_dir, "> ");
		//printf("%s", prompt);
		//fflush(stdout);
		write(STDOUT_FILENO, prompt, strlen(prompt));
		// _Bool type applies to c99 standard (but equivalent to bool type)				
		_Bool in_background = false;
		// check if & has been removed from tokens
		_Bool remove = false;

		read_command(input_buffer, tokens, &in_background, &remove);
		// check if user command is an empty line
		if(tokens[0] == '\0') {
			continue;
		}
		// check the value of errno, if it is the case crtl + c, then jump back to the beginning of loop
		if(errno == EINTR) {
			// set the value of errno back to default before running new iteration
			errno = ENOENT;
			continue;
		}

		// every time get a new command, history count increments
		history_count = history_count + 1;
		// token generated!

		// DEBUG: Dump out arguments: -------------------------------- TOKEN
		/*
		for (int i = 0; tokens[i] != NULL; i++) {
			// these are outputs to console
			write(STDOUT_FILENO, "   Token: ", strlen("   Token: "));
			write(STDOUT_FILENO, tokens[i], strlen(tokens[i]));
			write(STDOUT_FILENO, "\n", strlen("\n"));
		}
		*/
		// ------------------------------------------------------------
		if (in_background) {
			write(STDOUT_FILENO, "Run in background.", strlen("Run in background."));
		}
		// check if user command is "!" -------------------------------------------------------------
		if(tokens[0][0] == '!') {
			// get command number from command_list()
			int command_num = command_list(tokens[0], history_count);
			char *get_command;
			char *new_token;
			int i = 0;
			if(command_num != 0) {
			get_command = command_history[command_num - 1];
			// now we have command to be executed
			// we need to replace tokens with get_command
			// ensure ! command will not be printed in history list
			new_token = strtok(get_command, " ");
			while(new_token != NULL) {
				tokens[i] = new_token;
				i ++;
				new_token = strtok(NULL, " ");
			}
			}	
		}
		// ------------------------------------------------------------------------------------------
		// store the tokens into history_command
		int j = 0;
		command_history[history_count - 1] = (char *)malloc(sizeof(tokens));
		
		// if !! is the first command that has been entered (no previous command available)
		if(strcmp(tokens[0], "!!") == 0) {
			history_count = history_count - 1;
		}
		else {
			for(; tokens[j] != NULL; j++) {
				command_history[history_count - 1] = strcat(command_history[history_count - 1], " ");
				command_history[history_count - 1] = strcat(command_history[history_count - 1], tokens[j]);
			}
			if(remove == true) {
				remove = false;
				command_history[history_count - 1] = strcat(command_history[history_count - 1], " ");
				command_history[history_count - 1] = strcat(command_history[history_count - 1], "&");
			}
		}
		/*
		printf("\n");
		printf("current command is: %s\n", command_history[history_count - 1]);
		printf("\n");
		printf("current history count is: %i\n", history_count);
		printf("\n");
		*/
		// ---------------------------------------------------------CHECK
		/*
		printf("command_history[0] is: %s\n", command_history[0]);
		printf("history count is: %i\n", history_count);

		printf("command is: %s", command_history[history_count - 1]);
		printf("\n");
		*/
		// ---------------------------------------------------------
		// now we have user command and parameter to that command stored in token[]
		// right now we are in parent process

		// internal command
		// if user command is not internal command
		// note: user command is stored in tokens[0]
		// if user command is "exit"

		if(strcmp(tokens[0], "exit") == 0) {
			int i = 1;
			for(; i <= history_count; i++) {
			free(command_history[i - 1]);
			}
			exit(0);
		}
		// if user command is "pwd"
		else if(strcmp(tokens[0], "pwd") == 0) {
			// a char buffer that stores the absolute current path
			char buffer[100];
			// a char pointer pointing to directory
			char *current_dir;
			current_dir = getcwd(buffer, sizeof(buffer));
			write(STDOUT_FILENO, current_dir, strlen(current_dir));
			write(STDOUT_FILENO, "\n", strlen("\n"));
		}
		// if user command is "cd"
		else if(strcmp(tokens[0], "cd") == 0) {
			char *pathname = tokens[1];
			char buffer2[100];
			int path_ret;
			// if user command is cd
			// that is, change to home directory
			if(pathname == NULL) {
				char *home = getenv("HOME");
				path_ret = chdir(home);
			}
			// if user command is cd .. 
			// that is, change to the previous directory
			else if(strcmp(pathname, "..") == 0) {
				char *dir;
				dir = getcwd(buffer2, sizeof(buffer2));
				// now dir stores the absolute pathname
				// get the counts of "/" in dir
				int count = 0;
				int dash_count = 0;
				for(; count < strlen(dir); count++) {
					if(dir[count] == '/') {
						dash_count ++;
					}
				}
				//printf("dash count is: %i\n", dash_count);
				// now we have the count of dash line in dir (CHECK!)
				char *token;
				// dynamically allocate memory for new directory
				char new_dir[100];
				// initialize new_dir to ensure it is empty (nothing left from the last operation)
				new_dir[0] = '\0';
				token = strtok(dir, "/");
				count = 0; 
				while(token != NULL) {
					if(count != dash_count - 1) {
					strcat(new_dir, "/");
					strcat(new_dir, token);
					//printf("tokens are: %s\n", token);
					count ++;
				}
					token = strtok(NULL, "/");
				}
				//printf("the new path is: %s\n", new_dir);
				path_ret = chdir(new_dir);
				// free new directory from memory
				//free(new_dir);
			}
			// if there is parameter after cd
			else {
				path_ret = chdir(pathname);
				// if pathname is invalid (chdir() returns an error)
				if(path_ret != 0) {
					write(STDOUT_FILENO, "No such file or directory", strlen("No such file or directory"));
					write(STDOUT_FILENO, "\n", strlen("\n"));
				}
			}
		}
		// if user command is "history"
		else if(strcmp(tokens[0], "history") == 0) {
			// display history list
			display_history(history_count, command_history);
		}
		else {
		// declare a child process
		pid_t pid;
		// fork a child process
		pid = fork();
		// if fork fails
		if(pid < 0) {
			write(STDOUT_FILENO, "Fork fails!", strlen("Fork fails!"));
		}
		// if child process
		// child process executes user command
		else if(pid == 0) {
			execvp(tokens[0], tokens);
			// make sure exit the sub-shell
			exit(EXIT_FAILURE);
		}
		// if parent process and no child process running in background
		else if((pid != 0) && (in_background == false)){
			// waitpid()
			// first parameter: the id of the process to be waited
			// second parameter: if the value of NULL, the return status of child can be ignored
			// third parameter: option (additional information)
			waitpid(pid, &status, WUNTRACED);
		}
		// if parent process and there is child process running in background
		else if((pid != 0) && (in_background == true)) {
			waitpid(-1, &status, WNOHANG);
		}
	}
		/**
		 * Steps For Basic Shell:
		 * 1. Fork a child process
		 * 2. Child process invokes execvp() using results in token array.
		 * 3. If in_background is false, parent waits for
		 *    child to finish. Otherwise, parent loops back to
		 *    read_command() again immediately.
		 */
	}

	return 0;
}
