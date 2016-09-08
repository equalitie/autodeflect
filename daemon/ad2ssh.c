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

/* "ssh-key" daemon */


int check_ssh_agent_socket(char *socket_name);
int check_need_agent(LIBSSH2_SESSION *session);
pid_t run_agent(char *ssh_agent);
int check_loaded_key(char *ssh_key_fingerprint, char *ssh_add, LIBSSH2_SESSION *session);
int load_ssh_key(char *ssh_add, char *ssh_key_file, char *passphrase);

int main(int argc, char **argv)
{

	LIBSSH2_SESSION *session = NULL;
	pid_t ssh_agent_pid;
	int fail = 0;

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
				printf ("We should start ssh-agent\n");
				/* This fuction should return -1 or pid */
				if (!(ssh_agent_pid = run_agent(ssh_agent))) {
					fprintf(stderr, "ssh-agent would not start\n");
					break;
				}
			} else {
				fprintf(stderr, "Need ssh-agent but not safe to start\n");
				break;
			}
		}

		if (!check_loaded_key(ssh_key_fingerprint, ssh_add, session)) {
			printf("Need to load ssh-key file %s\n", ssh_key_file);
			fail++;
		} else {
			fail = 0;
		}

		if (fail > 5) {
			fprintf(stderr, "Failed %d times. Fix problem and remove:\n%s Exiting\n", fail, pid_ssh_key);
			/* FIXME: We need to exit clean here., ie; remove .pid */
			break;
		}


		printf("EHLO\n");
		printf("ssh_agent_pid: %d\n", ssh_agent_pid);
		sleep(5);

	}

	/* Do some cleanup when break from while loop */
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

	struct stat buffer;
	int ret = FALSE;

	if (stat(socket_name, &buffer) == 0) {
		fprintf(stderr, "File %s exits", socket_name);
		/* if socket */
		if (S_ISSOCK(buffer.st_mode)) {
			printf(" and is a socket.\n");
			ret = TRUE;
		}
		printf(" and is not a socket. Why? Exiting.\n");
		exit(1);
	} else {
		printf("%s not found.\n", socket_name);
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
/* ssh_agent */
	FILE *fp;
	char *buffer, *ssh_agent_pid;
	size_t bufsiz = 0;
	ssize_t nbytes;
	pid_t pid = -1;

	if (!(fp = popen(ssh_agent, "r"))) {
		return pid;
	}

	while ((nbytes = getline(&buffer, &bufsiz, fp) != -1)) {
		if (buffer[strlen(buffer) -1] == '\n') {
			buffer[strlen(buffer) -1] = 0;
		}

		if (!strlen(buffer)) {
			continue;
		}

		strtok(buffer, ";");

		char* value = strtok(buffer, "=");

		if (!strcasecmp(buffer, "SSH_AGENT_PID")) {
			while (value != NULL) {
				value = strtok(NULL, "=");
				if (value != NULL) {
					if ((ssh_agent_pid = (char *)calloc((strlen(value) + 1), sizeof(char))) == NULL) {
						break;
					}

					strncpy(ssh_agent_pid, value, (strlen(value) + 1));
					strtok(NULL, "=");
				}
			}
		}
	}

	if (!(pid = atoi(ssh_agent_pid)))
		pid = -1;

	free(buffer);
	free(ssh_agent_pid);
	pclose(fp);

	return pid;
}

/************************************************************************************************
************************************************************************************************/

int check_loaded_key(char *ssh_key_fingerprint, char *ssh_add, LIBSSH2_SESSION *session) {

	FILE *fp;
	char command[256];
	char buffer[512];
	int ret = FALSE;

	if (!check_need_agent(session)) {
		return(ret);
	} 

	sprintf(command, "%s -ls", ssh_add);
        if (!(fp = popen(command, "r"))) {
                return(ret);
        }

	while (fgets(buffer, 512, fp) != NULL) {
		if ((strstr(buffer, ssh_key_fingerprint)) != NULL) {
			ret = TRUE;
			break;
		} else {
			continue;
		}
	}

	if (fp) {
		fclose(fp);
	}

	return(ret);
}

/************************************************************************************************
************************************************************************************************/

int load_ssh_key(char *ssh_add, char *ssh_key_file, char *passphrase) {

	pid_t pid;
	int rv;
	int extpipe[2];

	pipe(extpipe);

	if ((pid=fork()) == -1) {
		fprintf(stderr, "Could not fork\n");
		exit(1);
	}

	if (pid) {
		/* We are parent */
		dup2(extpipe[1],1);
		close(extpipe[0]);
		setvbuf(stdout,(char*)NULL,_IONBF,0);
		sleep(5);
		printf("%s\n", passphrase);
		wait(&rv);
		if (rv != 0) {
			return(FALSE);
		}
	} else {
		/* We are child */
		dup2(extpipe[0],0);
		close(extpipe[1]);
		/* FIXME: Need to make this handler 
		signal(SIGALARM, my_handle); */
		alarm(10);
		if (execl(ssh_add,ssh_add,ssh_key_file, (char*) NULL) == -1) {
			fprintf(stderr, "ssh-add failed\n");
			exit(1);
		}
		return(0);
	}

	return(TRUE);

}
