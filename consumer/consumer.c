#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

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

	n_items = atoi(argv[1]);
	name = argv[2];
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
				fprintf(stderr, RED "Consumer %s: buffer empty, no producers available\n" RESET, name);
				flag = true;
				break;
			default:
				printf(BLU "Consumer %s: read: %s\n" RESET, name, buf);
				break;
		}

        if(flag)
			break;
		//sleep(1);
	}

	printf(GRN "Consumer %s: total number of items read: %d\n" RESET, name, i);

    // close the scull_buffer
    close(dev);
    exit(0);
}
