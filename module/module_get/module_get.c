

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>




// module init ----------------------------------------------------------

static int __init module_get_init(void)
{
    pr_info("module_get_init begin\n");


    pr_info("module_get_init end\n");
    return 0;
}

static void __exit module_get_exit(void)
{
    pr_info("module_get_exit begin\n");


    pr_info("module_get_exit end\n");
    pr_debug("-------------------------------------------------\n");
}


module_init(module_get_init);
module_exit(module_get_exit);
MODULE_LICENSE("GPL");
