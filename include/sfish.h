#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>

void help();
int rl_help(int, int);
int rl_info(int, int);
char *replace_str(char*, char*, char*, int);
int file_exist(char*);
void sigchld_handler(int);
void sigint_handler(int);
void sigtstp_handler(int);

typedef struct job {
	char command[1024];
	pid_t pid;
	int stopped;
	int disowned;
} job;
