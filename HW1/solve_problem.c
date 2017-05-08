#include "myheader.h"

#define LED(i) (1 << (8-i))

#define TIMER       30
#define PLUS         0
#define MUL          1

/* TEXT LCD */
#define LINE_BUFF   16
#define MAX_STRING   8

#define ERASE        6
#define SUBMIT      48
#define ZERO       384

extern unsigned char BLANK[10];
extern unsigned char O[10];
extern unsigned char X[10];

extern int CURRENT_MODE;
extern int QUIT_FLAG;

extern int input_Q_id;
extern int output_Q_id;

void SOLVE_PROBLEM_MODE(void)
{
	srand((unsigned int)time(NULL));

	/* Variable for Receiver Message */
	msg_buf_int    key_buf;
	msg_buf_int    switch_buf;

	/* Variable for Send Message */
	msg_buf_int    fnd_buf;
	msg_buf_int    led_buf;
   	msg_buf_char   lcd_buf;
	msg_buf_array  dot_buf;

	/* Variable for SOLVE_PROBLEM_MODE */
	int prev_sec = 0, cur_sec;
	int i, idx, loc = 0, timer = 15;
	int rand_num1, rand_num2, operator; 
	int answer, value; 
	int score = 0, led_on = 0;
	int break_flag = FALSE;
	char text1[LINE_BUFF] = {'\0'};
	char text2[LINE_BUFF] = {'\0'};
	unsigned char decimal[10] = 
	{
		'0', 
		'1', '2', '3', 
		'4', '5', '6', 
		'7', '8', '9'
	};

	/* Determine Message Type */
	fnd_buf.mtype = FND_OUTPUT;
	led_buf.mtype = LED_OUTPUT;
	lcd_buf.mtype = LCD_OUTPUT;
	dot_buf.mtype = DOT_OUTPUT;

	/* Solve 8 problems */
	for(i=1; i<9; i++)
	{
		if(break_flag == TRUE)
			break;

		memset(text1, '\0', LINE_BUFF);
		memset(text2, '\0', LINE_BUFF);
		text2[0] = ' '; loc = 0;

		/* Create 1~9 random number */
		rand_num1 = (rand() % 9) + 1;
		rand_num2 = (rand() % 9) + 1;
		/* PLUS or MUL */
		operator  = (rand() % 2);	

		/* Write problem */
		if(operator == PLUS)
		{
			answer = rand_num1 + rand_num2;
			sprintf(text1, "%d+%d=?", rand_num1, rand_num2);
		}
		else if(operator == MUL)
		{
			answer = rand_num1 * rand_num2;
			sprintf(text1, "%d*%d=?", rand_num1, rand_num2);		
		}

		/* Send Message to Initialize FND Device */
		fnd_buf.mcontent = timer;
		msgsnd(output_Q_id, &fnd_buf, sizeof(fnd_buf), IPC_NOWAIT);

		/* Send Message to Initialize LED Device */
		led_buf.mcontent = led_on;
		msgsnd(output_Q_id, &led_buf, sizeof(led_buf), IPC_NOWAIT);

		/* Send Message to Initialize LCD Device */
		memcpy(lcd_buf.mtext, text1, LINE_BUFF);
		memcpy(lcd_buf.mtext + LINE_BUFF, text2, LINE_BUFF);
		msgsnd(output_Q_id, &lcd_buf, sizeof(lcd_buf), IPC_NOWAIT);

		/* Send Message to Initialize DOT Device */
		memcpy(dot_buf.marray, BLANK, sizeof(BLANK));
		msgsnd(output_Q_id, &dot_buf, sizeof(dot_buf), IPC_NOWAIT);


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
						CURRENT_MODE = CLOCK;
						break;

					case VOL_DOWN:
						CURRENT_MODE = DRAW_BOARD;
						break;
				}
				
				break_flag = TRUE;

				break;
			}

			/* If there are Message from Switch Device */
			if(msgrcv(input_Q_id, &switch_buf, sizeof(switch_buf), SWITCH_INPUT, IPC_NOWAIT) > 0)
			{
				/* Erase last Input */
				if(switch_buf.mcontent == ERASE)
				{
					if(loc > 0)
						loc--;

					text2[loc] = ' ';
				}
				/* Submit current Input */
				else if(switch_buf.mcontent == SUBMIT)
				{
					value = atoi(text2);

					printf("P%d - answer : %d   |   value : %d\n", i, answer, value);

					if(value == answer)
					{
						score ++;
						led_on |= LED(i);

						dot_buf.mtype = DOT_OUTPUT;
						memcpy(dot_buf.marray, O, sizeof(O));
						msgsnd(output_Q_id, &dot_buf, sizeof(dot_buf), IPC_NOWAIT);
					}
					else
					{
						msg_buf_array dot_buf1;
						dot_buf1.mtype = DOT_OUTPUT;
						memcpy(dot_buf1.marray, X, sizeof(X));
						msgsnd(output_Q_id, &dot_buf1, sizeof(dot_buf1), IPC_NOWAIT);
					}

					led_buf.mcontent = led_on;
					msgsnd(output_Q_id, &led_buf, sizeof(led_buf), IPC_NOWAIT);

					sleep(1);

					break;
				}
				/* Input 0 */
				else if(switch_buf.mcontent == ZERO)
				{
					text2[loc] = decimal[0];

					if(loc < MAX_STRING - 1)
						loc++;
				}
				/* Input 1~9 */
				else
				{
					/* Convert Switch Input to index */
					switch (switch_buf.mcontent)
					{
						case SW1: idx = 1; break;
						case SW2: idx = 2; break;
						case SW3: idx = 3; break;
						case SW4: idx = 4; break;
						case SW5: idx = 5; break;
						case SW6: idx = 6; break;
						case SW7: idx = 7; break;
						case SW8: idx = 8; break;
						case SW9: idx = 9; break;
						default: continue;
					}		
					
					text2[loc] = decimal[idx];

					if(loc < MAX_STRING - 1)
						loc++;
				}

				/* Send Change point to LCD Device */
				lcd_buf.mtype = LCD_OUTPUT;
				memcpy(lcd_buf.mtext, text1, LINE_BUFF);
				memcpy(lcd_buf.mtext + LINE_BUFF, text2, LINE_BUFF);
				msgsnd(output_Q_id, &lcd_buf, sizeof(lcd_buf), IPC_NOWAIT);		
			}

			cur_sec = time(NULL) & 1;

			if(cur_sec != prev_sec)
			{
				prev_sec = cur_sec;

				/* Send change point to LED Device */
				(fnd_buf.mcontent) --;
				msgsnd(output_Q_id, &fnd_buf, sizeof(fnd_buf), IPC_NOWAIT);

				/* Show time out message*/
				if(fnd_buf.mcontent == 0)
				{
					memcpy(text1, "Time", 5); 
					memcpy(text2, "Out!", 5);

					lcd_buf.mtype = LCD_OUTPUT;
					memcpy(lcd_buf.mtext, text1, LINE_BUFF);
					memcpy(lcd_buf.mtext + LINE_BUFF, text2, LINE_BUFF);
					msgsnd(output_Q_id, &lcd_buf, sizeof(lcd_buf), IPC_NOWAIT);						

					sleep(2);
					break;
				}
			}	
		}
	}

	if(break_flag == FALSE)
	{
		/* Show Score */
		memcpy(text1, "Score", 6); 
		sprintf(text2, ": %d", score);

		lcd_buf.mtype = LCD_OUTPUT;
		memcpy(lcd_buf.mtext, text1, LINE_BUFF);
		memcpy(lcd_buf.mtext + LINE_BUFF, text2, LINE_BUFF);
		msgsnd(output_Q_id, &lcd_buf, sizeof(lcd_buf), IPC_NOWAIT);			

		sleep(2);
	}
}
