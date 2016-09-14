#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include "conf.h"

void lib_common_option_handling(int argc, char **argv)
{
	int c, debug = 0;
	char *filename = NULL;

	while(1) {
		int option_index = 0;
		static struct option long_options[] = {
			{"debug", 0, 0, 0},
			{"config", 1, 0, 0},
			{0, 0, 0, 0}
		};

		c = getopt_long(argc, argv, "", long_options, &option_index);

		if (c == -1) {
			break;
		}

		switch (c) {
			case 0:
				if (!strcmp(long_options[option_index].name, "debug")) {
					debug = 1;
				} else if (!strcmp(long_options[option_index].name, "config")) {
					filename = optarg;
				}
				break;

			default:
				exit(0);
		}
	}

	if (filename == NULL) {
		printf("Missing filename\n");
		exit(1);
	}

	if (!config_load(filename)) {
		printf("Failed to load configuration file\n");
		exit(0);
	}

	if (!debug) {
		daemon(0, 0);
	}
}

/******************************************************************************/
/******************************************************************************/
