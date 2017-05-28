#include <mach/gpio.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/gpio.h>
#include <asm/uaccess.h>

#include <linux/fs.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/cdev.h>
#include <linux/wait.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>


#define TRUE   1
#define FALSE  0

#define DEVICE_MAJOR        242
#define DEVICE_NAME         "/dev/stopwatch"

#define IOM_FND_ADDRESS 	0x08000004


/*************** Define function ***************/
void         stopwatch_timer_handler  (unsigned long timeout);
void         exit_timer_handler       (unsigned long timeout);

void         clear_stopwatch          (void);

irqreturn_t  home_interrupt           (int irq, void* dev_id, struct pt_regs* reg);
irqreturn_t  back_interrupt           (int irq, void* dev_id, struct pt_regs* reg);
irqreturn_t  volume_up_interrupt      (int irq, void* dev_id, struct pt_regs* reg);
irqreturn_t  volume_down_interrupt    (int irq, void* dev_id, struct pt_regs* reg);

int          device_open              (struct inode *minode, struct file *mfile);
int          device_release           (struct inode *minode, struct file *mfile);
ssize_t      device_write             (struct file *inode, const char *gdata, size_t length, loff_t *off_what);



/*************** Define Struct ***************/
struct file_operations device_fops =
{
	.open		=	device_open,
	.release	=	device_release,
	.write		=	device_write,	
};


/*************** Define Global Variable ***************/
int device_port_usage = 0;

unsigned char *iom_fpga_fnd_addr;

wait_queue_head_t wq_write;
DECLARE_WAIT_QUEUE_HEAD(wq_write);

struct timer_list stopwatch_timer;
struct timer_list exit_timer;

int start_flag = FALSE;
int pause_flag = FALSE;

unsigned short int current_seconds = 0;



/************************* Program Code *************************/

/* Write time value at fnd_device on every second */
void stopwatch_timer_handler(unsigned long timeout)
{
	unsigned char value[4];
	unsigned short int value_short = 0;
	unsigned short int minute = 0;
	unsigned short int second = 0;

	if(pause_flag == FALSE)
	{
		current_seconds++;

		if(current_seconds == 3600)
			current_seconds = 0;
			
		minute = current_seconds / 60;
		second = current_seconds % 60;

		/* Make value to write at fnd_device */
		value[0] = minute / 10;
		value[1] = minute % 10;
		value[2] = second / 10;
		value[3] = second % 10;

		value_short = value[0] << 12 | value[1] << 8 | value[2] << 4 | value[3];

	    outw(value_short,(unsigned int)iom_fpga_fnd_addr);	

		stopwatch_timer.expires   =  get_jiffies_64() + (1 * HZ);
		stopwatch_timer.function  =  stopwatch_timer_handler;

		add_timer(&stopwatch_timer);
	}
}

/* Exit stopwatch device and clear fnd_device */
void exit_timer_handler(unsigned long timeout)
{
	printk("stopwatch exit\n");

	clear_stopwatch();

	start_flag = FALSE;
	pause_flag = FALSE;

	del_timer(&stopwatch_timer);

	__wake_up(&wq_write, 1, 1, NULL);
}

/* Clear fnd_device */
void clear_stopwatch(void)
{
	current_seconds = 0;
	outw(0x0000,(unsigned int)iom_fpga_fnd_addr);
}

/* Start stopwatch interrupt */
irqreturn_t home_interrupt(int irq, void* dev_id, struct pt_regs* reg)
{
	if(start_flag == FALSE)
	{
		if(pause_flag == FALSE)
			printk("stopwatch start\n");
		else
			printk("stopwatch restart\n");

		pause_flag = FALSE;	
		start_flag = TRUE;

		stopwatch_timer.expires   =  get_jiffies_64() + (1 * HZ);
		stopwatch_timer.function  =  stopwatch_timer_handler;

		add_timer(&stopwatch_timer);
	}

	return IRQ_HANDLED;
}

/* Pause stopwatch interrupt */
irqreturn_t back_interrupt(int irq, void* dev_id, struct pt_regs* reg)
{
	if(pause_flag == FALSE)
		printk("stopwatch pause\n");

	pause_flag = TRUE;
	start_flag = FALSE;

	/* Deactivate timer before expire */
	del_timer(&stopwatch_timer);

	return IRQ_HANDLED;
}

