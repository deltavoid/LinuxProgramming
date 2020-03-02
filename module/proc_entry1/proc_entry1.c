

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



#define FOOBAR_LEN 60

struct fb_data{
	char name[FOOBAR_LEN];
	char value[FOOBAR_LEN];
};


// foo ---------------------------------------------------------

// static int pet_entry_show(struct seq_file *seq, void *arg)
// {
//     int i;

//     pr_debug("pet_entry_show\n");
    
//     seq_puts(seq, "proc entry0 hello\n");

//     for (i = 0; i < 10; i++)
//         seq_puts(seq, "proc entry0 world\n");

//     return 0;
// }

// static int pet_seq_open(struct inode *inode, struct file *file)
// {
//     pr_debug("pet_seq_open\n");

//     return single_open(file, pet_entry_show, NULL);
// }


// static const struct file_operations foo_entry_fops = 
// {
//     .owner      = THIS_MODULE,
//     .open       = pet_seq_open,
//     .read       = seq_read,
//     .llseek     = seq_lseek,
//     .release    = single_release,
// };

// bar --------------------------------------------------------------


static struct fb_data bar_data = { "bar", "blank"};

static ssize_t bar_entry_read(struct file *fp, char __user *buf, size_t size, loff_t *offp )
{
    char res[128];
    int len;

    if  (*offp > 0)  return 0;

    len = sprintf(res, "%s is %s", bar_data.name, bar_data.value);
    if  (len < 0)  return -1;

    if  (size < len)  return -1;

    if  (copy_to_user(buf, res, len) != 0)  return -1;
    
    // operation per file
    *offp += len;

    return len;
}

static ssize_t bar_entry_write(struct file *fp, const char __user *buf, size_t size, loff_t *offp)
{
    if  (size > FOOBAR_LEN - 1)  return -1;

    if  (copy_from_user(bar_data.value, buf, size) != 0)  return -1;
    bar_data.value[size] = '\0';

    *offp += size;

    return size;
}

static const struct file_operations bar_entry_fops = 
{
    .owner      = THIS_MODULE,
    .read       = bar_entry_read,
    .write      = bar_entry_write,
};

// jiffies ----------------------------------------------------------


static ssize_t jiffies_entry_read(struct file *fp, char __user *buf, size_t size, loff_t *offp)
{
    char res[100];
    int len = 0;
    
    pr_debug("offp: %lx, off: %lld\n", (long unsigned int)offp, *offp);
    if  (*offp > 0)  return 0;

    len = sprintf(res, "jiffies = %ld\n", jiffies);
    if  (len < 0)  return -1;

    if  (size < len)  return -1;

    if  (copy_to_user(buf, res, len) != 0)  return -1;
    
    // operation per file
    *offp += len;

    return len;
}


static const struct file_operations jiffies_entry_fops = 
{
    .owner      = THIS_MODULE,
    .read       = jiffies_entry_read,
};



// init --------------------------------------------------------------

// static int __init pet_entry_init(void)
// {
//     proc_create("proc_entry1", 0, init_net.proc_net, &pet_entry_fops);

//     return 0;
// }

// static void __exit pet_entry_exit(void)
// {
//     remove_proc_entry("proc_entry1", init_net.proc_net);
// }

static struct proc_dir_entry *example_entry, *foo_entry, *bar_entry, *jiffies_entry;

static int __init proc_entry1_init(void)
{

    example_entry = proc_mkdir("example", NULL);
    if  (!example_entry) goto err_example;

    jiffies_entry = proc_create("jiffies", 0444, example_entry, &jiffies_entry_fops);
    if  (!jiffies_entry)  goto err_jiffies;

    bar_entry = proc_create("bar", 0, example_entry, &bar_entry_fops);
    if  (!bar_entry)  goto err_bar;

    pr_info("proc_entry1 inserted\n");
    return 0;

    
err_bar:
    remove_proc_entry("jiffies", example_entry);
err_jiffies:
    remove_proc_entry("example", NULL);
err_example:
    return -1;
}

static void __exit proc_entry1_exit(void)
{
    remove_proc_entry("bar", example_entry);
    remove_proc_entry("jiffies", example_entry);
	remove_proc_entry("example", NULL);
    
    pr_info("proc_entry1 removed\n");
    pr_debug("-------------------------------------------------\n");
}


module_init(proc_entry1_init);
module_exit(proc_entry1_exit);
MODULE_LICENSE("GPL");
