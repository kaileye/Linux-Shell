#include "sfish.h"

char buf1[1024];
int jobcnt = 1, cmdcnt = 0, fgpid = 0;
job jobs[4096];

int main(int argc, char** argv) {
    //DO NOT MODIFY THIS. If you do you will get a ZERO.
    rl_catch_signals = 0;
    //This is disable readline's default signal handlers, since you are going
    //to install your own.

    char *cmd, *arg, arg1[1024], *args[1024], *cmds[1024], *wd, *token, path[4096], cwd[1024], prev[1024], prev2[1024], msg[1024], host[1024], in[1024], out[1024], setting[10], color[10], buf[256], buf2[2048];
	const char s[2] = " ";	
	pid_t pid;
	int status = 0, cdbasecase = 0, user = 1, machine = 1, userbold = 0, machinebold = 0, usercolor = 0, machinecolor = 0, cnt, cnt2, exist, infd, outfd, ifile, ofile, p[2], i, bg;

	rl_bind_keyseq("\\C-h", rl_help);
	rl_bind_keyseq("\\C-p", rl_info);
	signal(SIGCHLD, sigchld_handler);
	signal(SIGINT, sigint_handler);
	signal(SIGTSTP, sigtstp_handler);

    while(1) {
		gethostname(host, sizeof(host));
		if (getcwd(cwd, sizeof(cwd)) == NULL) {
			perror("getcwd");
			exit(EXIT_FAILURE);			
		}
		wd = replace_str(cwd, getenv("HOME"), "~", 0);
		memset(msg, 0, 1024);
		strcpy(msg, "sfish");
		if (user == 1 || machine == 1) {
			strcat(msg, "-");
		}
		if (user == 1) {
			strcat(msg, "\033[");
			if (userbold == 0 && usercolor == 0) {
				strcat(msg, "0m");
			} else if (userbold == 1) {
				strcat(msg, "1");
				if (usercolor != 0) {
					strcat(msg, ";");
					sprintf(buf, "%d", usercolor);
					strcat(msg, buf);
				}
				strcat(msg, "m");
			} else if (usercolor != 0) {
				sprintf(buf, "%d", usercolor);
				strcat(msg, buf);
				strcat(msg, "m");
			}
			strcat(msg, getenv("USER"));
			strcat(msg, "\033[0m");
		}
		if (user == 1 && machine == 1) {
			strcat(msg, "@");
		}
		if (machine == 1) {
			strcat(msg, "\033[");
			if (machinebold == 0 && machinecolor == 0) {
				strcat(msg, "0m");
			} else if (machinebold == 1) {
				strcat(msg, "1");
				if (machinecolor != 0) {
					strcat(msg, ";");
					sprintf(buf, "%d", machinecolor);
					strcat(msg, buf);
				} 
				strcat(msg, "m");
			} else if (machinecolor != 0) {
				sprintf(buf, "%d", machinecolor);
				strcat(msg, buf);
				strcat(msg, "m");
			}
			strcat(msg, host);
			strcat(msg, "\033[0m");
		}
		strcat(msg, ":[");
		strcat(msg, wd);
		strcat(msg, "]> ");
		cmd = readline(msg);
		cmdcnt++;
		if (cmd == NULL) {
			printf("hi\n");
			break;
		}
        if (strcmp(cmd, "exit") == 0) {
            exit(EXIT_SUCCESS);
		} else if (strcmp(cmd, "help") == 0) {
			pid = fork();
			if (pid == 0) {
				help();			
				exit(EXIT_SUCCESS);
			} else if (pid > 0) {
				wait(&status);			
			} else {
				perror("fork");
				exit(EXIT_FAILURE);
			} 
		} else if (strncmp(cmd, "cd", 2) == 0) {
			if ((arg = malloc(strlen(cmd)-2)) == NULL) {
				perror("malloc");
				exit(EXIT_FAILURE);			
			}
			if (strlen(cmd) > 2) {
				strncpy(arg, cmd+3, strlen(cmd)-2);
				if (strcmp(arg, "") == 0) {
					if (getcwd(prev, sizeof(prev)) == NULL) {
						perror("getcwd");
						exit(EXIT_FAILURE);			
					}
					chdir(getenv("HOME"));
					cdbasecase = 1;
				} else if (strcmp(arg, "-") == 0) {
					if (cdbasecase == 0) {
						fprintf(stderr, "sfish: cd: OLDPWD not set\n");
						status = 1;
						continue;
					} else {
						if (getcwd(prev2, sizeof(prev2)) == NULL) {
							perror("getcwd");
							exit(EXIT_FAILURE);			
						}					
						chdir(prev);
						strcpy(prev, prev2);		
					}
				} else {
					if (getcwd(prev, sizeof(prev)) == NULL) {
						perror("getcwd");
						exit(EXIT_FAILURE);			
					}
					chdir(arg);
					cdbasecase = 1;
				}
			} else {
				if (getcwd(prev, sizeof(prev)) == NULL) {
					perror("getcwd");
					exit(EXIT_FAILURE);			
				}
				chdir(getenv("HOME"));
				cdbasecase = 1;
			}
			free(arg);
		} else if (strcmp(cmd, "pwd") == 0) {
			pid = fork();
			if (pid == 0) {
				if (getcwd(cwd, sizeof(cwd)) == NULL) {
					perror("getcwd");
					exit(EXIT_FAILURE);			
				}
				printf("%s\n", cwd);
				exit(EXIT_SUCCESS);
			} else if (pid > 0) {
				wait(&status);			
			} else {
				perror("fork");
				exit(EXIT_FAILURE);
			}
		} else if (strcmp(cmd, "prt") == 0) {
			pid = fork();
			if (pid == 0) {
				printf("%d\n", status);
				exit(EXIT_SUCCESS);		
			} else if (pid > 0) {
				wait(&status);	
			} else {
				perror("fork");
				exit(EXIT_FAILURE);
			}
		} else if (strncmp(cmd, "chpmt", 5) == 0) {
			if (strncmp(cmd+6, "user", 4) != 0 && strncmp(cmd+6, "machine", 7) != 0) {
				fprintf(stderr, "Input either user or machine settings\n");
				status = 1;	
				continue;	
			} else if (strncmp(cmd+6, "user", 4) == 0) {
				if (strncmp(cmd+11, "1", 1) == 0) {
					user = 1;
				} else if (strncmp(cmd+11, "0", 1) == 0) {
					user = 0;				
				} else {
					fprintf(stderr, "Input valid toggle after setting\n");
					status = 1;
					continue;
				}
			} else if (strncmp(cmd+6, "machine", 7) == 0) {
				if (strncmp(cmd+14, "1", 1) == 0) {
					machine = 1;
				} else if (strncmp(cmd+14, "0", 1) == 0) {
					machine = 0;
				} else {
					fprintf(stderr, "Input valid toggle after setting\n");
					status = 1;
					continue;
				}
			}
		} else if (strncmp(cmd, "chclr", 5) == 0) {
			token = strtok(cmd, s);
			token = strtok(NULL, s);
			if (token == NULL || (strcmp(token, "user") != 0 &&	strcmp(token, "machine") != 0)) {
				fprintf(stderr, "Input either user or machine settings\n");
				status = 1;
				continue;
			} else {
				strcpy(setting, token);
				token = strtok(NULL, s);
				if (token == NULL || (strcmp(token, "red") != 0 &&	strcmp(token, "blue") != 0 && strcmp(token, "green") != 0 && strcmp(token, "yellow") != 0 && strcmp(token, "cyan") != 0 && strcmp(token, "magenta") != 0 && strcmp(token, "black") != 0 && strcmp(token, "white") != 0)) {
					fprintf(stderr, "Input a valid color\n");
					status = 1;
					continue;
				} else {
					strcpy(color, token);
					token = strtok(NULL, s);
					if (token == NULL || (strcmp(token, "0") != 0 &&	strcmp(token, "1") != 0)) {
						fprintf(stderr, "Input either 0 or 1 to disable or enable boldness\n");
						status = 1;
						continue;
					} else {
						if (strcmp(setting, "user") == 0) {
							if (strcmp(color, "red") == 0) {
								usercolor = 31;							
							} else if (strcmp(color, "blue") == 0) {
								usercolor = 34;							
							} else if (strcmp(color, "green") == 0) {
								usercolor = 32;							
							} else if (strcmp(color, "yellow") == 0) {
								usercolor = 33;							
							} else if (strcmp(color, "cyan") == 0) {
								usercolor = 36;							
							} else if (strcmp(color, "magenta") == 0) {
								usercolor = 35;							
							} else if (strcmp(color, "black") == 0) {
								usercolor = 30;							
							} else if (strcmp(color, "white") == 0) {
								usercolor = 37;							
							}
							if (strcmp(token, "0") == 0) {
								userbold = 0;	
							} else {
								userbold = 1;
							}
						} else if (strcmp(setting, "machine") == 0) {
							if (strcmp(color, "red") == 0) {
								machinecolor = 31;							
							} else if (strcmp(color, "blue") == 0) {
								machinecolor = 34;							
							} else if (strcmp(color, "green") == 0) {
								machinecolor = 32;							
							} else if (strcmp(color, "yellow") == 0) {
								machinecolor = 33;							
							} else if (strcmp(color, "cyan") == 0) {
								machinecolor = 36;							
							} else if (strcmp(color, "magenta") == 0) {
								machinecolor = 35;							
							} else if (strcmp(color, "black") == 0) {
								machinecolor = 30;							
							} else if (strcmp(color, "white") == 0) {
								machinecolor = 37;							
							}							
							if (strcmp(token, "0") == 0) {
								machinebold = 0;
							} else { 
								machinebold = 1;
							}
						}
					}
				}
			}
		} else if (strcmp(cmd, "jobs") == 0) {
			for (i = 1; i < jobcnt; i++) {
				if (kill(jobs[i].pid, 0) == 0 && jobs[i].disowned == 0) {
					printf("[%d]\t", i);
					if (jobs[i].stopped == 0) {
						printf("Running\t\t");
					} else {
						printf("Stopped\t\t");
					}
					printf("%d\t%s\n", jobs[i].pid, jobs[i].command);
				}
			}
		} else if (strncmp(cmd, "fg", 2) == 0) {
			token = strtok(cmd, " ");
			token = strtok(NULL, " ");
			if (token != NULL) {
				if (strstr(token, "%") != NULL) {
					token = strtok(token, "%");
					fgpid = jobs[atoi(token)].pid;
					kill(jobs[atoi(token)].pid, SIGCONT);
					jobs[atoi(token)].stopped = 0;
					waitpid(jobs[atoi(token)].pid, &status, WUNTRACED);
					fgpid = 0;
				} else {
					fgpid = atoi(token);
					kill(atoi(token), SIGCONT);
					for (i = 1; i < jobcnt; i++) {
						if (jobs[i].pid == atoi(token)) {
							jobs[i].stopped = 0;					
						}
					}
					waitpid(atoi(token), &status, WUNTRACED);
					fgpid = 0;
				}
			}
		} else if (strncmp(cmd, "bg", 2) == 0) {
			token = strtok(cmd, " ");
			token = strtok(NULL, " ");
			if (token != NULL) {
				if (strstr(token, "%") != NULL) {
					token = strtok(token, "%");
					kill(jobs[atoi(token)].pid, SIGCONT);
					jobs[atoi(token)].stopped = 0;
				} else {
					kill(atoi(token), SIGCONT);
					for (i = 1; i < jobcnt; i++) {
						if (jobs[i].pid == atoi(token)) {
							jobs[i].stopped = 0;					
						}
					}
				}
			}
		} else if (strncmp(cmd, "kill", 3) == 0) {
			token = strtok(cmd, " ");
			token = strtok(NULL, " ");
			if (token != NULL) {
				strcpy(arg1, token);
				token = strtok(NULL, " ");
				if (token != NULL) {
					if (atoi(arg1) >= 1 && atoi(arg1) <= 31) {
						if (strstr(token, "%") != NULL) {
							token = strtok(token, "%");
							kill(jobs[atoi(token)].pid, atoi(arg1));
							if (atoi(arg1) == 23) {
								jobs[atoi(token)].stopped = 1;
							} else if (atoi(arg1) == 25) {
								jobs[atoi(token)].stopped = 0;
							}
						} else {
							kill(atoi(token), atoi(arg1));
							if (atoi(arg1) == 23) {
								for (i = 1; i < jobcnt; i++) {
									if (jobs[i].pid == atoi(token)) {
										jobs[i].stopped = 1;
									}
								}
							} else if (atoi(arg1) == 25) {
								for (i = 1; i < jobcnt; i++) {
									if (jobs[i].pid == atoi(token)) {
										jobs[i].stopped = 0;
									}
								}
							}
						}
					}
				} else {
					if (strstr(arg1, "%") != NULL) {
						token = strtok(arg1, "%");
						kill(jobs[atoi(token)].pid, SIGTERM);
					} else {
						kill(atoi(arg1), SIGTERM);
					}
				}
			}
		} else if (strncmp(cmd, "disown", 6) == 0) {
			token = strtok(cmd, " ");
			token = strtok(NULL, " ");
			if (token != NULL) {
				if (strstr(token, "%") != NULL) {
					token = strtok(token, "%");
					jobs[atoi(token)].disowned = 1;
				} else {
					for (i = 1; i < jobcnt; i++) {
						if (jobs[i].pid == atoi(token)) {
							jobs[i].disowned = 1;					
						}
					}
				}
			} else {
				for (i = 1; i < jobcnt; i++) {
					jobs[i].disowned = 1;
				}
			}
		} else if (strcmp(cmd, "") == 0) {
			cmdcnt--;
			continue;
		} else {
			strcpy(buf1, cmd);
			bg = 0;
			if (strstr(cmd, "&") != NULL) {
				bg = 1;
				cmd = strtok(cmd, "&");
			}
			pid = fork();
			if (pid == 0) {
				setpgid(0,0);
				cnt2 = 0;
				infd = 0;
				outfd = 1;
				memset(cmds, 0, sizeof(cmds));

				token = strtok(cmd, "|");
				while (token != NULL) {
					cmds[cnt2] = token;					
					token = strtok(NULL, "|");
					cnt2++;
				}
				cmds[cnt2] = NULL;
				for (i = 0; cmds[i] != NULL; i++) {
					cnt = 0;
					exist = 0;
					ifile = 0;
					ofile = 0;
					memset(args, 0, sizeof(args));
					memset(in, 0, sizeof(in));
					memset(out, 0, sizeof(out));
				
					if (strstr(cmd, "<") != NULL) {
						if (i > 0) {
							fprintf(stderr, "Invalid arguments\n");
							break;
						}
						token = strtok(cmds[i], "<");
						strcpy(buf2, token);
						token = strtok(NULL, "<");
						if (strstr(token, ">") != NULL) {
							if (cmds[i+1] != NULL) {
								fprintf(stderr, "Invalid arguments\n");
								break;
							}
							token = strtok(token, " >");
							strcpy(in, token);
							ifile = 1;
							token = strtok(NULL, " >");
							strcpy(out, token);
							ofile = 1;
							token = strtok(NULL, " >");
							if (token != NULL) {
								fprintf(stderr, "Invalid arguments\n");
								break;
							}					
						} else {
							token = strtok(token, " ");
							strcpy(in, token);
							ifile = 1;
							token = strtok(NULL, " ");
							if (token != NULL) {
								fprintf(stderr, "Invalid arguments\n");
								break;
							}	
		
						}
					} else if (strstr(cmds[i], ">") != NULL) {
						if (cmds[i+1] != NULL) {
							fprintf(stderr, "Invalid arguments\n");
							break;
						}
						token = strtok(cmds[i], ">");
						strcpy(buf2, token);
						token = strtok(NULL, ">");
						token = strtok(token, " ");
						strcpy(out, token);
						ofile = 1;
						token = strtok(NULL, " ");
						if (token != NULL) {
							fprintf(stderr, "Invalid arguments\n");
							break;
						}	
					} else {
						strcpy(buf2, cmds[i]);
					}
					if (ifile == 1 && !file_exist(in)) {
						fprintf(stderr, "%s: No such file or directory\n", in);
						break;
					}

					token = strtok(buf2, " ");
					while (token != NULL) {
						args[cnt] = token;
						token = strtok(NULL, " ");
						cnt++;
					}
					args[cnt] = NULL;
					arg = strstr(args[0], "/");
					if (arg == NULL) {
						strcpy(path, getenv("PATH"));
						token = strtok(path, ":");
						while (token != NULL) {
							strcpy(cwd, token);
							strcat(cwd, "/");
							strcat(cwd, args[0]);
							if (file_exist(cwd)) {
								exist = 1;
								break;
							}
							token = strtok(NULL, ":");
						}
					} else if (file_exist(args[0])) {
						exist = 1;
					}
					if (exist == 0) {
						fprintf(stderr, "Command not found\n");
						break;
					}
			
					if (ifile == 1) {
						infd = open(in, O_RDONLY, 0666);
					}
					if (ofile == 1) {
						outfd = open(out, O_WRONLY | O_CREAT | O_TRUNC, 0666);	
					}
					if (cnt2 > 1) {
						if (pipe(p) == -1) {
							perror("pipe");
							exit(1);
						}
					}
					pid = fork();
					if (pid == 0) {
						dup2(infd, 0);
						if (ofile == 1) {
							dup2(outfd, 1);
						} else if (cmds[i+1] != NULL) {
							dup2(p[1], 1);
						}
						if (cnt2 > 1) {
							close(p[0]);
						}
						execvp(args[0], args);
						exit(EXIT_FAILURE);
					} else if (pid > 0) {
						wait(&status);
						if (cnt2 > 1) {
							close(p[1]);
						}
						if (cnt2 > 1) {
							infd = p[0];
						}
					} else {
						perror("fork");
						exit(EXIT_FAILURE);
					}
				}
				exit(EXIT_SUCCESS);
			} else if (pid > 0) {
				if (bg == 0) {
					fgpid = pid;
					waitpid(pid, &status, WUNTRACED);
					fgpid = 0;
				} else {
					strcpy(jobs[jobcnt].command, buf1);
					jobs[jobcnt].pid = pid;
					jobs[jobcnt].stopped = 0;
					jobs[jobcnt].disowned = 0;
					jobcnt++;
				}
			} else {
				perror("fork");
				exit(EXIT_FAILURE);
			}
		}
	
        //All your debug print statments should be surrounded by this #ifdef
        //block. Use the debug target in the makefile to run with these enabled.
        #ifdef DEBUG
        fprintf(stderr, "Length of command entered: %ld\n", strlen(cmd));
        #endif
        //You WILL lose points if your shell prints out garbage values.
		free(cmd);
    }

    //Don't forget to free allocated memory, and close file descriptors.
    free(cmd);
    //WE WILL CHECK VALGRIND!

    return EXIT_SUCCESS;
}

