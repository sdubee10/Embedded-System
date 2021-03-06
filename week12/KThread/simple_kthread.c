#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");

struct task_struct *my_kthread = NULL;

int simple_kthread_func(void * dats)
{
	while(!kthread_should_stop())
	{
		printk("simple_kthread: iffies in thread = %ld \n", jiffies);

		msleep(1000);
	}
	return 0;
}

static int __init simple_kthread_init(void)
{
	printk("simple_kthread: Init Module \n");

	my_kthread = kthread_create(simple_kthread_func, NULL, "My Kthread");
	if(IS_ERR(my_kthread))
	{
		my_kthread = NULL;
		printk("simple_kthread : my kernel thread ERROR \n");
	}

	wake_up_process(my_kthread);
	return 0;
}

static void __exit simple_kthread_exit(void)
{
	if(my_kthread)
	{
		kthread_stop(my_kthread);
		printk("simple_kthread : my kernel thread STOP \n");
	}
	printk("simple_kthread: Exit Module\n");
}

module_init(simple_kthread_init);
module_exit(simple_kthread_exit);
