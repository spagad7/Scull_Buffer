#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#define BUFFER_SIZE 32

#define BLU   "\x1B[34m"
#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define RESET "\x1B[0m"

int main(int argc, char * argv[])
{
	int result, dev, n_items, i=0;
	bool flag = false;
	char *color;
	char buf[BUFFER_SIZE];

	// if incorrect number of arguments are passed, terminate
	if( argc != 3 )
	{
		printf(RED "Usage: producer num_items color\n" RESET);
		exit(1);
	}

	// get n_items and color
	n_items = atoi(argv[1]);
	color = argv[2];
	// open log file
	char *filename = malloc(strlen("producer_") + strlen(color) + strlen(".txt") + 1);
	strcpy(filename, "producer_");
	strcat(filename, color);
	strcat(filename, ".txt");
	FILE *fp;
	fp = fopen(filename, "w");

	// open scull_buffer
	dev = open("/dev/scull", O_WRONLY);
	if (dev == -1)
	{
		fprintf(stderr, RED "Producer: %s, failed to open scull_buffer\n" RESET, color);
		return -1;
	}

	sleep(1);

	for(; i<n_items; i++) {
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "%s %d",color,i+1);
		result =  write(dev, buf, BUFFER_SIZE);
		switch (result)
		{
			case -1:
				perror("Producer failed to write: ");
				break;
			case 0:
				//fprintf(stderr, RED "Buffer full, no consumers available\n" RESET);
				flag = true;
				break;
			default:
				//printf(BLU "Producer %s: wrote: %s\n" RESET, color, buf);
				fprintf (fp, "Producer %s: wrote: %s; size: %d bytes\n", color, buf, (int)strlen(buf));
				break;
		}
        if(flag)
			break;
	}

	printf(GRN "Producer %s: total number of items produced: %d\n" RESET, color, i);
	fprintf (fp, "Producer %s: total number of items produced: %d\n", color, i);
	fclose(fp);

    // close the scull_buffer
	close(dev);
	exit (0);
}
