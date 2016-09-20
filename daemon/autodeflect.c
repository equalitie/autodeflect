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
#define PROGRAM_ACTION_SSH_KEY	2 
#define PROGRAM_ACTION_RUNNER	4

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
			{"sshkey", 0, 0, 0},
			{"runner", 0, 0, 0},
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
					programs |= PROGRAM_ACTION_SSH_KEY | PROGRAM_ACTION_PROCESS | PROGRAM_ACTION_RUNNER;
				else if (!strcmp(long_options[option_index].name, "process"))
					programs |= PROGRAM_ACTION_PROCESS;
				else if (!strcmp(long_options[option_index].name, "sshkey"))
					programs |= PROGRAM_ACTION_SSH_KEY;
				else if (!strcmp(long_options[option_index].name, "runner"))
					programs |= PROGRAM_ACTION_RUNNER;
				else if (!strcmp(long_options[option_index].name, "version")) {
					show_version();
					exit(EXIT_SUCCESS);
				} else if (!strcmp(long_options[option_index].name, "show-conf")) {
					show_conf = 1;
				} else {
					autodctl_usage();
					exit(EXIT_SUCCESS);
				}
				break;

			default:
				autodctl_usage();
				exit(EXIT_SUCCESS);
		}
	}

	if (show_conf) {
		if (filename == NULL) {
			autodctl_usage();
			exit(EXIT_SUCCESS);
		}
		show_configuration(filename);

		exit(EXIT_SUCCESS);
	}

	if (!mode) {
		autodctl_usage();
		exit(EXIT_SUCCESS);
	}

	if (programs == PROGRAM_ACTION_NONE) {
		autodctl_usage();
		exit(EXIT_SUCCESS);
	}

	if (mode == MODE_START) {
		if (filename == NULL) {
			autodctl_usage();
			exit(EXIT_SUCCESS);
		}

		start_daemons(filename, programs, debug);
	} else if (mode == MODE_STOP) {
		if (filename == NULL) {
			autodctl_usage();
			exit(EXIT_SUCCESS);
		}

		stop_daemons(filename, programs);
	}

	return(EXIT_SUCCESS);
}

/******************************************************************************/
/******************************************************************************/

void start_daemons(char *config_filename, int programs, int debug)
{

	char passphrase[MAX_PASS_SIZE];

	if (!config_load(config_filename)) {
		printf("Failed to load configuration file\n");
		exit(EXIT_FAILURE);
	}

	if (programs & PROGRAM_ACTION_SSH_KEY) {
		printf("passphrase to unlock your key: ");
		fgets(passphrase,MAX_PASS_SIZE,stdin);
		system ("clear");
	/* FIXME: Need to do a check on this passphrase instead of just failing later when tried */
		setenv("PASSPHRASE", passphrase, 1);
		start_program(PROGRAM_NAME_SSH_KEY, pid_ssh_key, config_filename, debug);
		unsetenv("PASSPHRASE");
	}

	if (programs & PROGRAM_ACTION_PROCESS) {
		start_program(PROGRAM_NAME_PROCESS, pid_process, config_filename, debug);
	}

	if (programs & PROGRAM_ACTION_RUNNER) {
		start_program(PROGRAM_NAME_RUNNER, pid_runner, config_filename, debug);
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
		exit(EXIT_FAILURE);
	}

	if (programs & PROGRAM_ACTION_PROCESS) {
		stop_program(PROGRAM_NAME_PROCESS, pid_process);
	}

	if (programs & PROGRAM_ACTION_SSH_KEY) {
		stop_program(PROGRAM_NAME_SSH_KEY, pid_ssh_key);
	}

	if (programs & PROGRAM_ACTION_RUNNER) {
		stop_program(PROGRAM_NAME_RUNNER, pid_runner);
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
		exit(EXIT_FAILURE);
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
	printf("ssh_agent Command:\t%s\n", ssh_agent);
	printf("ssh_add:\t\t%s\n", ssh_add);
	printf("ssh_key_fingerprint:\t%s\n", ssh_key_fingerprint);
	printf("ssh_key_file:\t\t%s\n", ssh_key_file);
	printf("Dasboard Host:\t\t%s\n", dashboard_host); 
	printf("Dasboard Port:\t\t%d\n", dashboard_port); 
	printf("Dasboard User:\t\t%s\n", dashboard_user); 
	printf("Dasboard Client.yml:\t%s\n", dashboard_client_yml); 
	printf("Last Client.yml:\t%s\n", last_clients_yml); 
	printf("PID Directory:\t\t%s\n", directory_run);
	printf("PID File Suffix:\t%s\n", pid_suffix);
	printf("PID File Process:\t%s\n", pid_process);
	printf("PID File sshkey:\t%s\n", pid_ssh_key);
	printf("PID File runner:\t%s\n", pid_runner);

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
	printf("\t\t--sshkey:\tsshkey daemon\n");
	printf("\t\t--runner:\trunner daemon\n");

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
