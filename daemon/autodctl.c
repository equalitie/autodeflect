#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <getopt.h>
#include "conf.h"

#define MODE_START		1
#define MODE_STOP		2

#define PROGRAM_ACTION_NONE	0
#define PROGRAM_ACTION_PROCESS	1

void start_daemons(char *config_filename, int programs, int debug);
void stop_daemons(char *config_filename, int programs);
int start_program(const char *program_name, const char *pid_file, char *config_filename, int debug);
int stop_program(const char *program_name, const char *pid_file);
void show_configuration(char *filename);
void show_version(void);
void autodctl_usage(void);


int main(int argc, char **argv)
{
	int c;
	int mode = 0;
	int debug = 0;
	int show_conf = 0;
	int programs = PROGRAM_ACTION_NONE;
	char *filename = NULL;

	while (1) {
		int option_index = 0;
		static struct option long_options[] = {
			{"start", 0, 0, 0},
			{"stop", 0, 0, 0},
			{"debug", 0, 0, 0},
			{"config", 1, 0, 0},
			{"all", 0, 0, 0},
			{"process", 0, 0, 0},
			{"help", 0, 0, 0},
			{"show-conf", 0, 0, 0},
			{"version", 0, 0, 0},
			{0, 0, 0, 0}
		};

		c = getopt_long(argc, argv, "", long_options, &option_index);

		if (c == -1) {
			break;
		}

		switch (c) {
			case 0:
				if (!strcmp(long_options[option_index].name, "start") && !mode)
					mode = MODE_START;
				else if (!strcmp(long_options[option_index].name, "stop") && !mode)
					mode = MODE_STOP;
				else if (!strcmp(long_options[option_index].name, "debug") && !debug)
					debug = 1;
				else if (!strcmp(long_options[option_index].name, "config"))
					filename = optarg;
				else if (!strcmp(long_options[option_index].name, "all"))
					programs |= PROGRAM_ACTION_PROCESS;
				else if (!strcmp(long_options[option_index].name, "version")) {
					show_version();
					exit(0);
				} else if (!strcmp(long_options[option_index].name, "show-conf")) {
					show_conf = 1;
				} else {
					autodctl_usage();
					exit(0);
				}
				break;

			default:
				autodctl_usage();
				exit(0);
		}
	}

	if (show_conf) {
		if (filename == NULL) {
			autodctl_usage();
			exit(0);
		}
		show_configuration(filename);

		exit(0);
	}

	if (!mode) {
		autodctl_usage();
		exit(0);
	}

	if (programs == PROGRAM_ACTION_NONE) {
		autodctl_usage();
		exit(0);
	}

	if (mode == MODE_START) {
		if (filename == NULL) {
			autodctl_usage();
			exit(0);
		}

		start_daemons(filename, programs, debug);
	} else if (mode == MODE_STOP) {
		if (filename == NULL) {
			autodctl_usage();
			exit(0);
		}

		stop_daemons(filename, programs);
	}

	return(0);
}

/******************************************************************************/
/******************************************************************************/

void start_daemons(char *config_filename, int programs, int debug)
{
	if (!config_load(config_filename)) {
		printf("Failed to load configuration file\n");
		exit(0);
	}

	if (programs & PROGRAM_ACTION_PROCESS) {
		start_program(PROGRAM_NAME_PROCESS, pid_process, config_filename, debug);
	}


}

/******************************************************************************/
/******************************************************************************/

int start_program(const char *program_name, const char *pid_file, char *config_filename, int debug)
{
	int exists;
	int running;
	pid_t existing_pid;
	char filename[1024];

	// From pid.c
	if (!check_pid_file_exists(pid_file, &exists, &existing_pid))
		return FALSE;

	// File exists. Check if program is already running.
	if (exists) {
		// From pid.c
		if (!check_process_running(existing_pid, program_name, &running))
			return FALSE;

		// Program is already running.
		if (running)
			return FALSE;
	}

	// File does not exist or program is not running. Start program.
	// NOTE: Daemons have the previous checks built in. I'm doing this here
	// for possible status reports in the future.

	sprintf(filename, "%s/%s --config %s", directory_daemon, program_name, config_filename);

	if (debug)
		strcat(filename, " --debug &");

	system(filename);

	return TRUE;
	
}

