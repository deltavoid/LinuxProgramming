

// #define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>


static int __init hello_init(void)
{
    pr_info("hello inserted\n");
    // printk(KERN_INFO "hello inserted\n");
    // printk(KERN_DEBUG "hello debug\n");
    pr_debug("hello debug");
    
    return 0;
}

static void __exit hello_exit(void)
{
    pr_info("hello removed\n");
    // printk(KERN_INFO "hello removed\n");
    pr_debug("-------------------------------------------------\n");
    // printk(KERN_DEBUG "------------------------------------------------\n");
}


module_init(hello_init);
module_exit(hello_exit);
MODULE_LICENSE("GPL");
