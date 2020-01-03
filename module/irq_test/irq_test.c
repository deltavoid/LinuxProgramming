

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/interrupt.h>



static int irq; /*模块参数-中断号*/
module_param(irq, int, 0644);

static char dev_name[] = "test";
static long count = 0; /*统计插入模块期间发生的中断次数*/
static long last_jiffies;



static irqreturn_t intr_handler(int irq, void *dev_id)
{
    count++;

    if  (count % 10 == 0)
    {
        long now_jiffies =  jiffies;
        pr_info("irq %d happens for %lx, count: %ld, interval: %ld\n", 
                irq, (unsigned long)dev_id, count, now_jiffies - last_jiffies);
        last_jiffies = now_jiffies;
    }

    return IRQ_NONE;
}



static int __init irq_test_init(void)
{
    pr_info("irq: %d, dev: %lx\n", irq, &irq);

    if  (request_irq(irq, &intr_handler, IRQF_SHARED, dev_name, &irq) != 0)
    {   pr_warn("request_irq failed\n");
    }

    pr_info("irq_test inserted\n");
    return 0;
}

static void __exit irq_test_exit(void)
{
    if  (free_irq(irq, &irq) == NULL)
    {   pr_warn("free_irq %d failed\n", irq); 
    }
    
    pr_info("irq_test removed\n");
    pr_info("---------------------------------------------------\n");
}


module_init(irq_test_init);
module_exit(irq_test_exit);
MODULE_LICENSE("GPL");
