

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>




// module init ----------------------------------------------------------

static int __init irq_request_init(void)
{
    pr_info("irq_request_init begin\n");


    pr_info("irq_request_init end\n");
    return 0;
}

static void __exit irq_request_exit(void)
{
    pr_info("irq_request_exit begin\n");


    pr_info("irq_request_exit end\n");
    pr_debug("-------------------------------------------------\n");
}


module_init(irq_request_init);
module_exit(irq_request_exit);
MODULE_LICENSE("GPL");
