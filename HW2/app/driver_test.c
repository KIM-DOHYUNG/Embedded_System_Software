#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <memory.h>
#include <syscall.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/ioctl.h>

#define SYSCALL_PACK_PARAM  376

#define DEVICE_MAJOR        242
#define DEVICE_NAME         "/dev/dev_driver"

#define IOCTL_WRITE _IOW(DEVICE_MAJOR, 0, unsigned long)


/********** Define function **********/
int check_argument(char *argv[]);


/********** Define Struct **********/
struct param_info
{
	int start_location;
	int start_value;
	int time_interval;
	int timer_count;
};


/********** Main Start **********/
int main(int argc, char *argv[])
{
	int i;
	int fd;
	struct param_info param;
	unsigned long packed_value = 0;

	/* If arguements were invalid exit program */
	if(check_argument(argv) == -1)
	{
		printf("Invalid Arguments!\n");
		return 0;
	}

	/* Assign arguements to each Struct memnet */
	for(i=0; i<4; i++)
	{
		argv[3][i] -= '0';

		if(argv[3][i] != 0)
		{
			param.start_location  =  i+1;
			param.start_value     =  argv[3][i];

			break; 
		}
	}

	param.time_interval  =  atoi(argv[1]);
	param.timer_count    =  atoi(argv[2]);

	/* Make packed parameter value using system call */
	syscall(SYSCALL_PACK_PARAM, &param, &packed_value);

	/* Open integrated device driver */
	fd = open(DEVICE_NAME, O_RDWR);

	if(fd < 0)
	{
		perror("Device Open Error");
		exit(1);
	}

	/* Write to device using ioctl */
	ioctl(fd, IOCTL_WRITE, &packed_value);
	
	close(fd);

	return 0;
}

/* Check all arguements */
int check_argument(char *argv[])
{
	int i;
	int time_interval;
	int timer_count;
	int zero_count = 0;


	/* Check arguement 1 */
	if(argv[1] == NULL || argv[1][0] == '0')
		return -1;

	for(i=0; i<strlen(argv[1]); i++)
	{
		if(isdigit(argv[1][i]) == 0)
			return -1;
	}

	time_interval = atoi(argv[1]);
	if(time_interval < 1 || time_interval > 100)
		return -1;



	/* Check arguement 2 */
	if(argv[2] == NULL || argv[2][0] == '0')
		return -1;

	for(i=0; i<strlen(argv[2]); i++)
	{
		if(isdigit(argv[2][i]) == 0)
			return -1;
	}

	timer_count = atoi(argv[2]);
	if(timer_count < 1 || timer_count > 100)
		return -1;



	/* Check arguement 3 */
	if(argv[3] == NULL || strlen(argv[3]) != 4)
		return -1;

	for(i=0; i<strlen(argv[3]); i++)
	{
		if(argv[3][i] == '9' || isdigit(argv[3][i]) == 0) 
			return -1;

		if(argv[3][i] == '0')
			zero_count++;
	}

	if(zero_count != 3)
		return -1;



	/* Check if other arguements were exist */
	if(argv[4] != NULL)
		return -1;
	

	/* If all arguements were correct */
	return 1;
}

