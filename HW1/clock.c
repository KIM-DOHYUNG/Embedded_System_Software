#include "myheader.h"

/* LED */
#define LED1     128
#define LED2      64
#define LED3      32
#define LED4      16

extern int CURRENT_MODE;
extern int QUIT_FLAG;

extern int input_Q_id;
extern int output_Q_id;

void CLOCK_MODE(void)
{
	/* Variable for Receive Message */
	msg_buf_int key_buf;
	msg_buf_int switch_buf;

	/* Variable for Send Message */
	msg_buf_int fnd_buf;
	msg_buf_int led_buf;

	/* Variable for Clock Mode */
	time_t t;
	struct tm *cur_time;
	long prev_sec = 0, cur_sec;
	int adjust_flag = FALSE;

	/* Determine Message Type */
	fnd_buf.mtype = FND_OUTPUT;
	led_buf.mtype = LED_OUTPUT;

	/* Send Message to Initialize FND Device */
	time(&t);
	cur_time = localtime(&t);
	fnd_buf.mcontent = (cur_time->tm_hour * 100) + (cur_time->tm_min);
	msgsnd(output_Q_id, &fnd_buf, sizeof(fnd_buf), IPC_NOWAIT);

	/* Send Message to Initialize LED Device */
	led_buf.mcontent = LED1;
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
					CURRENT_MODE = COUNTER;
					break;

				case VOL_DOWN:
					CURRENT_MODE = SOLVE_PROBLEM;
					break;
			}

			break;
		}

		/* If there are Message from Switch Device */
		if(msgrcv(input_Q_id, &switch_buf, sizeof(switch_buf), SWITCH_INPUT, IPC_NOWAIT) > 0)
		{
			/* Adjust time and save */ 
			if(switch_buf.mcontent == SW1)	
			{
				if(adjust_flag == FALSE)
					adjust_flag = TRUE;
				else if(adjust_flag == TRUE)
				{
					adjust_flag = FALSE;
					led_buf.mcontent = LED1;
					msgsnd(output_Q_id, &led_buf, sizeof(led_buf), IPC_NOWAIT);
				}
			}
			else
			{
				if(adjust_flag == TRUE)
				{
					switch (switch_buf.mcontent)
					{
						/* Reset to Board time */
						case SW2:
							time(&t);
							cur_time = localtime(&t);
							break;
						
						/* Increase hour */
						case SW3:
							(cur_time->tm_hour) ++;
							(cur_time->tm_hour) %= 24;
							break;

						/* Increase minute */
						case SW4:
							(cur_time->tm_min) ++;
							(cur_time->tm_min) %= 60;
							break;

						/* Handle invalid Switch Input */  
						default:
							continue;
					}

					/* Send change point to FND Device */
					fnd_buf.mcontent = (cur_time->tm_hour * 100) + (cur_time->tm_min);
					msgsnd(output_Q_id, &fnd_buf, sizeof(fnd_buf), IPC_NOWAIT);
				}
			}
		}

		/* If adjust time mode, blink LED Device */		
		if(adjust_flag == TRUE)
		{
			cur_sec = time(NULL) & 1;

			if(cur_sec != prev_sec)
			{
				prev_sec = cur_sec;

				/* Send change point to LED Device */
				led_buf.mcontent = (cur_sec) ? LED3 : LED4;
				msgsnd(output_Q_id, &led_buf, sizeof(led_buf), IPC_NOWAIT);
			}
		}
		/* Update time every 1 minute */
		else
		{
			if(time(NULL) % 60 == 0)
			{
				(cur_time->tm_min) ++;

				if(cur_time->tm_min == 60)
				{
					(cur_time->tm_hour) ++;
					(cur_time->tm_hour) %= 24;
				}

				(cur_time->tm_min) %= 60;

				/* Send change point to FND Device */
				fnd_buf.mcontent = (cur_time->tm_hour * 100) + (cur_time->tm_min);
				msgsnd(output_Q_id, &fnd_buf, sizeof(fnd_buf), IPC_NOWAIT);

				sleep(1);
			}
		}
	}
}
