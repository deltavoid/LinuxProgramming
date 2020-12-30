

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>


static int __init mutex_use_init(void)
{
    pr_info("mutex_use inserted\n");
    return 0;
}

static void __exit mutex_use_exit(void)
{
    pr_info("mutex_use removed\n");
    pr_debug("-------------------------------------------------\n");
}


module_init(mutex_use_init);
module_exit(mutex_use_exit);
MODULE_LICENSE("GPL");