void help() {
	printf("exit: exit shell\n");
	printf("cd [DIR]: change current working directory\n");
	printf("pwd: print absolute path of current working directory\n");
	printf("prt: print return code of last executed command\n");
	printf("chpmt [SETTING][TOGGLE]: change prompt settings\n");
	printf("chclr [SETTING][COLOR][BOLD]: change prompt colors\n");
}

int rl_help(int count, int key) {
	printf("\n");
	help();
	rl_done = 1;
	return 0;
}

int rl_info(int count, int key) {
	int i;
	printf("\n");
	printf("----Info----\n");
	printf("help\n");
	printf("prt\n");
	printf("----CTRL----\n");
	printf("cd\n");
	printf("chclr\n");
	printf("chpmt\n");
	printf("pwd\n");
	printf("exit\n");
	printf("----Job Control----\n");
	printf("bg\n");
	printf("fg\n");
	printf("disown\n");
	printf("jobs\n");
	printf("----Number of Commands Run----\n");
	printf("%d\n", cmdcnt);
	printf("----Process Table----\n");
	printf("PGID\tPID\tTIME\tCMD\n");
	for (i = 1; i < jobcnt; i++) {
		if (kill(jobs[i].pid, 0) == 0 && jobs[i].stopped == 0) {
			printf("%d\t%d\t00:00\t%s\n", getpgid(jobs[i].pid), jobs[i].pid, jobs[i].command);
		}
	}	

	rl_done = 1;
	return 0;
}

