

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>




// module init ----------------------------------------------------------

atomic_t cnt;

static int __init atomic_use_init(void)
{
    pr_info("atomic_use_init begin\n");

    atomic_set(&cnt, 0);
    pr_debug("cnt: %d\n", atomic_read(&cnt));

    atomic_inc(&cnt);
    pr_debug("cnt: %d\n", atomic_read(&cnt));

    atomic_add(1, &cnt);
    pr_debug("cnt: %d\n", atomic_read(&cnt));

    atomic_sub(1, &cnt);
    pr_debug("cnt: %d\n", atomic_read(&cnt));


    pr_info("atomic_use_init end\n");
    return 0;
}

static void __exit atomic_use_exit(void)
{
    pr_info("atomic_use_exit begin\n");


    pr_info("atomic_use_exit end\n");
    pr_debug("-------------------------------------------------\n");
}


module_init(atomic_use_init);
module_exit(atomic_use_exit);
MODULE_LICENSE("GPL");
