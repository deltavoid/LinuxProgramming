

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/interrupt.h>



void tasklet_hello(unsigned long arg)
{
    pr_info("%s: %lx\n", __func__, arg);
}

DECLARE_TASKLET(my_tasklet, tasklet_hello, 0x12345678);



static __init int tasklet_module_init(void)
{
    pr_info("%s: 1\n", __func__);
    tasklet_schedule(&my_tasklet);

    pr_info("tasklet inserted\n");
    return 0;
}

static __exit void tasklet_module_exit(void)
{
    pr_info("tasklet removed\n");
}


module_init(tasklet_module_init);
module_exit(tasklet_module_exit);
MODULE_LICENSE("GPL");
