#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <sys/stat.h>
#include <libssh2.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include "conf.h"
#include "cglobals.h"

/* "ssh-key" daemon */


int check_ssh_agent_socket(char *socket_name);
int check_need_agent(LIBSSH2_SESSION *session);
pid_t run_agent(char *ssh_agent);
int check_loaded_key(char *ssh_key_fingerprint, char *ssh_add, LIBSSH2_SESSION *session);
int load_ssh_key(char *ssh_key_file, char *passphrase);

int main(int argc, char **argv)
{

	LIBSSH2_SESSION *session = NULL;
	pid_t ssh_agent_pid = 0;
	int fail = 0, ping = 0;
	int err = EXIT_SUCCESS;

	lib_common_option_handling(argc, argv);

	if (!handle_pid_file_checks(pid_ssh_key, PROGRAM_NAME_SSH_KEY)) {
		printf("handle_pid_file_checks indicates exit required\n");
		exit(EXIT_FAILURE);
	}

	populate_globals();

	session = libssh2_session_init();

	if (!session) {
		fprintf(stderr, "Failure establishing SSH session\n");
		exit(EXIT_FAILURE);
	}

	while (1) {
		if (need_finish == TRUE) {
			break;
		}

		if (!check_need_agent(session)) {
			if (!check_ssh_agent_socket(ssh_agent_sock)) {
				fprintf (stderr, "ssh-agent needs to be started\n");
				/* This fuction should return -1 or pid */
				if (!(ssh_agent_pid = run_agent(ssh_agent))) {
					fprintf(stderr, "ssh-agent would not start\n");
					err = EXIT_FAILURE;
					break;
				}
			} else {
				fprintf(stderr, "ssh-agent needs to be started but not safe\n");
				err = EXIT_FAILURE;
				break;
			}
		}

		if (!check_loaded_key(ssh_key_fingerprint, ssh_add, session)) {
			fprintf(stderr, "Need to load ssh-key file %s\n", ssh_key_file);
			if (!load_ssh_key(ssh_key_file, passphrase)) {
				fprintf(stderr, "Could not load %s\n", ssh_key_file);
			} else {
				fprintf(stderr, "Loaded %s\n", ssh_key_file);
			}
			fail++;
		} else {
			fail = 0;
		}

		if (fail > 5) {
			fprintf(stderr, "Failed %d times. Fix problem and remove:\n%s Exiting\n", fail, pid_ssh_key);
			/* FIXME: We need to exit clean here., ie; remove .pid */
			break;
		}

		ping++;
		if (ping > 24) {
			fprintf(stderr, "ssh-key: ping\n");
			ping = 0;
		}

		sleep(daemon_interval_ssh_key);

	}

	/* Do some cleanup when break from while loop */
	if(session) {
		libssh2_session_disconnect(session, "Shutdown ssh session");
		libssh2_session_free(session);
	}

	if (ssh_agent_pid)
		kill(ssh_agent_pid, SIGTERM);

	exit(err);
}

/************************************************************************************************
************************************************************************************************/

int check_ssh_agent_socket(char *socket_name)
{

	struct stat buffer;
	int ret = FALSE;

	if (stat(socket_name, &buffer) == 0) {
		fprintf(stderr, "File %s exits", socket_name);
		ret = TRUE;
		/* if socket */
		if (S_ISSOCK(buffer.st_mode)) {
			fprintf(stderr, " and is a socket\n");
		} else {
			fprintf(stderr, " and is not a socket. Why?\n");
		}
	} else {
		fprintf(stderr, "%s not found.\n", socket_name);
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
		exit(EXIT_FAILURE);
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

	FILE *fp = NULL;
	char cmd[256];
	char buffer[512];
	int ret = FALSE;

	if (!check_need_agent(session)) {
		fprintf(stderr, "Checking if key loaded but no agent session\n");
		goto end;
	} 

	sprintf(cmd, "%s -ls", ssh_add);
        if (!(fp = popen(cmd, "r"))) {
		fprintf(stderr, "Could not run %s\n", ssh_add);
		goto end;
        }

	while (fgets(buffer, 512, fp) != NULL) {
		if ((strstr(buffer, ssh_key_fingerprint)) != NULL) {
			ret = TRUE;
			break;
		}
	}

end:

	if (fp) {
		fclose(fp);
	}

	return ret;
}

/************************************************************************************************
************************************************************************************************/

int load_ssh_key(char *ssh_key_file, char *passphrase) {

	EVP_PKEY *privkey = NULL;
	FILE *fp = NULL;
	FILE *pCmd = NULL;
	/* FIXME: 256 is probably overkill, fix later */
	char cmd[256];
	int ret = TRUE;

	OpenSSL_add_all_algorithms();
	privkey = EVP_PKEY_new();

	fp = fopen (ssh_key_file, "r");
	if (!fp) {
		fprintf(stderr, "Could not open %s\n", ssh_key_file);
		ret = FALSE;
		goto end;
	}

	if (!PEM_read_PrivateKey(fp, &privkey, NULL, passphrase)) {
		fprintf(stderr, "Could not create new private key. Passphrase?\n");
		ret = FALSE;
		goto end;
	}

	sprintf(cmd, "%s -", ssh_add);
	pCmd = popen(cmd, "w");
	if (pCmd == NULL) {
		fprintf(stderr, "Could not run %s\n", cmd);
		ret = FALSE;
		goto end;
	}

	sleep(1);

	if (!PEM_write_PrivateKey(pCmd, privkey, NULL, NULL, 0, 0, NULL)) {
		fprintf(stderr, "Could not write new private key\n");
		ret = FALSE;
		goto end;
	}

end:
	if (fp)
		fclose(fp);
	if (privkey)
		EVP_PKEY_free(privkey);
	if (pCmd)
		pclose(pCmd);

	return ret;
}
