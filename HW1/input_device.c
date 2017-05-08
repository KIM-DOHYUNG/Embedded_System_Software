#include "myheader.h"

/* READKEY */
#define BUFF_SIZE    64
#define KEY_RELEASE   0
#define KEY_PRESS     1

/* SWITCh */
#define MAX_BUTTON    9

int readkey(int dev)
{
	struct input_event ev[BUFF_SIZE];
	int rd, size = sizeof (struct input_event);

	read(dev, ev, size * BUFF_SIZE);

	if(ev[0].value == KEY_PRESS)		
	{	
		ev[0].value = KEY_RELEASE;
		return ev[0].code;
	}
	else
		return -1;
}

int push_switch(int dev)
{
	int i;
	int buff_size, bit_mark = 0;
	unsigned char push_sw_buff[MAX_BUTTON];

	buff_size=sizeof(push_sw_buff);

	usleep(400000);
	read(dev, &push_sw_buff, buff_size);

	for(i=0;i<MAX_BUTTON;i++) 
		bit_mark |= (push_sw_buff[i] << i);

	return bit_mark;
}

