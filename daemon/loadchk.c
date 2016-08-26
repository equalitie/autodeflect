#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "conf.h"

int load_check(double *current_load)
{
	FILE *fp;
	char loadavg[50];
	int bytes = 0;
	double avg_1=0, avg_5=0, avg_15=0; // From procps

	if ((fp = fopen("/proc/loadavg", "r")) == NULL)
		return FALSE;

	if(!(bytes = fread(loadavg, sizeof(char), 49, fp))) {
		printf("Error reading load average\n");
		fclose(fp);
		return FALSE;
	}

	loadavg[bytes] = 0;
		
	fclose(fp);

	// From procps
	if (sscanf(loadavg, "%lf %lf %lf", &avg_1, &avg_5, &avg_15) < 3) {
		printf("Error getting load average\n");
		return FALSE;
	}

	*current_load = avg_1;

	return TRUE;
}

/******************************************************************************/
/******************************************************************************/
