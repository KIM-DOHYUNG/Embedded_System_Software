#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <linux/input.h>
#include <termios.h>
#include <signal.h>
#include <time.h>

#define TRUE          1
#define FALSE         0

/* QUEUE KEY */
#define INPUT         1
#define OUTPUT        2

/* MSG TYPE */
#define KEY_INPUT    1
#define SWITCH_INPUT  2
#define FND_OUTPUT    3
#define LED_OUTPUT    4  
#define LCD_OUTPUT    5
#define DOT_OUTPUT    6

/* MODE */
#define CLOCK         1
#define COUNTER       2
#define TEXT_EDITOR   3
#define DRAW_BOARD    4
#define SOLVE_PROBLEM 5

/* KEY */
#define BACK        158
#define PROG        116
#define VOL_UP      115
#define VOL_DOWN    114

/* SWITCH */
#define SW1      1 
#define SW2      1 << 1
#define SW3      1 << 2
#define SW4      1 << 3
#define SW5      1 << 4
#define SW6      1 << 5
#define SW7      1 << 6
#define SW8      1 << 7
#define SW9      1 << 8


typedef struct msg_int
{
	long mtype;
	int mcontent;
}msg_buf_int;

typedef struct msg_char
{
	long mtype;
	char mtext[32];
}msg_buf_char;

typedef struct msg_array
{
	long mtype;
	unsigned char marray[10];
}msg_buf_array;


int readkey(int dev);
int push_switch(int dev);

void fnd(int dev, int number);
void led(int dev, int number);
void lcd(int dev, char *text);
void dot(int dev, unsigned char *array);

void CLOCK_MODE(void);
void COUNTER_MODE(void);
void TEXT_EDITOR_MODE(void);
void DRAW_BOARD_MODE(void);
void SOLVE_PROBLEM_MODE(void);
