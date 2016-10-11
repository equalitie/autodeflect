#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include "conf.h"
#include "cglobals.h"

/* "process" daemon */

int main(int argc, char **argv)
{
	double current_load;
	int count = 0;

	lib_common_option_handling(argc, argv);

	if (!handle_pid_file_checks(pid_process, PROGRAM_NAME_PROCESS)) {
		fprintf(stderr, "handle_pid_file_checks indicates exit required\n");
		exit(EXIT_FAILURE);
	}

	populate_globals();

	while (1) {
		if (need_finish == TRUE)
			break;

		if (load_check(&current_load)) {
			if (current_load > max_load) {
				fprintf(stderr, "Load too high\n");
				sleep(daemon_interval_high_load);
				continue;
			}
		} else {
			fprintf(stderr, "Unable to check load\n");
			sleep(daemon_interval_generic);
			continue;
		}

		if (access(process_file, F_OK) == 0) {
			fprintf(stderr, "Need to run '%s' but doing courtesy wait\n", program_process);
			while (count <= 6) {
				if (need_finish == TRUE)
					break;
				fprintf(stderr, "Courtesy sleep for %d sec. on loop count %d of 6\n", daemon_interval_generic, count);
				sleep(daemon_interval_generic);
				count++;
			}
			if (need_finish == TRUE)
				continue;

			count = 0;
				
			fprintf(stderr, "Running '%s'\n", program_process);
			int e = WEXITSTATUS(system(program_process));
			fprintf(stderr, "Exit status of %s is %d\n", program_process, e);
	/* FIXME: unlink only if return success */
			unlink(process_file);

			if (access(process_file, F_OK) == 0) {
				fprintf(stderr, "Could not remove '%s'. EXITING.\n", process_file);
				exit(EXIT_FAILURE);
			}
		}

		sleep(daemon_interval_process);
	}

	return EXIT_SUCCESS;
}
