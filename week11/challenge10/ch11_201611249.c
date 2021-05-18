#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/cdev.h>

MODULE_LICENSE("GPL");

#define PIN1 6
#define PIN2 13
#define PIN3 19
#define PIN4 26

#define STEPS 8
#define PHASE 4

int steps[STEPS][PHASE] = {
	{1, 0, 0, 0},
	{1, 1, 0, 0},
	{0, 1, 0, 0},
	{0, 1, 1, 0},
	{0, 0, 1, 0},
	{0, 0, 1, 1},
	{0, 0, 0, 1},
	{1, 0, 0, 1}
};

void setSteps(int p1, int p2, int p3, int p4)
{
	gpio_set_value(PIN1, p1);
	gpio_set_value(PIN2, p2);
	gpio_set_value(PIN3, p3);
	gpio_set_value(PIN4, p4);
}

void forward(int degree, int delay)
{
	int i, j;
	for(i = 0; i < (int)512*degree/360; i++)
		for(j = 0; j < STEPS; j++)
		{
			setSteps(steps[j][0],steps[j][1], steps[j][2], steps[j][3]);
			udelay(delay);
		}
}


void backward(int degree, int delay)
{
	int i, j;
	for(i = 0; i < (int)512*degree/360; i++)
		for(j = STEPS - 1; j >=0; j--)
		{
			setSteps(steps[j][0],steps[j][1], steps[j][2], steps[j][3]);
			udelay(delay);
		}
}

void moveDegree(int degree, int delay, int direction)
{
	if(direction)
		backward(degree, delay);
	else
		forward(degree, delay);
}

static int __init motor_init(void)
{
	gpio_request_one(PIN1, GPIOF_OUT_INIT_LOW, "p1");
	gpio_request_one(PIN2, GPIOF_OUT_INIT_LOW, "p2");
	gpio_request_one(PIN3, GPIOF_OUT_INIT_LOW, "p3");
	gpio_request_one(PIN4, GPIOF_OUT_INIT_LOW, "p4");

	moveDegree(90, 3000, 0);
	mdelay(1200);
	moveDegree(45, 3000, 0);
	mdelay(1200);
	moveDegree(45, 3000, 0);
	mdelay(1200);
	moveDegree(180, 3000, 0);

	//실험
	/*
	moveDegree(90, 3000, 1);
	mdelay(1200);
	moveDegree(55, 3000, 1);
	mdelay(1200);
	moveDegree(270, 3000, 0);
	mdelay(1200);
	moveDegree(90, 3000, 1);
	*/
	return 0;
}


static void __exit motor_exit(void)
{
	gpio_free(PIN1);
	gpio_free(PIN2);
	gpio_free(PIN3);
	gpio_free(PIN4);
}

module_init(motor_init);
module_exit(motor_exit);
