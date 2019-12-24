

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/workqueue.h>


void work_func(struct work_struct* work)
{
    pr_info("%s: %lx\n", __func__, (unsigned long)work);
}

// unsigned long data;

static DECLARE_WORK(my_work, work_func);


static int __init workqueue0_init(void)
{
    pr_info("%s: 1\n", __func__);
    schedule_work(&my_work);

    pr_info("workqueue0 inserted\n");
    return 0;
}

static void __exit workqueue0_exit(void)
{
    pr_info("workqueue0 removed\n");
}


module_init(workqueue0_init);
module_exit(workqueue0_exit);
MODULE_LICENSE("GPL v2");
