#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "conf.h"

char *sh_command = NULL;
char *directory_script = NULL;
char *directory_daemon = NULL;
char *directory_run = NULL;
char *pid_suffix = NULL;
char *process_file = NULL;
char *ssh_agent = NULL;
char *ssh_add = NULL;
char *ssh_key_file = NULL;
char *ssh_key_fingerprint = NULL;
char *program_process = NULL;
char *pid_process = NULL;
char *pid_ssh_key = NULL;
char *pid_runner = NULL;
char *ssh_agent_sock = NULL;
char *path = NULL;
char *passphrase = NULL;
char *dashboard_user = NULL;
char *dashboard_host = NULL;
char *dashboard_client_yml = NULL;
char *last_clients_yml = NULL;
char *external_trigger = NULL;
int dashboard_port = 0;

int validate_core(void)
{
	if (sh_command == NULL) {
		return FALSE;
	}

	if (directory_script == NULL) {
		return FALSE;
	}

	if (directory_daemon == NULL) {
		return FALSE;
	}

	if (directory_run == NULL) {
		return FALSE;
	}

	if (pid_suffix == NULL) {
		return FALSE;
	}

	if (process_file == NULL) {
		return FALSE;
	}

	if (ssh_agent == NULL) {
		return FALSE;
	}

	if (ssh_add == NULL) {
		return FALSE;
	}

	if (ssh_key_file == NULL) {
		return FALSE;
	}

	if (ssh_key_fingerprint == NULL) {
		return FALSE;
	}

	if (path == NULL) {
		return FALSE;
	}

	if (dashboard_user == NULL) {
		return FALSE;
	}

	if (dashboard_host == NULL) {
		return FALSE;
	}

	if (dashboard_client_yml == NULL) {
		return FALSE;
	}

	if (last_clients_yml == NULL) {
		return FALSE;
	}

	if (dashboard_port == 0) {
		return FALSE;
	}

	return TRUE;
}

