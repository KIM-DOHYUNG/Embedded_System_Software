#include "myheader.h"

#define FIELD_SIZE 10

#define MOVE_CURSOR(cursor) (cursor = (1 << (6 - col)))

extern unsigned char BLANK[10];
extern unsigned char FULL[10];

extern int CURRENT_MODE;
extern int QUIT_FLAG;

extern int input_Q_id;
extern int output_Q_id;

void DRAW_BOARD_MODE(void)
{
	/* Variable for Receive Message */
	msg_buf_int    key_buf;
	msg_buf_int    switch_buf;

	/* Variable for Send Message */
	msg_buf_int    fnd_buf;
	msg_buf_array  dot_buf;	

	/* Variable for Draw Board Mode */
	int blink_flag = TRUE;
	int reverse_flag = FALSE;
	long cur_sec, prev_sec = 0;
	int i, row = 0, col = 0;
	unsigned char cursor = 0;
	unsigned char prev_field[FIELD_SIZE] = {0,};
	unsigned char cur_field[FIELD_SIZE] = {0,};

	/* Determine Message Type */
	fnd_buf.mtype = FND_OUTPUT;
	dot_buf.mtype = DOT_OUTPUT;

	/* Send Message to Initialize FND Device */
	fnd_buf.mcontent = 0;
	msgsnd(output_Q_id, &dot_buf, sizeof(dot_buf), IPC_NOWAIT);

	/* Send Message to Initialize DOT Device */
	memcpy(dot_buf.marray, cur_field, FIELD_SIZE);
	msgsnd(output_Q_id, &dot_buf, sizeof(dot_buf), IPC_NOWAIT);

	/* Set cursor to current location */
	MOVE_CURSOR(cursor);
	
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
					CURRENT_MODE = SOLVE_PROBLEM;
					break;

				case VOL_DOWN:
					CURRENT_MODE = TEXT_EDITOR;
					break;
			}

			break;
		}

		/* If there are Message from Switch Device */
		if(msgrcv(input_Q_id, &switch_buf, sizeof(switch_buf), SWITCH_INPUT, IPC_NOWAIT) > 0)
		{
			switch (switch_buf.mcontent)
			{
				/* Initialize cursor location */
				case SW1:
					row = 0; col = 0;
					MOVE_CURSOR(cursor);
					
					/* Set all dot on when reverse_flag is True */
					if(reverse_flag == TRUE)
					{
						memcpy(prev_field, FULL, FIELD_SIZE);
						memcpy(cur_field, FULL, FIELD_SIZE);
						memcpy(dot_buf.marray, FULL, FIELD_SIZE);
					}

					/* Set all dot off when reverse_flag is False */
					else if(reverse_flag == FALSE)
					{
						memcpy(prev_field, BLANK, FIELD_SIZE);
						memcpy(cur_field, BLANK, FIELD_SIZE);
						memcpy(dot_buf.marray, BLANK, FIELD_SIZE);
					}

					/* Send change point to DOT Device */
					msgsnd(output_Q_id, &dot_buf, sizeof(dot_buf), IPC_NOWAIT);
					break;

				/* Move cursor up */
				case SW2:
					if(row > 0)
					{
						row--;
						memcpy(prev_field, cur_field, FIELD_SIZE);
					}	
					break;

				/* On, Off cursor */
				case SW3:
					if(blink_flag == TRUE)
						blink_flag = FALSE;
					else if(blink_flag == FALSE)
						blink_flag = TRUE;
					break;

				/* Move cursor left */
				case SW4:
					if(col > 0)
					{
						col--;
						MOVE_CURSOR(cursor);

						memcpy(prev_field, cur_field, FIELD_SIZE);
					}
					break;

				/* Mark current cursor location */ 
				case SW5:
					if(reverse_flag == TRUE)
						cur_field[row] ^= cursor;
					else if(reverse_flag == FALSE)
						cur_field[row] |= cursor;
					break;

				/* Move cursor rigth */
				case SW6:
					if(col < 6)
					{
						col++;
						MOVE_CURSOR(cursor);

						memcpy(prev_field, cur_field, FIELD_SIZE);
					}
					break;

				/* Erase current field */
				case SW7:
					/* Set all dot on when reverse_flag is True */
					if(reverse_flag == TRUE)
					{
						memcpy(prev_field, FULL, FIELD_SIZE);
						memcpy(cur_field, FULL, FIELD_SIZE);
						memcpy(dot_buf.marray, FULL, FIELD_SIZE);
					}
					/* Set all dot off when reverse_flag is False */
					else if(reverse_flag == FALSE)
					{
						memcpy(prev_field, BLANK, FIELD_SIZE);
						memcpy(cur_field, BLANK, FIELD_SIZE);
						memcpy(dot_buf.marray, BLANK, FIELD_SIZE);
					}

					/* Send change point to DOT Device */
					msgsnd(output_Q_id, &dot_buf, sizeof(dot_buf), IPC_NOWAIT);
					break;

				/* Move cursor down */
				case SW8:
					if(row < 9)
					{
						row++;
						memcpy(prev_field, cur_field, FIELD_SIZE);
					}
					break;

				/* Reverse current field */
				case SW9:
					if(reverse_flag == TRUE)
						reverse_flag = FALSE;
					else if(reverse_flag == FALSE)
						reverse_flag = TRUE;

					/* Flip bit to reverser field */
					for(i=0;i<=FIELD_SIZE;i++)
					{
						prev_field[i] = ~(prev_field[i]);
						cur_field[i] = ~(cur_field[i]);
					}
					break;

				default:
					continue;
			}

			/* Send change point to FND Device */ 
			(fnd_buf.mcontent) ++;
			(fnd_buf.mcontent) %= 10000;	
			msgsnd(output_Q_id, &fnd_buf, sizeof(fnd_buf), IPC_NOWAIT);
		}

		cur_sec = time(NULL) & 1;

		/* Reflect field at DOT Device every 1 second */
		if(cur_sec != prev_sec)
		{
			prev_sec = cur_sec;		
			
			if(blink_flag == TRUE)
			{
				/* Blink cursor every 1 second */
				if(cur_sec)
				{
					/* Variable for reflect field */
					unsigned char temp_field[10] = {0,};

					memcpy(temp_field, prev_field, FIELD_SIZE);

					/* Reflect cursor on temp field */
					if(reverse_flag == TRUE)
						temp_field[row] ^= cursor;
					else if(reverse_flag == FALSE)
						temp_field[row] |= cursor;

					memcpy(dot_buf.marray, temp_field, FIELD_SIZE);
				}
				else
					memcpy(dot_buf.marray, prev_field, FIELD_SIZE);
			}
			else
				memcpy(dot_buf.marray, cur_field, FIELD_SIZE);

			/* Send change point to DOT Device */
			msgsnd(output_Q_id, &dot_buf, sizeof(dot_buf), IPC_NOWAIT);
		}
	}
}

