

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


static int foo_entry_show(struct seq_file *seq, void *arg)
{
    // arg is not for inode data
    struct fb_data* data = PDE_DATA(file_inode(seq->file));;
    pr_debug("foo_entry_show, arg: %lx, data: %lx\n", 
            (unsigned long)arg, (unsigned long)data);

    seq_puts(seq, "hello, ");
    
    seq_printf(seq, "%s is %s", data->name, data->value);

    return 0;
}

static int foo_entry_open(struct inode *inode, struct file *file)
{
    void* data = PDE_DATA(inode);
    pr_debug("foo_entry_open, data: %lx\n", (unsigned long)data);


    return single_open(file, foo_entry_show, NULL);
}

static ssize_t foo_entry_write(struct file *fp, const char __user *buf, size_t size, loff_t *offp)
{
    struct fb_data* data = PDE_DATA(file_inode(fp));
    pr_debug("foo_entry_write, data: %lx\n", (unsigned long)data);

    if  (size > FOOBAR_LEN - 1)  return -1;

    if  (copy_from_user(data->value, buf, size) != 0)  return -1;
    data->value[size] = '\0';

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



// init --------------------------------------------------------------



static struct proc_dir_entry *example_entry, *foo_entry;

struct fb_data foo_data = {
    .name = "foo", 
    .value = "blank"
};

static int __init proc_entry1_init(void)
{

    example_entry = proc_mkdir("example", NULL);
    if  (!example_entry) goto err_example;


    pr_debug("foo_data: %lx\n", (unsigned long)&foo_data);
    foo_entry = proc_create_data("foo", 0, example_entry, &foo_entry_fops, &foo_data);
    if  (!foo_entry)  goto err_foo;

    pr_info("proc_entry1 inserted\n");
    return 0;

err_foo:
    remove_proc_entry("example", NULL);
err_example:
    return -1;
}

static void __exit proc_entry1_exit(void)
{
    remove_proc_entry("foo", example_entry);
	remove_proc_entry("example", NULL);
    
    pr_info("proc_entry1 removed\n");
    pr_debug("-------------------------------------------------\n");
}


module_init(proc_entry1_init);
module_exit(proc_entry1_exit);
MODULE_LICENSE("GPL");