/******************************************************************************/
/******************************************************************************/

void stop_daemons(char *config_filename, int programs)
{
	if (!config_load(config_filename)) {
		printf("Failed to load configuration file\n");
		exit(0);
	}

	if (programs & PROGRAM_ACTION_PROCESS) {
		stop_program(PROGRAM_NAME_PROCESS, pid_process);
	}

}

/******************************************************************************/
/******************************************************************************/

int stop_program(const char *program_name, const char *pid_file)
{
	int exists;
	int running;
	pid_t existing_pid;

	// From pid.c
	if (!check_pid_file_exists(pid_file, &exists, &existing_pid))
		return FALSE;

	// File doesn't exist. Daemon is not running or something has gone
	// very wrong. It is likely not running though. Return Okay.
	if (!exists)
		return TRUE;

	// From pid.c
	if (!check_process_running(existing_pid, program_name, &running))
		return FALSE;

	// Same as file not existing. Either program is not running or
	// something else happened. Return Okay.
	if (!running)
		return TRUE;

	// It is running. Kill it...
	if (kill(existing_pid, SIGTERM) != 0)
		return FALSE;

	if (unlink(pid_file) != 0)
		return FALSE;

	return TRUE;
}

/******************************************************************************/
/******************************************************************************/

void show_configuration(char *filename)
{
	extern char **environ;
	int i = 0;

	if (!config_load(filename)) {
		printf("Failed to load configuration file\n");
		exit(0);
	}

	printf("------------------------------------\n");
	printf("AUTODEFLECT Unified Daemon Configuration:\n");
	printf("------------------------------------\n\n");

	printf("Core:\n");
	printf("-----\n\n");
	printf("Version:\t\t%s\n", VERSION);

	printf("\n");

	printf("Host Specific:\n");
	printf("--------------\n\n");
	printf("Shell Command:\t\t%s\n", sh_command);
	printf("Script Directory:\t%s\n", directory_script);
	printf("Daemon Directory:\t%s\n", directory_daemon);
	printf("Process File:\t\t%s\n", process_file);
	printf("SSH_AGENT Command:\t%s\n", ssh_agent);
	printf("PID Directory:\t\t%s\n", directory_run);
	printf("PID File Suffix:\t%s\n", pid_suffix);
	printf("PID File Process:\t%s\n", pid_process);

	printf("\n");

	printf("Processing Program Command Line:\n");
	printf("--------------------------------\n\n");
	printf("Process Script:\t%s\n", program_process);
	printf("\n");

	printf("Set Environment(s):\n");
	printf("------------------\n\n");

	while(environ[i]) {
		printf("%s\n", environ[i++]);
	}
	printf("\n\n");
}

/******************************************************************************/
/******************************************************************************/

void show_version(void)
{
	printf("autodctl Autodeflect Control Program v%s\n", VERSION);
}

/******************************************************************************/
/******************************************************************************/

void autodctl_usage(void)
{
	printf("Usage: autodctl <options>\n\n");
	printf("Options:\n");
	printf("\tMain Commands (Only one at a time):\n");
	printf("\t\t--start:\tStart daemons\n");
	printf("\t\t--stop:\t\tStop daemons\n");

	printf("\n");
	printf("\tDaemons:\n");
	printf("\t\t--all:\t\tAll daemons\n");
	printf("\t\t--process:\tProcess daemon\n");

	printf("\n");
	printf("\tModifiers:\n");
	printf("\t\t--debug:\tEnable debug mode (start only)\n");
	printf("\t\t--config:\tSpecify the configuration file to use\n");

	printf("\n");
	printf("\tMiscellaneous:\n");
	printf("\t\t--version:\tShow software version\n");
	printf("\t\t--show-conf:\tShow configuration information\n");
	printf("\t\t--help:\t\tThis message\n");
}

