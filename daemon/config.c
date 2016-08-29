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

char *program_process = NULL;

char *pid_process = NULL;

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


	}

	if (!validate_core()) {
		return FALSE;
	}

	fclose(fp);


	sprintf(buffer, "%s %s/%s", sh_command, directory_script, PROGRAM_PROCESS);

	if ((program_process = (char *)calloc(strlen(buffer) + 1, sizeof(char))) == NULL) {
		return FALSE;
	}

	strncpy(program_process, buffer, strlen(buffer) + 1);


	sprintf(buffer, "%s/%s%s.pid", directory_run, PROGRAM_NAME_PROCESS, pid_suffix);

	if ((pid_process = (char *)calloc(strlen(buffer) + 1, sizeof(char))) == NULL) {
		return FALSE;
	}

	strncpy(pid_process, buffer, strlen(buffer) + 1);

	sprintf(buffer, "%s/%s", directory_script, process_file);

	if ((process_file = (char *)calloc(strlen(buffer) + 1, sizeof(char))) == NULL) {
		return FALSE;
	}

	strncpy(process_file, buffer, strlen(buffer) + 1);

	free(buffer);
	free(key);
	free(value);

	return TRUE;
}
