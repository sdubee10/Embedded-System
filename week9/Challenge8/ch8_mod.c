#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/timer.h>
#include <linux/interrupt.h>

MODULE_LICENSE("GPL");

//SW.K
#define LED1 5
#define SENSOR1 17

struct my_timer_info{
	struct timer_list timer;
	long delay_jiffies;
};
static int irq_num;

static struct my_timer_info my_timer;

static void my_timer_func(struct timer_list *t)
{
	struct my_timer_info *info = from_timer(info, t, timer);
	//mod_timer(&my_timer.timer, jiffies + info->delay_jiffies);
	
	if(gpio_get_value(LED1))
		gpio_set_value(LED1,0);
	else
	{
		gpio_set_value(LED1, 0);
	}
}

static irqreturn_t simple_sensor_isr(int irq, void* dev_id)
{
	printk("detect\n");
	if(gpio_get_value(LED1))
	{
		gpio_set_value(LED1, 0);
		timer_setup(&my_timer.timer, my_timer_func, 0);
		my_timer.timer.expires = jiffies + msecs_to_jiffies(500);
		add_timer(&my_timer.timer);
	}
	else
	{
		gpio_set_value(LED1, 1);
		timer_setup(&my_timer.timer, my_timer_func, 0);
		my_timer.timer.expires = jiffies + my_timer.delay_jiffies;
		add_timer(&my_timer.timer);
	}

	return IRQ_HANDLED;
}

static int __init ch8_mod_init(void)
{
	int ret;
	printk("Init Module\n");

	gpio_request_one(LED1, GPIOF_OUT_INIT_LOW, "LED1");
	gpio_request_one(SENSOR1, GPIOF_IN, "sensor1");
	gpio_set_value(LED1, 0);
	my_timer.delay_jiffies = msecs_to_jiffies(2000);
	//timer_setup(&my_timer.timer, my_timer_func, 0);
	//my_timer.timer.expires=jiffies + my_timer.delay_jiffies;
	//add_timer(&my_timer.timer);

	irq_num = gpio_to_irq(SENSOR1);
	ret = request_irq(irq_num, simple_sensor_isr, IRQF_TRIGGER_RISING, "sensor_irq", NULL);
	if(ret)
	{
		printk("Unable to request IRQ: %d\n", ret);
		free_irq(irq_num, NULL);
	}
	else
	{
		disable_irq(irq_num);
	}
	enable_irq(irq_num);
	return 0;
}

static void __exit ch8_mod_exit(void)
{
	printk("Exit Module\n");
	
	disable_irq(irq_num);
	free_irq(irq_num, NULL);
	gpio_set_value(LED1,0);
	gpio_free(LED1);
	gpio_free(SENSOR1);
	del_timer(&my_timer.timer);
}

module_init(ch8_mod_init);
module_exit(ch8_mod_exit);
