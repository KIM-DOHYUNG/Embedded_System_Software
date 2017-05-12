#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/timer.h>
#include <linux/memory.h>

#define TRUE   1
#define FALSE  0

#define IOM_FPGA_LED_ADDRESS        0x08000016     
#define IOM_FPGA_FND_ADDRESS        0x08000004       
#define IOM_FPGA_DOT_ADDRESS        0x08000210       
#define IOM_FPGA_TEXT_LCD_ADDRESS   0x08000090       

#define DEVICE_MAJOR                242
#define DEVICE_NAME                 "/dev/dev_driver"

#define IOCTL_WRITE _IOW(DEVICE_MAJOR, 0, int)


/*************** Define function ***************/
void     write_handler       (unsigned long arg);
void     write_to_led        (void);
void     write_to_fnd        (void);
void     write_to_dot        (void);
void     write_to_text_lcd   (void);

void     device_clear        (void);
int      device_open         (struct inode *minode, struct file *mfile);
int      device_release      (struct inode *minode, struct file *mfile);
long     device_ioctl        (struct file *mfile, unsigned int cmd, unsigned long arg);
ssize_t  device_write        (struct file *mfile, const char *gdata, size_t length, loff_t *off_what);


/*************** Define Struct ***************/
struct file_operations device_fops =
{
	.open            =  device_open,
	.release         =  device_release,
	.unlocked_ioctl  =  device_ioctl,
	.write           =  device_write,
};


/*************** Define Global Variable ***************/
struct timer_list timer;

/* Variable for write value to device */
int start_location    =  0;
int start_value       =  0;
int time_interval     =  0;
int timer_count       =  0;
int current_location  =  0;
int current_value     =  0;

int device_port_usage; 

/* Variable for device memory map */
unsigned char *iom_fpga_led_addr;
unsigned char *iom_fpga_fnd_addr;
unsigned char *iom_fpga_dot_addr;
unsigned char *iom_fpga_text_lcd_addr;

/* Variable for TEXT_LCD Device */
unsigned char ID[9]     =  "20121564";
unsigned char NAME[12]  =  "Dohyung Kim";
int line1_position      =  0; 
int line1_reverse_flag  =  FALSE;
int line2_position      =  0;
int line2_reverse_flag  =  FALSE;

/* Variable for DOT Device */
unsigned char dot_number[10][10] = 
{
	{0x3e,0x7f,0x63,0x73,0x73,0x6f,0x67,0x63,0x7f,0x3e}, // 0
	{0x0c,0x1c,0x1c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x1e}, // 1
	{0x7e,0x7f,0x03,0x03,0x3f,0x7e,0x60,0x60,0x7f,0x7f}, // 2
	{0xfe,0x7f,0x03,0x03,0x7f,0x7f,0x03,0x03,0x7f,0x7e}, // 3
	{0x66,0x66,0x66,0x66,0x66,0x66,0x7f,0x7f,0x06,0x06}, // 4
	{0x7f,0x7f,0x60,0x60,0x7e,0x7f,0x03,0x03,0x7f,0x7e}, // 5
	{0x60,0x60,0x60,0x60,0x7e,0x7f,0x63,0x63,0x7f,0x3e}, // 6
	{0x7f,0x7f,0x63,0x63,0x03,0x03,0x03,0x03,0x03,0x03}, // 7
	{0x3e,0x7f,0x63,0x63,0x7f,0x7f,0x63,0x63,0x7f,0x3e}, // 8
	{0x3e,0x7f,0x63,0x63,0x7f,0x3f,0x03,0x03,0x03,0x03}  // 9
};



/*************** Program Code ***************/

/* 
 * Write value to each device until timer_count is zero 
 * using kernel timer
 */
void write_handler(unsigned long timeout) 
{
	if((timer_count--) == 0)
	{
		device_clear();			
		return;
	}

	write_to_led();
	write_to_fnd();
	write_to_dot();
	write_to_text_lcd();

	current_value = (current_value % 8) + 1;

	if(current_value == start_value)
		current_location = (current_location % 4) + 1;

	timer.expires   =  get_jiffies_64() + (time_interval * HZ / 10);
	timer.function  =  write_handler;

	add_timer(&timer);
}

/* Write value to LED Device */
void write_to_led(void)
{
	unsigned short  led_value;

	led_value = 1 << (8 - current_value);
    outw(led_value, (unsigned int)iom_fpga_led_addr);
}

/* Write value to FND Device */
void write_to_fnd(void)
{
	unsigned short  fnd_value;

	fnd_value = current_value << ((4 - current_location) * 4);
	outw(fnd_value, (unsigned int)iom_fpga_fnd_addr);
}

/* Write value to DOT Device */
void write_to_dot(void)
{
	int i;
	unsigned char *dot_value;

	dot_value = (unsigned char *)(dot_number + current_value);
	for(i=0; i<10; i++)
		outw(dot_value[i], (unsigned int)iom_fpga_dot_addr + i * 2);	
}

