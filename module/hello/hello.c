

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>




// module init ----------------------------------------------------------

static int __init hello_init(void)
{
    pr_info("hello_init begin\n");


    pr_info("hello_init end\n");
    return 0;
}

static void __exit hello_exit(void)
{
    pr_info("hello_exit begin\n");


    pr_info("hello_exit end\n");
    pr_debug("-------------------------------------------------\n");
}


module_init(hello_init);
module_exit(hello_exit);
MODULE_LICENSE("GPL");
