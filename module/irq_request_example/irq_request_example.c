

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>

#include <linux/interrupt.h>


// gaea nic input irq line on cpu0
#define example_irq_line 33


static irqreturn_t example_interrupt(int irq, void *dev_data)
{
    uint64_t* cnt = dev_data;

    // if  (++*cnt % 100 == 0)
    {
        pr_debug("example_interrupt, irq: %d, cnt: %lld\n", irq, *cnt);
    }

    return IRQ_HANDLED;
}


// module init ----------------------------------------------------------

static uint64_t count;

int irq_line;


static void test_irq(void)
{
    int i;

    for (i = 0; i < 100; i++)
    {
        if  (request_irq(i, example_interrupt, IRQF_SHARED, "test", &count) < 0)
        {
            pr_debug("request_irq %d failed\n", i);
            continue;
        }

        pr_debug("request_irq %d success\n", i);
        free_irq(i, &count);
    }
}


static int __init irq_request_init(void)
{
    // int i;
    pr_info("irq_request_init begin\n");

    irq_line = 3;
    if  (request_irq(irq_line, example_interrupt, IRQF_SHARED, "test", &count) < 0)
    {   pr_warn("request_irq failed\n");
        return -1;
    }
    // irq_line = -1;
    // for (i = 0; i < 100; i++)
    // {
    //     if  (request_irq(i, example_interrupt, IRQF_SHARED, "test", &count) == 0)
    //     {   pr_debug("request_irq %d success\n", i);
    //         irq_line = i;
    //         break;
    //     }
    // }

    // if  (irq_line < 0)
    //     return -1;

    // test_irq();

    pr_info("irq_request_init end\n");
    return 0;
}

static void __exit irq_request_exit(void)
{
    pr_info("irq_request_exit begin\n");

    free_irq(irq_line, &count);


    pr_info("irq_request_exit end\n");
    pr_debug("-------------------------------------------------\n");
}


module_init(irq_request_init);
module_exit(irq_request_exit);
MODULE_LICENSE("GPL");
