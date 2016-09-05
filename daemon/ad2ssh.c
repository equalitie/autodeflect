#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <libssh2.h>
#include "conf.h"
#include "cglobals.h"

// "ssh-key" daemon

// FIXME: DO NOT USE THIS YET. WILL RUN OUT OF CONTROLL

int check_ssh_agent_socket(char *socket_name);
int check_need_agent(LIBSSH2_SESSION *session);
pid_t run_agent(char *ssh_agent);

int main(int argc, char **argv)
{

	pid_t ssh_agent_pid;

	LIBSSH2_SESSION *session = NULL;

	lib_common_option_handling(argc, argv);

	if (!handle_pid_file_checks(pid_ssh_key, PROGRAM_NAME_SSH_KEY)) {
		printf("handle_pid_file_checks indicates exit required\n");
		exit(1);
	}

	populate_globals();

	session = libssh2_session_init();

	if (!session) {
		fprintf(stderr, "Failure establishing SSH session\n");
		exit(1);
	}

	while (1) {
		if (need_finish == TRUE)
			break;

		if (!check_need_agent(session)) {
			if (!check_ssh_agent_socket(ssh_agent_sock)) {
				printf("We should start ssh-agent\n");
				// This fuction should return -1 or pid
				ssh_agent_pid = run_agent(ssh_agent);
			} else {
				fprintf(stderr, "Need ssh-agent but not safe to start\n");
				break;
			}
		}


		printf("Hello\n");
		sleep(5);

	}

	if(session) {
		libssh2_session_disconnect(session, "Shutdown ssh session");
		libssh2_session_free(session);
	}

	if (ssh_agent_pid)
		kill(ssh_agent_pid, SIGTERM);

	return (0);
}

/************************************************************************************************
************************************************************************************************/

int check_ssh_agent_socket(char *socket_name)
{

	struct stat buff;
	int ret = FALSE;

	if (stat(socket_name, &buff) == 0) {
		fprintf(stderr, "File %s exits", socket_name);
		// if socket
		if (S_ISSOCK(buff.st_mode)) {
			printf(" and is a socket.\n");
			ret = TRUE;
		}
		printf(" and is not a socket. Why? Exiting.\n");
		exit(1);
	} else {
		printf("File %s not created.\n", socket_name);
	}

	return(ret);

}

/************************************************************************************************
************************************************************************************************/

int check_need_agent(LIBSSH2_SESSION *session) {

	LIBSSH2_AGENT *agent = NULL;
	int ret;

	agent = libssh2_agent_init(session);

	if (!agent) {
		/* Not sure why this would ever happen, but bad if it does. */
		fprintf(stderr, "Failure establishing ssh-agent session\n");
		exit(1);
	}


	if (libssh2_agent_connect(agent)) 
		ret = FALSE;
	else
		ret = TRUE;


	libssh2_agent_disconnect(agent);
	libssh2_agent_free(agent);

	return(ret);

}

/************************************************************************************************
************************************************************************************************/

pid_t run_agent(char *ssh_agent) {

	pid_t pid;
	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;

	pid = -1;

	fp = popen(ssh_agent, "r");

	while ((read = getline(&line, &len, fp)) != -1) {
		/** FIXME: find SSH_AGENT_PID=####  key=value  atoi value for pid **/
		printf("%s", line);
	}

	free(line);
	fclose(fp);

	/** FIXME: force pid = -1 for now to make sure we do not kill while testing **/
	pid = -1;
	return(pid);

}

/************************************************************************************************
************************************************************************************************/
