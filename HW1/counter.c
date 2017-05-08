#include "myheader.h"

#define SYSTEM_2   1
#define SYSTEM_10  2
#define SYSTEM_8   3
#define SYSTEM_4   4

/* LED */
#define LED1     128
#define LED2      64
#define LED3      32
#define LED4      16

extern int CURRENT_MODE;
extern int QUIT_FLAG;

extern int input_Q_id;
extern int output_Q_id;

void COUNTER_MODE(void)
{
	/* Variable for Receive Message */
	msg_buf_int key_buf;
	msg_buf_int switch_buf;

	/* Variable for Send Message */
	msg_buf_int fnd_buf;
	msg_buf_int led_buf;

	/* Variable for Counter Mode */
	int number_system = SYSTEM_10, i;
	int number = 0, convert_number = 0;	
	int convert[4];

	/* Determine Message Type */	
	fnd_buf.mtype = FND_OUTPUT;
	led_buf.mtype = LED_OUTPUT;

	/* Send Message to Initialize FND Device */
	fnd_buf.mcontent = 0;
	msgsnd(output_Q_id, &fnd_buf, sizeof(fnd_buf), IPC_NOWAIT);

	/* Send Message to Initialize LED Device */
	led_buf.mcontent = LED2;
	msgsnd(output_Q_id, &led_buf, sizeof(led_buf), IPC_NOWAIT);

	while(1)			
	{
		/* If there are Message from Readkey Device */
		if(msgrcv(input_Q_id, &key_buf, sizeof(key_buf), KEY_INPUT, IPC_NOWAIT) > 0)
		{	
			/* Change Mode or Exit Program */
			switch (key_buf.mcontent)
			{
				case BACK:
					QUIT_FLAG = TRUE;
					break;

				case VOL_UP:
					CURRENT_MODE = TEXT_EDITOR;
					break;

				case VOL_DOWN:
					CURRENT_MODE = CLOCK;
					break;
			}

			break;
		}
		
		/* If there are Message from Switch Device */
		if(msgrcv(input_Q_id, &switch_buf, sizeof(switch_buf), SWITCH_INPUT, IPC_NOWAIT) > 0)
		{
			if(switch_buf.mcontent == SW1)
			{
				number_system = (number_system % 4) + 1;
				if(number_system == SYSTEM_2)
					led_buf.mcontent = LED1;
				else if(number_system == SYSTEM_10) 
					led_buf.mcontent = LED2;
				else if(number_system == SYSTEM_8)  
					led_buf.mcontent = LED3;
				else if(number_system == SYSTEM_4)	 
					led_buf.mcontent = LED4;	

				/* Send change point to LED Device */
				msgsnd(output_Q_id, &led_buf, sizeof(led_buf), IPC_NOWAIT);
			}
			else
			{
				/* 100 digit */
				if(switch_buf.mcontent == SW2)
				{
					switch (number_system)
					{
						case SYSTEM_2:   number +=   4;  break;
						case SYSTEM_10:  number += 100;  break;
						case SYSTEM_8:   number +=  64;  break;
						case SYSTEM_4:   number +=  16;  break;
					}
				}
				/* 10 digit */
				else if(switch_buf.mcontent == SW3)
				{
					switch (number_system)
					{
						case SYSTEM_2:   number +=   2;  break;
						case SYSTEM_10:  number +=  10;  break;
						case SYSTEM_8:   number +=   8;  break;
						case SYSTEM_4:   number +=   4;  break;
					}
				}
				/* 1 digit */
				else if(switch_buf.mcontent == SW4)
					number += 1;
			}
	
			number %= 1000;

			/* Convert number to current number_system */
			if(number_system == SYSTEM_2)
			{
		    	convert[0] = (number / 8) % 2; 
		    	convert[1] = (number / 4) % 2;
		    	convert[2] = (number % 4) / 2;
		    	convert[3] =  number % 2;
			}
			else if(number_system == SYSTEM_10)
			{
				convert[0] = 0;
		    	convert[1] = (number / 100);
		    	convert[2] = (number % 100) / 10;
		    	convert[3] =  number % 10;
			}
			else if(number_system == SYSTEM_8)
			{
				convert[0] = (number / 512) % 8;
				convert[1] = (number / 64)  % 8;
				convert[2] = (number % 64)  / 8;
				convert[3] =  number % 8;
			}
			else if(number_system == SYSTEM_4)
			{
				convert[0] = (number / 64)  % 4;
				convert[1] = (number / 16)  % 4;
				convert[2] = (number % 16)  / 4;
				convert[3] =  number % 4;
			}	

			convert_number = (convert[0] * 1000) + \
							 (convert[1] * 100)  + \
							 (convert[2] * 10)   + \
							 (convert[3] * 1);

			/* Send change point to FND Device */
			fnd_buf.mcontent = convert_number;
			msgsnd(output_Q_id, &fnd_buf, sizeof(fnd_buf), IPC_NOWAIT);
		}
	}
}
