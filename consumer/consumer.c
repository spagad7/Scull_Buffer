#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#define BLU   "\x1B[34m"
#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define RESET "\x1B[0m"
#define BUFFER_SIZE 32

int main(int argc, char * argv[])
{
	int n_items, dev, result, i=0;
	bool flag = false;
	char *name;
	char buf[BUFFER_SIZE];

	// if incorrect number of arguments are passed, terminate
	if( argc != 3 ){
		printf(RED "Usage: consumer num_items name\n" RESET);
		return -1;
	}

	// read n_items and name
	n_items = atoi(argv[1]);
	name = argv[2];

	// open log file
	char *filename = malloc(strlen("consumer_") + strlen(name) + strlen(".txt") + 1);
	strcpy(filename, "consumer_");
	strcat(filename, name);
	strcat(filename, ".txt");
	FILE *fp;
	fp = fopen(filename, "w");

	dev = open("/dev/scull", O_RDONLY);
	if (dev == -1) {
		fprintf(stderr, RED "Consumer %s: failed to open scull_buffer\n" RESET, name);
		return -1;
	}
	sleep(1);

	for(; i<n_items; i++)
	{
		result = read(dev, &buf, BUFFER_SIZE);
		switch (result)
		{
			case -1:
				fprintf(stderr, RED "Consumer %s: failed to read\n" RESET, name);
				break;
			case 0:
				//fprintf(stderr, RED "Consumer %s: buffer empty, no producers available\n" RESET, name);
				flag = true;
				break;
			default:
				//printf(BLU "Consumer %s: read: %s\n" RESET, name, buf);
				fprintf (fp, "Consumer %s: read: %s; size: %d bytes\n", name, buf, result);
				break;
		}

        if(flag)
			break;
	}

	printf(GRN "Consumer %s: total number of items read: %d\n" RESET, name, i);
	fprintf (fp, "Consumer %s: total number of items read: %d\n", name, i);
	fclose(fp);

    // close the scull_buffer
    close(dev);
    exit(0);
}