/* Write value to TEXT_LCD Device */
void write_to_text_lcd(void)
{
	int i;
	unsigned char text_lcd_value[32];

	/* Copy Student ID and Student NAME */
	memset(text_lcd_value, ' ', 32);
	memcpy(text_lcd_value + line1_position,       ID,   sizeof(ID)   -  1);
	memcpy(text_lcd_value + line2_position + 16,  NAME, sizeof(NAME) -  1);

	for(i=0; i<32; i+=2)
		outw(text_lcd_value[i] << 8 | text_lcd_value[i+1], (unsigned int)iom_fpga_text_lcd_addr + i);

	/* Shift Student ID */
	if(line1_reverse_flag == FALSE)
	{
		if(line1_position + sizeof(ID) - 1 < 16)
			line1_position++;
		else
		{
			line1_reverse_flag = TRUE;
			line1_position--;
		}
	}
	else
	{
		if(line1_position > 0)
			line1_position--;
		else
		{
			line1_reverse_flag = FALSE;
			line1_position++;
		}
	}

	/* Shift Student NAME */
	if(line2_reverse_flag == FALSE)
	{
		if(line2_position + sizeof(NAME) - 1 < 16)
			line2_position++;
		else
		{
			line2_reverse_flag = TRUE;
			line2_position--;
		}
	}
	else
	{
		if(line2_position > 0)
			line2_position--;
		else
		{
			line2_reverse_flag = FALSE;
			line2_position++;
		}
	}
}

/* 
 * Clean all written value in device 
 * when timer_count is zero
 */
void device_clear(void)
{
	int i;

	start_location    =  0;
	start_value       =  0;
	time_interval     =  0;
	timer_count       =  0;
	
	current_location  =  0;
	current_value     =  0;

	outw(0x00, (unsigned int)iom_fpga_led_addr);
	outw(0x00, (unsigned int)iom_fpga_fnd_addr);

	for(i=0; i<10; i++)
		outw(0x00, (unsigned int)iom_fpga_dot_addr + i * 2);

	for(i=0; i<32; i++)
		outw(0x00, (unsigned int)iom_fpga_text_lcd_addr + i);

	line1_position      =  0; 
	line1_reverse_flag  =  FALSE;
	line2_position      =  0;
	line2_reverse_flag  =  FALSE;
}

/* Open integrated device driver */
int device_open(struct inode *minode, struct file *mfile) 
{	
	if(device_port_usage != 0) 
		return -EBUSY;

	printk("device_open\n");
	device_port_usage = 1;

	return 0;
}

/* Close integrated device driver */
int device_release(struct inode *minode, struct file *mfile) 
{
	printk("device_release\n");
	device_port_usage = 0;

	return 0;
}

/* Device control (write) using ioctl */
long device_ioctl(struct file *mfile, unsigned int cmd, unsigned long arg)
{
	const char *tmp = (char*)arg;

	if(cmd == IOCTL_WRITE)
	{
		unsigned char gdata[4];
			
		if(copy_from_user(&gdata, tmp, sizeof(unsigned long)) == 0)
			device_write(mfile, gdata, sizeof(unsigned long), 0);
	}

	return 0;
}

/* Device control using write */
ssize_t device_write(struct file *mfile, const char *gdata, size_t length, loff_t *off_what)
{
	char value[4];

	/* Copy packed parameter (gdata) to local variable */
	memcpy(value, gdata, length);

	/* Unpacking parameter ang assign value to each global variable */
	start_location    =  value[3];
	start_value       =  value[2];
	time_interval     =  value[1];
	timer_count       =  value[0];

	/* Initiate current_location and current_value */
	current_location  =  start_location;
	current_value     =  start_value;

	/* Initiate Timer */
	init_timer(&timer);	

	/* Set expire time and function */
	timer.expires     =  jiffies;
	timer.function    =  write_handler;

	/* Add to kernel timer list */
	add_timer(&timer);

	return length;
}

/* Register device driver module */
int __init device_init(void)
{
	int result;
	
	result = register_chrdev(DEVICE_MAJOR, DEVICE_NAME, &device_fops);

	if(result < 0) 
	{
		printk(KERN_WARNING"Can't get any major\n");

		return result;
	}

	/* Memory mapping */
	iom_fpga_led_addr       =  ioremap(IOM_FPGA_LED_ADDRESS,       0x1);
	iom_fpga_fnd_addr       =  ioremap(IOM_FPGA_FND_ADDRESS,       0x4);
	iom_fpga_dot_addr       =  ioremap(IOM_FPGA_DOT_ADDRESS,       0x10);
	iom_fpga_text_lcd_addr  =  ioremap(IOM_FPGA_TEXT_LCD_ADDRESS,  0x32);

	return 0;
}

/* Remove device driver module */
void __exit device_exit(void) 
{
	/* Unmapping Device */
	iounmap(iom_fpga_led_addr);
	iounmap(iom_fpga_fnd_addr);
	iounmap(iom_fpga_dot_addr);
	iounmap(iom_fpga_text_lcd_addr);

	/* Deactivate timer */
	del_timer_sync(&timer);

	unregister_chrdev(DEVICE_MAJOR, DEVICE_NAME);
}


module_init(device_init);
module_exit(device_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huins");
