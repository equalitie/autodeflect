#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include "conf.h"

int need_finish = FALSE;

void finish_handler(int sig)
{
	need_finish = TRUE;

	printf("Caught signal - need to finish\n");

	signal(sig,SIG_IGN);
}
