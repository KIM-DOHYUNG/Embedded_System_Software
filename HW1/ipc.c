#include "myheader.h"

/* Device File Descriptor */
int dev_readkey;
int dev_switch;
int dev_fnd;
int dev_led;
int dev_lcd;
int dev_dot;

/* Control Mode */
int CURRENT_MODE = CLOCK;
int QUIT_FLAG = FALSE;

/* Message Queue ID */
int input_Q_id;
int output_Q_id;

/* Array for Dot Device */
unsigned char BLANK[10]  = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
unsigned char FULL[10]   = {0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f};
unsigned char ALPHA[10]  = {0x1c,0x36,0x63,0x63,0x63,0x7f,0x7f,0x63,0x63,0x63};
unsigned char DIGIT[10]  = {0x0c,0x1c,0x1c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x1e};
unsigned char O[10]      = {0x3e,0x7f,0x63,0x63,0x63,0x63,0x63,0x63,0x7f,0x3e};
unsigned char X[10]      = {0x63,0x66,0x3c,0x3c,0x18,0x18,0x3c,0x3c,0x66,0x63};


int main(void)
{
	/* Create Message Queue */
	input_Q_id  = msgget(INPUT, IPC_CREAT);
	output_Q_id = msgget(OUTPUT, IPC_CREAT);

	/* PID for Input, Output Process */
	pid_t input_pid;
	pid_t output_pid;

	/* Fork Input Process */
	input_pid = fork();

	/* Input Process */
	if(input_pid == 0)
	{
		/* Open Input Device */
		dev_readkey = open("/dev/input/event0", O_RDONLY|O_NONBLOCK);
		dev_switch = open("/dev/fpga_push_switch", O_RDWR|O_NONBLOCK);

		/* Variable for Send Message */
		msg_buf_int key_buf;
		msg_buf_int switch_buf;

		/* Determine Message Type */
		key_buf.mtype = KEY_INPUT;
		switch_buf.mtype = SWITCH_INPUT;
	
		while(1)
		{
			/* Store Readkey Input */
			key_buf.mcontent = readkey(dev_readkey);

			/* Send Valid Input */
			if(key_buf.mcontent > 0)
				msgsnd(input_Q_id, &key_buf, sizeof(key_buf), IPC_NOWAIT);

			/* Store Switch Input */
			switch_buf.mcontent = push_switch(dev_switch);

			/* Send Valid Input */
			if(switch_buf.mcontent > 0)
				msgsnd(input_Q_id, &switch_buf, sizeof(switch_buf), IPC_NOWAIT);
		}
	}
	else
	{
		/* Fork Output Process */
		output_pid = fork();

		/* Output Process */
		if(output_pid == 0)
		{
			/* Open Output Device */
			dev_fnd = open("/dev/fpga_fnd", O_RDWR);
			dev_led = open("/dev/fpga_led", O_RDWR);
			dev_lcd = open("/dev/fpga_text_lcd", O_WRONLY);
			dev_dot = open("/dev/fpga_dot", O_WRONLY);

			/* Variable for Receive Message */
			msg_buf_int    fnd_buf;
			msg_buf_int    led_buf;
			msg_buf_char   lcd_buf;
			msg_buf_array  dot_buf;

			while(1)
			{
				/* Receive FND Message and Write it to Device */ 
				if(msgrcv(output_Q_id, &fnd_buf, sizeof(fnd_buf), FND_OUTPUT, IPC_NOWAIT) > 0)
					fnd(dev_fnd, fnd_buf.mcontent);	
			
				/* Receive LED Message and Write it to Device */ 
				if(msgrcv(output_Q_id, &led_buf, sizeof(led_buf), LED_OUTPUT, IPC_NOWAIT) > 0)	
					led(dev_led, led_buf.mcontent);

				/* Receive LCD Message and Write it to Device */ 
				if(msgrcv(output_Q_id, &lcd_buf, sizeof(lcd_buf), LCD_OUTPUT, IPC_NOWAIT) > 0)
					lcd(dev_lcd, lcd_buf.mtext);

				/* Receive DOT Message and Write it to Device */ 
				if(msgrcv(output_Q_id, &dot_buf, sizeof(dot_buf), DOT_OUTPUT, IPC_NOWAIT) > 0)
					dot(dev_dot, dot_buf.marray);	
			}		
		}
		/* Main Process */
		else
		{
			/* Variable for Send Message */
			msg_buf_int    fnd_buf;
			msg_buf_int    led_buf;
			msg_buf_char   lcd_buf;
			msg_buf_array  dot_buf;

			/* Determine Message Type */
			fnd_buf.mtype = FND_OUTPUT;
			led_buf.mtype = LED_OUTPUT;
			lcd_buf.mtype = LCD_OUTPUT;
			dot_buf.mtype = DOT_OUTPUT;

			while(!QUIT_FLAG)
			{
				/* Send Message to Initialize FND Device */	
				fnd_buf.mcontent = 0;				
				msgsnd(output_Q_id, &fnd_buf, sizeof(fnd_buf), IPC_NOWAIT);

				/* Send Message to Initialize LED Device */	
				led_buf.mcontent = 0;
				msgsnd(output_Q_id, &led_buf, sizeof(led_buf), IPC_NOWAIT);

				/* Send Message to Initialize LCD Device */	
				memset(lcd_buf.mtext, '\0', sizeof(lcd_buf.mtext));
				msgsnd(output_Q_id, &lcd_buf, sizeof(lcd_buf), IPC_NOWAIT);

				/* Send Message to Initialize DOT Device */	
				memcpy(dot_buf.marray, BLANK, sizeof(BLANK));
				msgsnd(output_Q_id, &dot_buf, sizeof(dot_buf), IPC_NOWAIT);

				printf("CURRENT MODE : %d\n", CURRENT_MODE);

				switch (CURRENT_MODE)
				{
					/* Process Clock Mode*/
					case CLOCK:
						CLOCK_MODE();
						break;

					/* Process Counter Mode */
					case COUNTER:
						COUNTER_MODE();
						break;

					/* Process Text Editor Mode */
					case TEXT_EDITOR:
						TEXT_EDITOR_MODE();
						break;

					/* Process Draw Board Mode */
					case DRAW_BOARD:
						DRAW_BOARD_MODE();
						break;

					case SOLVE_PROBLEM:
						SOLVE_PROBLEM_MODE();
						break;
				}
			}
			
			/* Close Device */
			close(dev_readkey);
			close(dev_switch);
			close(dev_fnd);
			close(dev_led);
			close(dev_lcd);
			close(dev_dot);		
		
			/* Remove Message Queue */
			msgctl(input_Q_id, IPC_RMID, NULL);	
			msgctl(output_Q_id, IPC_RMID, NULL);

			/* Kill Input, Output Process */
			kill(input_pid, SIGKILL);
			kill(output_pid, SIGKILL);
		}
	}

	return 0;
}
