

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/interrupt.h>



static int irq; /*模块参数-中断号*/
module_param(irq, int, 0644);

// static char *interface; /*模块参数-设备名*/
// module_param(interface, char*, 0644);
static char dev_name[] = "test";

static long count = 0; /*统计插入模块期间发生的中断次数*/
static long last_jiffies;







static irqreturn_t intr_handler(int irq, void *dev_id)
{
    // static long interval = 0;

    // if(count==0)
    // {
    //     interval=jiffies;
    // }
    
    // /*计算两次中断之间的间隔,时间单位为节拍 */
    // interval = jiffies - interval; 
    
    // // pr_info("The interval between two interrupts is %ld\n" interval);

    // interval=jiffies;
    

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
    // /*注册中断服务程序，注意内核版本不同，共享标志可能有所不同*/
    // if (request_irq(irq, &intr_handler, IRQF_SHARED, interface, &irq)) {
    //     pr_err("Fails to register IRQ %dn", irq);
    //     return -EIO;
    // }
    // pr_info("%s Request on IRQ %d succeededn",interface,irq);
    pr_info("irq: %d, dev: %lx\n", irq, &irq);

    if  (request_irq(irq, &intr_handler, IRQF_SHARED, dev_name, &irq) != 0)
    {   pr_warn("request_irq failed\n");
    }

    pr_info("irq_test inserted\n");
    return 0;
}

static void __exit irq_test_exit(void)
{
    // pr_info("The %d interrupts happened on irq %d", count, irq);
    // free_irq(irq, &irq); /* 释放中断线*/
    // pr_info("Freeing IRQ %d\n", irq);

    if  (free_irq(irq, &irq) == NULL)
    {   pr_warn("free_irq %d failed\n", irq); 
    }
    
    pr_info("irq_test removed\n");
    pr_info("---------------------------------------------------\n");
}


module_init(irq_test_init);
module_exit(irq_test_exit);
MODULE_LICENSE("GPL");
