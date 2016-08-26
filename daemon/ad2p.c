#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include "conf.h"
#include "cglobals.h"

// "process" daemon

int main(int argc, char **argv)
{
	double current_load;
	char command[256];

	lib_common_option_handling(argc, argv);

	if (!handle_pid_file_checks(pid_process, PROGRAM_NAME_PROCESS)) {
		printf("handle_pid_file_checks indicates exit required\n");
		exit(1);
	}

	populate_globals();

	while (1) {
		if (need_finish == TRUE)
			break;

		if (load_check(&current_load)) {
			if (current_load > max_load) {
				printf("Load too high\n");
				sleep(daemon_interval_high_load);
				continue;
			}
		} else {
			printf("Unable to check load\n");
			sleep(daemon_interval_generic);
			continue;
		}

		sprintf(command, "%s %d %d &", program_process, 0, 0);
		printf("Running '%s'\n", command);
		system(command);

		sleep(daemon_interval_process);
	}

	return 0;
}