/* Reset stopwatch interrupt */
irqreturn_t volume_up_interrupt(int irq, void* dev_id, struct pt_regs* reg)
{
	printk("stopwatch reset\n");

	clear_stopwatch();

	pause_flag = FALSE;
	start_flag = FALSE;

	del_timer(&stopwatch_timer);

	return IRQ_HANDLED;
}

/* Exit stopwatch interrupt */
irqreturn_t volume_down_interrupt(int irq, void* dev_id, struct pt_regs* reg)
{
	if(gpio_get_value(IMX_GPIO_NR(5,14)) == 0)
	{
		printk("preparing exit\n");

		exit_timer.expires   =  get_jiffies_64() + (3 * HZ);
		exit_timer.function  =  exit_timer_handler;

		add_timer(&exit_timer);
	}
	else
	{
		printk("cancel exit\n");

		/* Deactivate timer before expire */
		del_timer(&exit_timer);
	}

	return IRQ_HANDLED;
}

/* Open stopwatch device */
int device_open(struct inode *minode, struct file *mfile) 
{	
	int ret, irq;

	if(device_port_usage != 0) 
		return -EBUSY;

	printk("device open\n");
	device_port_usage = 1;

	/* Home button interrupt */
	gpio_direction_input(IMX_GPIO_NR(1,11));
	irq = gpio_to_irq(IMX_GPIO_NR(1,11));
	ret = request_irq(irq, (irq_handler_t)home_interrupt, IRQF_TRIGGER_FALLING, "home", 0);

	/* Back button interrupt */
	gpio_direction_input(IMX_GPIO_NR(1,12));
	irq = gpio_to_irq(IMX_GPIO_NR(1,12));
	ret = request_irq(irq, (irq_handler_t)back_interrupt, IRQF_TRIGGER_FALLING, "back", 0);

	/* Volume up button interrupt */
	gpio_direction_input(IMX_GPIO_NR(2,15));
	irq = gpio_to_irq(IMX_GPIO_NR(2,15));
	ret = request_irq(irq, (irq_handler_t)volume_up_interrupt, IRQF_TRIGGER_FALLING, "volume_up", 0);

	/* Volume down button interrupt */
	gpio_direction_input(IMX_GPIO_NR(5,14));
	irq = gpio_to_irq(IMX_GPIO_NR(5,14));
	ret = request_irq(irq, (irq_handler_t)volume_down_interrupt, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "volume_down", 0);

	return 0;
}

/* Release stopwatch device */
int device_release(struct inode *minode, struct file *mfile) 
{
	printk("device release\n");
	device_port_usage = 0;

	/* Remove interrupt */
	free_irq(gpio_to_irq(IMX_GPIO_NR(1, 11)), NULL);
	free_irq(gpio_to_irq(IMX_GPIO_NR(1, 12)), NULL);
	free_irq(gpio_to_irq(IMX_GPIO_NR(2, 15)), NULL);
	free_irq(gpio_to_irq(IMX_GPIO_NR(5, 14)), NULL);

	/* Deactivate timer */
	del_timer(&exit_timer);

	return 0;
}

/* Initiate stopwatch device and block user program */
ssize_t device_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) 
{
	clear_stopwatch();

	/* Init timer */
	init_timer(&stopwatch_timer);
	init_timer(&exit_timer);

	/* Block user program */
    interruptible_sleep_on(&wq_write);

	return length;
}

/* Register stopwatch device driver moodule */
int __init device_init(void)
{
	int result;

	/* Register character device driver module */
	result = register_chrdev(DEVICE_MAJOR, DEVICE_NAME, &device_fops);

	if(result < 0) 
	{
		printk(KERN_WARNING"Can't get any major\n");

		return result;
	}

	/* Memory mapping */
	iom_fpga_fnd_addr = ioremap(IOM_FND_ADDRESS, 0x4);

	printk("init module, %s major number : %d\n", DEVICE_NAME, DEVICE_MAJOR);

	return 0;
}

/* Unregister stopwatch device drvier module */
void __exit device_exit(void) 
{
	/* Unmapping FND Device */
	iounmap(iom_fpga_fnd_addr);

	/* Unregister Device driver */
	unregister_chrdev(DEVICE_MAJOR, DEVICE_NAME);
}


module_init(device_init);
module_exit(device_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huins");
