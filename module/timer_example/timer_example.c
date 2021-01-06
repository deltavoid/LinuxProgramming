

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>

// #include <linux/tracepoint.h>
// #include <trace/events/sched.h>

#include <linux/sched.h>


// preempt_count display ------------------------------------------

static void preempt_count_display(void)
{
    struct task_struct* task_p = current;

    preempt_disable();
    // comm should use get_task_comm
    pr_debug("cpu_id: %d, task tid/pid: %d, pid/tgid: %d, comm: %s\n", 
            smp_processor_id(), task_p->pid, task_p->tgid, task_p->comm);
    preempt_enable();

    pr_debug("preempt_count: 0x%08x, test_preempt_need_resched: %d\n", preempt_count(), test_preempt_need_resched());
    // pr_debug("test_preempt_need_resched: %d\n", test_preempt_need_resched());
}

// timer example --------------------------------------------------------

struct timer_example {
    struct timer_list timer;
    uint64_t cnt;
};

#define timer_example_timeout 1

static void timer_example_expire__(struct timer_example* data, struct timer_list* timer)
{
    pr_debug("timer_example_expire__, cnt: %lld\n", data->cnt);
    preempt_count_display();
    dump_stack();

    if  (++data->cnt == 10)
    {   pr_debug("del_timer\n");
        del_timer(timer);
    }
    else
    {   pr_debug("jeffies: %ld\n", jiffies);
        mod_timer(timer, jiffies + timer_example_timeout * HZ);
    }
    printk("\n");
}

static void timer_example_expire(struct timer_list *timer)
{
    struct timer_example* data = from_timer(data, timer, timer);
    timer_example_expire__(data, timer);
}

static int timer_example_init(struct timer_example* data)
{
    data->cnt = 0;
    timer_setup(&data->timer, timer_example_expire, 0);
    mod_timer(&data->timer, jiffies + timer_example_timeout * HZ);
    return 0;
}

static void timer_example_exit(struct timer_example* data)
{
    pr_debug("timer_example_exit, timer_pending: %d\n", timer_pending(&data->timer));
    if  (timer_pending(&data->timer))
    {   pr_debug("timer_example_exit, del_timer\n");
        del_timer(&data->timer);
    }
}


// module init ----------------------------------------------------------

struct timer_example example;

static int __init timer_example_module_init(void)
{
    pr_info("timer_example_module_init begin\n");

    preempt_count_display();
    timer_example_init(&example);

    pr_info("timer_example_module_init end\n");
    return 0;
}

static void __exit timer_example_module_exit(void)
{
    pr_info("timer_example_module_exit begin\n");

    preempt_count_display();
    timer_example_exit(&example);

    pr_info("timer_example_module_exit end\n");
    pr_debug("-------------------------------------------------\n");
}


module_init(timer_example_module_init);
module_exit(timer_example_module_exit);
MODULE_LICENSE("GPL");
