

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/workqueue.h>


static void work_func(struct work_struct* work)
{
    pr_info("%s: %lx\n", __func__, (unsigned long)work);
}


static struct workqueue_struct* my_workqueue;

static DECLARE_WORK(my_work, work_func);


static int __init workqueue1_init(void)
{
    pr_info("%s: 1\n", __func__);
    my_workqueue = create_workqueue("my_workqueue");

    pr_info("%s: 2\n", __func__);
    queue_work(my_workqueue, &my_work);

    pr_info("workqueue1 inserted\n");
    return 0;
}

static void __exit workqueue1_exit(void)
{
    pr_info("%s: 1\n", __func__);
    flush_workqueue(my_workqueue);

    pr_info("%s: 2\n", __func__);
    destroy_workqueue(my_workqueue);

    pr_info("workqueue1 removed\n");
    pr_info("--------------------------------------\n");
}


module_init(workqueue1_init);
module_exit(workqueue1_exit);
MODULE_LICENSE("GPL");
