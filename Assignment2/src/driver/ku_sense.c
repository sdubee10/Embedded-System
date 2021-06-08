#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/rwlock.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include "ku_sense.h"


MODULE_LICENSE("GPL");

#define DEV_NAME "ku_sense_dev"
#define SWITCH 12
#define SPEAKER 23

//motor gpio pin
#define PIN1 6
#define PIN2 13
#define PIN3 19
#define PIN4 26

#define STEPS 8
#define ONEROUND 512

int blue[8] = {1, 1, 0, 0, 0, 0, 0, 1};
int pink[8] = {0, 1, 1, 1, 0, 0, 0, 0};
int yellow[8] = {0, 0, 0, 1, 1, 1, 0, 0};
int orange[8] = {0, 0, 0, 0, 0, 1, 1, 1};

//music note
int notes[] = {1911, 1516, 1275, 1012};

static dev_t dev_num;
static struct cdev *cd_cdev;

static int irq_num;
struct ku_data data;

//motor
void setstep(int p1, int p2, int p3, int p4)
{
	gpio_set_value(PIN1, p1);
	gpio_set_value(PIN2, p2);
	gpio_set_value(PIN3, p3);
	gpio_set_value(PIN4, p4);
}

void backward(int degree, int delay)
{
	int i = 0, j = 0;
	for (i = 0; i< ONEROUND * degree /360; i++)
	{
		for (j = STEPS; j > 0; j--)
		{
			setstep(blue[j], pink[j], yellow[j], orange[j]);
			udelay(delay);
		}
	}
	setstep(0, 0, 0, 0);
}

void forward(int degree, int delay)
{
	int i =0, j = 0;

	for(i = 0; i < ONEROUND * degree / 360; i++)
	{
		for(j = 0; j<STEPS; j++)
		{
			setstep(blue[j], pink[j], yellow[j], orange[j]);
			udelay(delay);
		}
	}
	setstep(0,0,0,0);
}

static void play(int note, int degree, int duration, int choice)
{
	//int i = 0;
	if (choice == 0)
	{
		gpio_set_value(SPEAKER, 1);
		forward(degree, duration);
	}
	else
	{
		gpio_set_value(SPEAKER, 1);
		backward(degree, duration);
	}
	//mdelay(duration);
	gpio_set_value(SPEAKER, 0);
	udelay(note);
}

//switch interrupt
static irqreturn_t switch_irq_isr(int irq, void* dev_id)
{
	printk("switch_irq : Detect\n");
	data.switchPower++;
	return IRQ_HANDLED;
}

static long ku_sense_mod_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret;
	int i;
	switch(cmd)
	{
		case(IOCTL_SWITCH):
			printk("SWITCH Mode\n");
			ret = request_irq(irq_num, switch_irq_isr, IRQF_TRIGGER_RISING, "sensor_irq", NULL);
			if(ret)
			{
				printk("switch_irq: Unable to reset request IRQ: %d\n", ret);
				free_irq(irq_num, NULL);
			}
			return (data.switchPower);
			break;
		case(IOCTL_MOTOR1):
			printk("MOTOR1 Mode\n");
			for(i = 0; i< 4; i++)
			{
				if (i % 4 == 0 || i % 4 == 2)
					play(notes[i], 90, 1800, 0);
				else
					play(notes[i], 90, 1800, 1);
			}
			break;

		case(IOCTL_MOTOR2):
			printk("MOTOR2 Mode\n");
			for(i = 0; i < 4; i++)
			{
				if (i % 4 == 0 || i % 4 == 2)
					play(notes[i], 60, 1300, 0);
				else
					play(notes[i], 60, 1300, 1);
			}
			break;
		case(IOCTL_MOTOR3):
			printk("MOTOR3 Mode\n");
			for(i = 0; i < 4; i++)
			{
				if (i % 4 == 0 || i % 4 == 2)
					play(notes[i], 30, 800, 0);
				else
					play(notes[i+1], 30, 800, 1);
			}
			break;
	}
	return 0;
}

static int ku_sense_mod_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int ku_sense_mod_release(struct inode *inode, struct file *file)
{
	return 0;
}

struct file_operations ku_sense_mod_fops =
{
	.unlocked_ioctl = ku_sense_mod_ioctl,
	.open = ku_sense_mod_open,
	.release = ku_sense_mod_release,
};

static int __init ku_sense_init(void)
{

	int ret = 0;
	data.switchPower = 0;
	printk("switch_irq: init Module \n");
	
	alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
	cd_cdev = cdev_alloc();
	
	cdev_init(cd_cdev, &ku_sense_mod_fops);
	
	ret = cdev_add(cd_cdev, dev_num, 1);
	if (ret < 0)
	{
		printk("fail to add charcter device \n");
		return (-1);
	}

	//
	gpio_request_one(SWITCH, GPIOF_IN, "SWITCH");
	
	//
	gpio_request_one(PIN1, GPIOF_OUT_INIT_LOW, "p1");
	gpio_request_one(PIN2, GPIOF_OUT_INIT_LOW, "p2");
	gpio_request_one(PIN3, GPIOF_OUT_INIT_LOW, "p3");
	gpio_request_one(PIN4, GPIOF_OUT_INIT_LOW, "p4");

	//
	gpio_request_one(SPEAKER, GPIOF_OUT_INIT_LOW, "SPEAKER");

	irq_num = gpio_to_irq(SWITCH);

	return 0;

}

static void __exit ku_sense_exit(void)
{
	printk("switch_irq: exit module \n");

	gpio_set_value(SPEAKER, 0);
	cdev_del(cd_cdev);
	unregister_chrdev_region(dev_num, 1);
	disable_irq(irq_num);
	free_irq(irq_num, NULL);
	
	gpio_free(SWITCH);
	
	gpio_free(PIN1);
	gpio_free(PIN2);
	gpio_free(PIN3);
	gpio_free(PIN4);

	gpio_free(SPEAKER);
}

module_init(ku_sense_init);
module_exit(ku_sense_exit);
