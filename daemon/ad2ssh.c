#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "conf.h"
#include "cglobals.h"

// "ssh-key" daemon

// FIXME: DO NOT USE THIS YET. WILL RUN OUT OF CONTROLL

int check_ssh_agent_socket(char *socket_name);

int main(int argc, char **argv)
{

	pid_t ssh_agent_pid, ssh_add_pid;
	int statval1, statval2;

	lib_common_option_handling(argc, argv);

	if (!handle_pid_file_checks(pid_ssh_key, PROGRAM_NAME_SSH_KEY)) {
		printf("handle_pid_file_checks indicates exit required\n");
		exit(1);
	}

	populate_globals();

	if ((ssh_agent_pid=fork()) == -1) {
		fprintf(stderr,"ssh_agent fork failed. Exiting.\n");
		exit(1);        
	}

	if ((ssh_add_pid=fork())  == -1) {
		fprintf(stderr,"ssh_add fork failed. Exiting.\n");
		exit(1);       
	}

	while (1) {
		if (need_finish == TRUE)
			break;

		if (ssh_agent_pid == 0) {
			printf("In ssh_agent_pid\n");
			sleep(20);
			// child 1
			return(0);
		}

		else if (ssh_add_pid == 0) {
			// child 2
			printf("In ssh_add_pid\n");
			exit(0);
		}

		else {

			// parent
			printf("Start parent\n");
			waitpid (ssh_agent_pid, &statval1, WUNTRACED
				#ifdef WCONTINUED
					| WCONTINUED
				#endif
				);

			waitpid (ssh_add_pid, &statval2, WUNTRACED
				#ifdef WCONTINUED
					| WCONTINUED
				#endif
				);
			sleep(4);
			printf("End of parent\n");
		}

	}
	

	return (0);
}

/************************************************************************************************
************************************************************************************************/

int check_ssh_agent_socket(char *socket_name)
{

	struct stat buff;

	if (stat(socket_name, &buff) == 0) {
		printf("File %s exits", socket_name);
		// if socket
		if (S_ISSOCK(buff.st_mode)) {
			printf(" and is a socket.\n");
			return TRUE;
		}
		printf(" and is not a socket. Why? Exiting.\n");
		exit(0);
	} else {
		printf("File %s not created.\n", socket_name);
	}

	return FALSE;

}
