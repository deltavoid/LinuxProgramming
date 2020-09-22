

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





// hello --------------------------------------------------------------


#define ENTRY_LEN 60

struct entry{
	char name[ENTRY_LEN];
	char value[ENTRY_LEN];
};




static ssize_t entry_read(struct file *fp, char __user *buf, size_t size, loff_t *offp )
{
    struct entry* entry = PDE_DATA(file_inode(fp));
    char res[128];
    int len;

    
    pr_debug("entry_read: 1, fp: %p, entry: %p, off: %lld\n", fp, entry, *offp);

    if  (*offp > 0)  return 0;

    len = sprintf(res, "%s is %s", entry->name, entry->value);
    if  (len < 0)  return -1;

    if  (size < len)  return -1;

    if  (copy_to_user(buf, res, len) != 0)  return -1;
    
    // operation per file
    *offp += len;

    return len;
}

static ssize_t entry_write(struct file *fp, const char __user *buf, size_t size, loff_t *offp)
{
    struct entry* entry = PDE_DATA(file_inode(fp));

    pr_debug("entry_write: 1, fp: %p, entry: %p, off: %lld\n", fp, entry, *offp);

    if  (size > ENTRY_LEN - 1)  return -1;

    if  (copy_from_user(entry->value, buf, size) != 0)  return -1;
    entry->value[size] = '\0';

    *offp += size;

    return size;
}

static const struct file_operations entry_fops = 
{
    .owner      = THIS_MODULE,
    .read       = entry_read,
    .write      = entry_write,
};




// init-----------------------------------------------------------------------------


static struct proc_dir_entry *example_entry;

static struct proc_dir_entry *hello_entry;
static struct entry hello_data = { .name = "hello", .value = "world\n"};




static int __init proc_entry2_init(void)
{

    example_entry = proc_mkdir("example", NULL);
    if  (!example_entry) goto err_example;


    hello_entry = proc_create_data("hello", 0, example_entry, &entry_fops, &hello_data);
    if  (!hello_entry)  goto err_hello;
    pr_debug("hello_entry: %p, hello_data: %p\n", hello_entry, &hello_data);


    pr_info("proc_entry2 inserted\n");
    return 0;


err_hello:
    remove_proc_entry("example", NULL);
err_example:
    return -1;
}

static void __exit proc_entry2_exit(void)
{

    remove_proc_entry("hello", example_entry);

	remove_proc_entry("example", NULL);
    
    pr_info("proc_entry2 removed\n");
    pr_debug("-------------------------------------------------\n");
}


module_init(proc_entry2_init);
module_exit(proc_entry2_exit);
MODULE_LICENSE("GPL");
