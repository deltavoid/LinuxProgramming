

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>

#include <linux/kprobes.h>



// preempt_count_display --------------------------------------------



static void preempt_count_display(void)
{
    unsigned preempt_cnt = preempt_count();
    // if  (preempt_cnt)
        pr_debug("preempt_count: 0x%08x\n", preempt_cnt);
}


// kprobe -----------------------------------------------------------

#define MAX_SYMBOL_LEN	64
static char symbol[MAX_SYMBOL_LEN] = "_do_fork";
module_param_string(symbol, symbol, sizeof(symbol), 0644);



/* For each probe you need to allocate a kprobe structure */
static struct kprobe kp = {
    .symbol_name	= symbol,
};

static unsigned long cnt = 0;

/* kprobe pre_handler: called just before the probed instruction is executed */
static int handler_pre(struct kprobe *p, struct pt_regs *regs)
{
    if  (++cnt % 10 == 0)
    {
        pr_debug("%s cnt: %ld --------------------------------------------------------------\n", 
                p->symbol_name, cnt);
        // pr_debug("cnt: %ld\n", cnt);
        preempt_count_display();
        
        dump_stack();
    }
    

    return 0;
}

/*
 * fault_handler: this is called if an exception is generated for any
 * instruction within the pre- or post-handler, or when Kprobes
 * single-steps the probed instruction.
 */
static int handler_fault(struct kprobe *p, struct pt_regs *regs, int trapnr)
{
    pr_info("fault_handler: p->addr = 0x%p, trap #%dn", p->addr, trapnr);
    /* Return 0 because we don't handle the fault. */
    return 0;
}


// module init ---------------------------------------------------------------


spinlock_t example_lock;

static int __init preempt_count_display_init(void)
{
    // int ret;
    // kp.pre_handler = handler_pre;
    // // kp.post_handler = handler_post;
    // kp.post_handler = NULL;
    // kp.fault_handler = handler_fault;

    // ret = register_kprobe(&kp);
    // if (ret < 0) {
    //     pr_err("register_kprobe failed, returned %d\n", ret);
    //     return ret;
    // }
    // pr_info("Planted kprobe at %p\n", kp.addr);

    pr_debug("module_init\n");
    preempt_count_display();



    spin_lock(&example_lock);

    pr_debug("spin_lock\n");
    preempt_count_display();

    spin_unlock(&example_lock);


    local_bh_disable();

    pr_debug("local_bh_disable\n");
    preempt_count_display();

    local_bh_enable();


    spin_lock_bh(&example_lock);

    pr_debug("spin_lock_bh\n");
    preempt_count_display();

    spin_unlock_bh(&example_lock);

    

    return 0;
}

static void __exit preempt_count_display_exit(void)
{
    // unregister_kprobe(&kp);

    preempt_count_display();


    pr_info("kprobe at %p unregistered\n", kp.addr);
    pr_debug("----------------------------------------------\n");
}

module_init(preempt_count_display_init)
module_exit(preempt_count_display_exit)
MODULE_LICENSE("GPL");