//helper function found online, to replace the home directory with ~
char *replace_str(char *str, char *orig, char *rep, int start) {
	static char temp[4096];
	static char buffer[4096];
	char *p;

	strcpy(temp, str + start);

	if(!(p = strstr(temp, orig))) {
		return temp;
	}
	strncpy(buffer, temp, p-temp);
	buffer[p-temp] = '\0';
	sprintf(buffer + (p - temp), "%s%s", rep, p + strlen(orig));
	sprintf(str + start, "%s", buffer);    
	return str;
}

int file_exist(char *file) {
	struct stat buf;
	
	return (stat(file, &buf) == 0);
}

void sigchld_handler(int s) {
	int olderrno = errno, i;
	pid_t pid;
	
	pid = waitpid(-1, 0, WNOHANG);
	for (i = 1; i < jobcnt; i++) {
		if (jobs[i].pid == pid) {
			jobs[i].stopped = 1;
		}
	}
	errno = olderrno;
}

void sigint_handler(int s) {
	if (fgpid != 0) {
		kill(-fgpid, SIGKILL);
		printf("\n");
	}
}

void sigtstp_handler(int s) {
	int exist = 0, i;

	if (fgpid != 0) {
		kill(-fgpid, SIGSTOP);
		printf("\n");
	
		for (i = 0; i < jobcnt; i++) {
			if (jobs[i].pid == fgpid) {
				jobs[i].stopped = 1;
				exist = 1;
			}
		}
		if (exist == 0) {
			strcpy(jobs[jobcnt].command, buf1);
			jobs[jobcnt].pid = fgpid;
			jobs[jobcnt].stopped = 1;
			jobs[jobcnt].disowned = 0;
			jobcnt++;
		}
	}
}

