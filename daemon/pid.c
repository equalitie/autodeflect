#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include "conf.h"

int handle_pid_file_checks(const char *pid_file, const char *program_name)
{
	int exists;
	int running;
	pid_t existing_pid;

	if (!check_pid_file_exists(pid_file, &exists, &existing_pid))
		return FALSE;

	if (!exists) {
		if (!create_pid_file(pid_file))
			return FALSE;
	} else {
		if (!check_process_running(existing_pid, program_name, &running))
			return FALSE;

		// Don't continue if process is already running
		if (running)
			return FALSE;

		// Process isn't running. Create new PID file.
		if (!create_pid_file(pid_file))
			return FALSE;
	}
	return TRUE;
}

/******************************************************************************/
/******************************************************************************/

int check_process_running(pid_t pid, const char *program_name, int *running)
{
	char filename[32];
	char cmdline[1024];
	size_t bytes = 0;
	FILE *fp;
	char *idx;
	char final[1024];

	*running = 0;

	snprintf(filename, sizeof(filename), "/proc/%d/cmdline", pid);

	if ((fp = fopen(filename, "r")) == NULL) {
		// If the error is something other than "file does not exist"
		// consider it a failure.
		if (errno != ENOENT) {
			printf("Failed to open '%s' for reading\n", filename);
			return FALSE;
		}

		// Unable to open the file, but errno was ENOENT meaning
		// the path was invalid. The directory for the process ID
		// being checked does not exist. "running" has already
		// been set to 0. Return "TRUE" for success and to
		// indicate the process it NOT running.
		return TRUE;
	}

	// Leave extra space for appending data by reading in 1000 bytes
	bytes = fread((char *)cmdline, sizeof(char), 1000, fp);

	if (!bytes)
		return FALSE;

	if (bytes != 1000 && !feof(fp))
		return FALSE;

	fclose(fp);

	if ((idx = rindex(cmdline, '/')) != NULL)
		strcpy(final, ++idx);
	else
		strcpy(final, cmdline);

	// If names are the same, process is currently running.
	// If names are different, process is not running but another
	// process has taken its slot in /proc.
	if (strcmp(final, program_name) == 0)
		*running = 1;

	return TRUE;
}

/******************************************************************************/
/******************************************************************************/

int check_pid_file_exists(const char *pid_file, int *exists, pid_t *existing_pid)
{
	FILE *fp;

	*exists = 0;

	if (access(pid_file, F_OK) == 0)
		*exists = 1;

	if (*exists) {
		if ((fp = fopen(pid_file, "r")) == NULL) {
			printf("Failed to open '%s' for reading\n", pid_file);
			return FALSE;
		}

		if (fscanf(fp, "%d", existing_pid) != 1) {
			printf("Failed to retrieve existing PID\n");
			return FALSE;
		}

		fclose(fp);
	}

	return TRUE;

}

/******************************************************************************/
/******************************************************************************/

int create_pid_file(const char *pid_file)
{
	pid_t cur_pid;
	FILE *fp;

	cur_pid = getpid();

	if ((fp = fopen(pid_file, "w")) == NULL) {
		printf("Failed to open '%s' for writing\n", pid_file);
		return FALSE;
	}

	fprintf(fp, "%d\n", cur_pid);

	fclose(fp);

	return TRUE;
}

/******************************************************************************/
/******************************************************************************/

