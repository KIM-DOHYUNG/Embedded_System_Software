#include "myheader.h"

#define IS_ALPHA        0
#define IS_DIGIT        1

/* TEXT LCD */
#define LINE_BUFF      16
#define MAX_STRING      8

#define CLEAR_LCD       6
#define CHANGE_INPUT   48
#define SPACE         384

extern unsigned char ALPHA[10];
extern unsigned char DIGIT[10];

extern int CURRENT_MODE;
extern int QUIT_FLAG;

extern int input_Q_id;
extern int output_Q_id;

void TEXT_EDITOR_MODE(void)
{
	/* Variable for Receiver Message */
	msg_buf_int    key_buf;
	msg_buf_int    switch_buf;

	/* Variable for Send Message */
	msg_buf_int    fnd_buf;
   	msg_buf_char   lcd_buf;
	msg_buf_array  dot_buf;	

	/* Variable for Text Editor Mode */
	int  prev_sw = SW2 | SW3;
	int  idx, loc = 0, count = 0;
	int  cur_input = IS_ALPHA;
	char text1[LINE_BUFF] = {'\0'};
	char text2[LINE_BUFF] = {'\0'};
	unsigned char decimal[10] = 
	{
		'\0', 
		'1', '2', '3', 
		'4', '5', '6', 
		'7', '8', '9'
	};
	unsigned char alphabet[10][3] = 
	{
		{'\0', '\0', '\0'},
		{'.', 'Q', 'Z'}, {'A', 'B', 'C'}, {'D', 'E', 'F'},
		{'G', 'H', 'I'}, {'J', 'K', 'L'}, {'M', 'N', 'O'},
		{'P', 'R', 'S'}, {'T', 'U', 'V'}, {'W', 'X', 'Y'}
	};

	/* Determine Message Type */
	fnd_buf.mtype = FND_OUTPUT;
	lcd_buf.mtype = LCD_OUTPUT;
	dot_buf.mtype = DOT_OUTPUT;

	/* Send Message to Initialize FND Device */
	fnd_buf.mcontent = 0;
	msgsnd(output_Q_id, &fnd_buf, sizeof(fnd_buf), IPC_NOWAIT);

	/* Send Message to Initialize LCD Device */
	text1[0] = ' '; text2[0] = ' ';
	memcpy(lcd_buf.mtext, text1, LINE_BUFF);
	memcpy(lcd_buf.mtext + LINE_BUFF, text2, LINE_BUFF);
	msgsnd(output_Q_id, &lcd_buf, sizeof(lcd_buf), IPC_NOWAIT);

	/* Send Message to Initialize DOT Device */
	memcpy(dot_buf.marray, ALPHA, sizeof(ALPHA));
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
					CURRENT_MODE = DRAW_BOARD;
					break;

				case VOL_DOWN:
					CURRENT_MODE = COUNTER;
					break;
			}

			break;
		}

		/* If there are Message from Switch Device */
		if(msgrcv(input_Q_id, &switch_buf, sizeof(switch_buf), SWITCH_INPUT, IPC_NOWAIT) > 0)
		{
			/* Clear LCD */
			if(switch_buf.mcontent == CLEAR_LCD)
			{
				memset(text1, '\0', LINE_BUFF);
				text1[0] = ' '; loc = 0;

				memcpy(lcd_buf.mtext, text1, LINE_BUFF);
				msgsnd(output_Q_id, &lcd_buf, sizeof(lcd_buf), IPC_NOWAIT);
			}
			/* Change Input (Alphabet , Digit) */
			else if(switch_buf.mcontent == CHANGE_INPUT)
			{
				if(cur_input == IS_ALPHA)	
				{
					cur_input = IS_DIGIT;
					memcpy(dot_buf.marray, DIGIT, sizeof(DIGIT));
				}
				else if(cur_input == IS_DIGIT)
				{
					cur_input = IS_ALPHA;
					memcpy(dot_buf.marray, ALPHA, sizeof(ALPHA));
				}	

				if(prev_sw == CLEAR_LCD)
					loc--;

				msgsnd(output_Q_id, &dot_buf, sizeof(dot_buf), IPC_NOWAIT);
			}
			/* Write space (' ') on LCD */
			else if(switch_buf.mcontent == SPACE)
			{
				if(loc < MAX_STRING -1)
				{
					if(prev_sw != CLEAR_LCD)
						loc++;
				}
				else
					memmove(text1, text1 + 1, MAX_STRING - 1);

				text1[loc] = ' ';	

				memcpy(lcd_buf.mtext, text1, LINE_BUFF);
				msgsnd(output_Q_id, &lcd_buf, sizeof(lcd_buf), IPC_NOWAIT);
			}
			/* Write Alphabet or Digit on LCD */
			else
			{
				/* Conver Switch input value to index */
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
				
				/* Write Alphabet */
				if(cur_input == IS_ALPHA)
				{
					/* Change Alphabet in same Switch */
					if(prev_sw == switch_buf.mcontent)
					{
						count ++;
						count %= 3;
					}
					else
					{
						count = 0;
				
						if(prev_sw != CLEAR_LCD)
						{
							/* If LCD is not FULL */
							if(loc < MAX_STRING - 1)
								loc++;
							/* If LCD is FULL */
							else
								memmove(text1, text1 + 1, MAX_STRING - 1);
						}
					}

					/* Make text to write LCD */
					text1[loc] = alphabet[idx][count];
				}
				/* Write Digit */
				else if(cur_input == IS_DIGIT)
				{
					/* If LCD is not FULL */
					if(loc < MAX_STRING - 1)
					{
						if(prev_sw != CLEAR_LCD)
							loc++;
					}
					/* If LCD is FULL */
					else
						memmove(text1, text1 + 1, MAX_STRING - 1);
		
					/* Make text to write LCD */
					text1[loc] = decimal[idx];
				}

				/* Send change point to LCD Device */
				memcpy(lcd_buf.mtext, text1, LINE_BUFF);
				msgsnd(output_Q_id, &lcd_buf, sizeof(lcd_buf), IPC_NOWAIT);
			}

			prev_sw = switch_buf.mcontent;

			/* Send change point to FND Device */
			(fnd_buf.mcontent) ++;
			(fnd_buf.mcontent) %= 10000;
			msgsnd(output_Q_id, &fnd_buf, sizeof(fnd_buf), IPC_NOWAIT);
		}
	}
}
