

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



// extern struct proc_dir_entry *proc_create_data(const char *, umode_t,
// 		struct proc_dir_entry *, const struct file_operations *, void *);
// extern void *PDE_DATA(const struct inode *);


#define FOOBAR_LEN 60

struct fb_data{
	char name[FOOBAR_LEN];
	char value[FOOBAR_LEN];
};


// foo ---------------------------------------------------------

struct fb_data foo_data = {"foo", "blank"};

static int foo_entry_show(struct seq_file *seq, void *arg)
{
    pr_debug("foo_entry_show\n");

    seq_puts(seq, "hello, ");
    
    seq_printf(seq, "%s is %s", foo_data.name, foo_data.value);


    return 0;
}

static int foo_entry_open(struct inode *inode, struct file *file)
{
    pr_debug("foo_entry_open\n");

    return single_open(file, foo_entry_show, NULL);
}

static ssize_t foo_entry_write(struct file *fp, const char __user *buf, size_t size, loff_t *offp)
{
    if  (size > FOOBAR_LEN - 1)  return -1;

    if  (copy_from_user(foo_data.value, buf, size) != 0)  return -1;
    foo_data.value[size] = '\0';

    *offp += size;

    return size;
}


static const struct file_operations foo_entry_fops = 
{
    .owner      = THIS_MODULE,
    .open       = foo_entry_open,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = single_release,
    .write      = foo_entry_write,
};

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
    int* data = PDE_DATA(file_inode(fp));
    
    pr_debug("data: %p, %x, offp: %lx, off: %lld\n", data, *data,  (long unsigned int)offp, *offp);
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



static struct proc_dir_entry *example_entry, *foo_entry, *bar_entry, *jiffies_entry;

int data = 0x12345678;

static int __init proc_entry2_init(void)
{

    example_entry = proc_mkdir("example", NULL);
    if  (!example_entry) goto err_example;

    jiffies_entry = proc_create_data("jiffies", 0444, example_entry, &jiffies_entry_fops, &data);
    if  (!jiffies_entry)  goto err_jiffies;

    bar_entry = proc_create("bar", 0, example_entry, &bar_entry_fops);
    if  (!bar_entry)  goto err_bar;

    foo_entry = proc_create("foo", 0, example_entry, &foo_entry_fops);
    if  (!foo_entry)  goto err_foo;

    pr_info("proc_entry2 inserted\n");
    return 0;

err_foo:
    remove_proc_entry("bar", example_entry);
err_bar:
    remove_proc_entry("jiffies", example_entry);
err_jiffies:
    remove_proc_entry("example", NULL);
err_example:
    return -1;
}

static void __exit proc_entry2_exit(void)
{
    remove_proc_entry("foo", example_entry);
    remove_proc_entry("bar", example_entry);
    remove_proc_entry("jiffies", example_entry);
	remove_proc_entry("example", NULL);
    
    pr_info("proc_entry2 removed\n");
    pr_debug("-------------------------------------------------\n");
}


module_init(proc_entry2_init);
module_exit(proc_entry2_exit);
MODULE_LICENSE("GPL");
