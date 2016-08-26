#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include "conf.h"

int populate_globals(void)
{
	signal(SIGINT, finish_handler);
	signal(SIGTERM, finish_handler);

	return TRUE;
}
