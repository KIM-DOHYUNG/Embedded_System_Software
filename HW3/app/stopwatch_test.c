#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#define DEVICE_NAME "/dev/stopwatch"


int main(void)
{
	int fd;
	char data[4] = {0, };

	fd = open(DEVICE_NAME, O_RDWR);

	if(fd < 0)
	{
		perror("Device Open Error");
		exit(1);
	}

	/* Initiate fnd_device */
	write(fd, &data, 4);

	close(fd);

	return 0;
}