int config_load(char *filename)
{
	FILE *fp;
	char *buffer, *key, *value;

	if ((buffer = (char *)calloc(2048, sizeof(char))) == NULL) {
		return FALSE;
	}

	if ((key = (char *)calloc(2048, sizeof(char))) == NULL) {
		return FALSE;
	}

	if ((value = (char *)calloc(2048, sizeof(char))) == NULL) {
		return FALSE;
	}

	if ((fp = fopen(filename, "r")) == NULL) {
		return FALSE;
	}

	while (!feof(fp)) {

		if (fgets(buffer, 2048, fp) == NULL) {
			if (feof(fp)) {
				break;
			}
			return FALSE;
		}

		if (buffer[strlen(buffer) -1] == '\n') {
			buffer[strlen(buffer) -1] = 0;
		}

		if (!strlen(buffer) || buffer[0] == '#') {
			continue;
		}

		if (!sscanf(buffer, "%s %s", key, value)) {
			return FALSE;
		}

		if (!strcasecmp(key, "sh_command")) {
			if ((sh_command = (char *)calloc((strlen(value) + 1), sizeof(char))) == NULL) {
				return FALSE;
			}

			strncpy(sh_command, value, (strlen(value) + 1));
		}

		if (!strcasecmp(key, "ssh_agent")) {
			if ((ssh_agent = (char *)calloc((strlen(value) + 1), sizeof(char))) == NULL) {
				return FALSE;
			}

			strncpy(ssh_agent, value, (strlen(value) + 1));
		}

		if (!strcasecmp(key, "ssh_add")) {
			if ((ssh_add = (char *)calloc((strlen(value) + 1), sizeof(char))) == NULL) {
				return FALSE;
			}

			strncpy(ssh_add, value, (strlen(value) + 1));
		}

		if (!strcasecmp(key, "ssh_key_file")) {
			if ((ssh_key_file = (char *)calloc((strlen(value) + 1), sizeof(char))) == NULL) {
				return FALSE;
			}

			strncpy(ssh_key_file, value, (strlen(value) + 1));
		}

		if (!strcasecmp(key, "ssh_key_fingerprint")) {
			if ((ssh_key_fingerprint = (char *)calloc((strlen(value) + 1), sizeof(char))) == NULL) {
				return FALSE;
			}

			strncpy(ssh_key_fingerprint, value, (strlen(value) + 1));
		}

		if (!strcasecmp(key, "directory_script")) {
			if ((directory_script = (char *)calloc((strlen(value) + 1), sizeof(char))) == NULL) {
				return FALSE;
			}

			strncpy(directory_script, value, (strlen(value) + 1));
		}

		if (!strcasecmp(key, "directory_daemon")) {
			if ((directory_daemon = (char *)calloc((strlen(value) + 1), sizeof(char))) == NULL) {
				return FALSE;
			}

			strncpy(directory_daemon, value, (strlen(value) + 1));
		}

		if (!strcasecmp(key, "directory_run")) {
			if ((directory_run = (char *)calloc((strlen(value) + 1), sizeof(char))) == NULL) {
				return FALSE;
			}

			strncpy(directory_run, value, (strlen(value) + 1));
		}

		if (!strcasecmp(key, "process_file")) {
			if ((process_file = (char *)calloc((strlen(value) + 1), sizeof(char))) == NULL) {
				return FALSE;
			}

			strncpy(process_file, value, (strlen(value) + 1));
		}

		if (!strcasecmp(key, "pid_suffix")) {
			if ((pid_suffix = (char *)calloc((strlen(value) + 1), sizeof(char))) == NULL) {
				return FALSE;
			}

			strncpy(pid_suffix, value, (strlen(value) + 1));
		}

		if (!strcasecmp(key, "path")) {
			if ((path = (char *)calloc((strlen(value) + 1), sizeof(char))) == NULL) {
				return FALSE;
			}

			strncpy(path, value, (strlen(value) + 1));
		}

		if (!strcasecmp(key, "dashboard_user")) {
			if ((dashboard_user = (char *)calloc((strlen(value) + 1), sizeof(char))) == NULL) {
				return FALSE;
			}

			strncpy(dashboard_user, value, (strlen(value) + 1));
		}

		if (!strcasecmp(key, "dashboard_host")) {
			if ((dashboard_host = (char *)calloc((strlen(value) + 1), sizeof(char))) == NULL) {
				return FALSE;
			}

			strncpy(dashboard_host, value, (strlen(value) + 1));
		}

		if (!strcasecmp(key, "dashboard_client_yml")) {
			if ((dashboard_client_yml = (char *)calloc((strlen(value) + 1), sizeof(char))) == NULL) {
				return FALSE;
			}

			strncpy(dashboard_client_yml, value, (strlen(value) + 1));
		}

		if (!strcasecmp(key, "last_clients_yml")) {
			if ((last_clients_yml = (char *)calloc((strlen(value) + 1), sizeof(char))) == NULL) {
				return FALSE;
			}

			strncpy(last_clients_yml, value, (strlen(value) + 1));
		}

		if (!strcasecmp(key, "dashboard_port")) {
			dashboard_port = atoi(value);
		}


	}

	if (!validate_core()) {
		return FALSE;
	}

	fclose(fp);

	// program_process
	sprintf(buffer, "%s %s/%s --smart --force", sh_command, directory_script, PROGRAM_PROCESS);

	if ((program_process = (char *)calloc(strlen(buffer) + 1, sizeof(char))) == NULL) {
		return FALSE;
	}

	strncpy(program_process, buffer, strlen(buffer) + 1);

	// pid_process
	sprintf(buffer, "%s/%s%s.pid", directory_run, PROGRAM_NAME_PROCESS, pid_suffix);

	if ((pid_process = (char *)calloc(strlen(buffer) + 1, sizeof(char))) == NULL) {
		return FALSE;
	}

	strncpy(pid_process, buffer, strlen(buffer) + 1);

	// pid_ssh_key
	sprintf(buffer, "%s/%s%s.pid", directory_run, PROGRAM_NAME_SSH_KEY, pid_suffix);

	if ((pid_ssh_key = (char *)calloc(strlen(buffer) + 1, sizeof(char))) == NULL) {
		return FALSE;
	}

	strncpy(pid_ssh_key, buffer, strlen(buffer) + 1);

	// pid_runner
	sprintf(buffer, "%s/%s%s.pid", directory_run, PROGRAM_NAME_RUNNER, pid_suffix);

	if ((pid_runner = (char *)calloc(strlen(buffer) + 1, sizeof(char))) == NULL) {
		return FALSE;
	}

	strncpy(pid_runner, buffer, strlen(buffer) + 1);

	// process_file
	sprintf(buffer, "%s/%s%s", directory_script, process_file, pid_suffix);

	free(process_file);

	if ((process_file = (char *)calloc(strlen(buffer) + 1, sizeof(char))) == NULL) {
		return FALSE;
	}

	strncpy(process_file, buffer, strlen(buffer) + 1);

	// ssh_agent_sock
	sprintf(buffer, "%s/%s%s.sock", directory_run, PROGRAM_NAME_SSH_KEY, pid_suffix);

	if ((ssh_agent_sock = (char *)calloc(strlen(buffer) + 1, sizeof(char))) == NULL) {
		return FALSE;
	}

	strncpy(ssh_agent_sock, buffer, strlen(buffer) + 1);

	// external_trigger
	sprintf(buffer, "%s/%s", directory_script, EXTERNAL_TRIGGER);

	if ((external_trigger = (char *)calloc(strlen(buffer) + 1, sizeof(char))) == NULL) {
		return FALSE;
	}

	strncpy(external_trigger, buffer, strlen(buffer) + 1);

	// ssh_agent
	sprintf(buffer, "%s -s -a %s", ssh_agent, ssh_agent_sock);

	free(ssh_agent);

	if ((ssh_agent = (char *)calloc(strlen(buffer) + 1, sizeof(char))) == NULL) {
		return FALSE;
	}

	strncpy(ssh_agent, buffer, strlen(buffer) + 1);

	if (getenv("PASSPHRASE")) {
		sprintf(buffer, "%s", getenv("PASSPHRASE"));

		if (buffer[strlen(buffer) -1] == '\n') {
			buffer[strlen(buffer) -1] = 0;
		}

		if ((passphrase = (char *)calloc(strlen(buffer) + 1, sizeof(char))) == NULL) {
			return FALSE;
		}

		strncpy(passphrase, buffer, strlen(buffer) + 1);
	}


	free(buffer);
	free(key);
	free(value);

	// Setup Env
	const char* u = getenv("USER");
	clearenv();
	if (u != NULL)
		setenv("USER", u, 1);

	setenv("HOME", directory_script, 1); 
	setenv("SSH_AUTH_SOCK", ssh_agent_sock, 1);
	setenv("PATH", path, 1);
	setenv("PWD", "/var/tmp", 1);
	setenv("TERM", "vt100", 1);
	setenv("SHELL", sh_command, 1);

	return TRUE;
}
