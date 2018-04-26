#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUFF_SIZE 32

#define BLU   "\x1B[34m"
#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define RESET "\x1B[0m"

int main(int argc, char * argv[]) {

	if( argc != 3 ){
		printf(RED "usage: producer number-of-items color\n" RESET);
		exit (1);
	}

	int total_num_item = atoi(argv[1]);
	char *base_color = argv[2];

	char buf[BUFF_SIZE];
	int scull;

	scull = open("/dev/scull", O_WRONLY);
	if (scull == -1) {
		perror("PRODUCER: Open failed: ");
	}

	int iters = 0;
	int done = 0;
	int written;
	for(; iters<total_num_item; iters++) {
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "%s %d",base_color,iters+1);
		written =  write(scull, buf, BUFF_SIZE);
		switch ( written ) {
			case -1:
				perror("Producer write failed: ");
				break;
			case 0:
				fprintf(stderr, RED "Buffer full, no consumers available\n" RESET);
				done = 1;
				break;
			default:
				printf(BLU "Producer wrote: %s\n" RESET, buf);
				;
		}

        if (done)
			break;
		sleep(2);
	}

	printf(GRN "Total number of Items produced by producer %s : %d\n" RESET, base_color, iters);

    // close the scullbuffer
	close(scull);
	exit (0);
}
