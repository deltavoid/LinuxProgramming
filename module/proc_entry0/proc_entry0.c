

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/kallsyms.h>
#include <linux/err.h>

#include <linux/sysctl.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

#include <linux/netdevice.h>


static int pet_entry_show(struct seq_file *seq, void *arg)
{
    int i;

    pr_debug("pet_entry_show\n");
    
    seq_puts(seq, "proc entry0 hello\n");

    for (i = 0; i < 10; i++)
        seq_puts(seq, "proc entry0 world\n");

    return 0;
}

static int pet_seq_open(struct inode *inode, struct file *file)
{
    pr_debug("pet_seq_open\n");

    return single_open(file, pet_entry_show, NULL);
}


static const struct file_operations pet_entry_fops = 
{
    .owner      = THIS_MODULE,
    .open       = pet_seq_open,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = single_release,
};


static int __init pet_entry_init(void)
{
    proc_create("proc_entry0", 0, init_net.proc_net, &pet_entry_fops);

    return 0;
}

static void __exit pet_entry_exit(void)
{
    remove_proc_entry("proc_entry0", init_net.proc_net);
}


static int __init proc_entry0_init(void)
{
    pet_entry_init();

    pr_info("proc_entry0 inserted\n");
    return 0;
}

static void __exit proc_entry0_exit(void)
{
    pet_entry_exit();

    pr_info("proc_entry0 removed\n");
    pr_debug("-------------------------------------------------\n");
}


module_init(proc_entry0_init);
module_exit(proc_entry0_exit);
MODULE_LICENSE("GPL");
