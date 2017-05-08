#include "myheader.h"

/* TEXT LCD */
#define MAX_BUFF     32
#define LINE_BUFF    16

void fnd(int dev, int number)
{
	unsigned char data[4];
	
	memset(data,0,sizeof(data));

	sprintf(data, "%04d", number);

	data[0] -= 0x30;
	data[1] -= 0x30;
	data[2] -= 0x30;
	data[3] -= 0x30;

	write(dev,&data,4);
}

void led(int dev, int number)
{
	unsigned char data;

	data = (unsigned char)number;

	write(dev,&data,1);
}

void lcd(int dev, char *text)
{
	int str_size;	
	char text1[LINE_BUFF];
	char text2[LINE_BUFF];
	char string[MAX_BUFF];

	memcpy(text1, text, LINE_BUFF);
	memcpy(text2, text + LINE_BUFF, LINE_BUFF);

	memset(string,0,sizeof(string));
	
	str_size=strlen(text1);
	if(str_size>0) 
	{
		strncat(string,text1,str_size);
		memset(string+str_size,' ',LINE_BUFF-str_size);
	}

	str_size=strlen(text2);
	if(str_size>0) 
	{
		strncat(string,text2,str_size);
		memset(string+LINE_BUFF+str_size,' ',LINE_BUFF-str_size);
	}

	write(dev,string,MAX_BUFF);	
}

void dot(int dev, unsigned char *array)
{
	unsigned char buf[10];

	memcpy(buf, array, 10);
	write(dev,buf, sizeof(buf));
}

