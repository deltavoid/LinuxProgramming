

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>




static __init int tasklet_init(void)
{
    pr_info("tasklet inserted\n");
    return 0;
}

static __exit void tasklet_exit(void)
{
    pr_info("tasklet removed\n");
}


module_init(tasklet_init);
module_exit(tasklet_exit);
MODULE_LICENSE("GPL");
