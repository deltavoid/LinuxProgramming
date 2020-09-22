

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

#include "proc_entry_hashmap.h"




// extern struct proc_dir_entry *proc_create_data(const char *, umode_t,
// 		struct proc_dir_entry *, const struct file_operations *, void *);
// extern void *PDE_DATA(const struct inode *);




// pe_hashmap ---------------------------------------------------------------


struct pe_map_bucket
{
	struct hlist_head head;
	spinlock_t       lock;
	long long num;
};


struct pe_map_entry
{
	struct hlist_node hlist;
	
	long long key;
    long long value;
};


struct pe_hashmap
{
    struct pe_map_bucket* buckets;
    struct kmem_cache* cache;
    unsigned buckets_bits;
    unsigned buckets_num;
    unsigned buckets_mask;
};



static struct pe_hashmap*  pe_map_create(int buckets_bits)
{
    struct pe_hashmap* map = vmalloc(sizeof(struct pe_hashmap));
    return map;
}

static void pe_map_destroy(struct pe_hashmap* map)
{
    vfree(map);
}

// static unsigned long long pe_entry_hash(long long key)
// {
//     return 0;
// }


static int pe_map_insert(struct pe_hashmap* map, long long key, long long value)
{
    pr_debug("pe_map_insert: 1, map: %p, key: %lld, vlaue: %lld\n", map, key, value);

    return 0;
}

static int pe_map_remove(struct pe_hashmap* map, long long key)
{
    pr_debug("pe_map_remove: 1, map: %p, key: %lld\n", map, key);

    return 0;
}

static int pe_map_get(struct pe_hashmap* map, long long key, long long* value)
{
    pr_debug("pe_map_get: 1, map: %p, key: %lld, vlaue: %p\n", map, key, value);

    *value = 12345678;


    return 0;
}



// hello entry --------------------------------------------------------------


static ssize_t entry_read(struct file *fp, char __user *buf, size_t size, loff_t *offp )
{
    struct pe_hashmap* map = PDE_DATA(file_inode(fp));
    struct hello_entry_param param;

    
    pr_debug("entry_read: 1, fp: %p, map: %p, off: %lld\n", fp, map, *offp); // (*offp)++
    *offp += 1;

    if  (size != sizeof(param))  return -1;

    if  (copy_from_user(&param, buf, size) != 0)  return -1;

    pr_debug("entry_read: 2, operation: %lld, key: %lld, value: %lld\n", 
        param.operation, param.key, param.value);

    if  (param.operation != HELLO_ENTRY_GET)  return -1;

    if  (pe_map_get(map, param.key, &param.value) < 0)  return -1;

    if  (copy_to_user(buf, &param, sizeof(param)) != 0)  return -1;

    return 0;
}

static ssize_t entry_write(struct file *fp, const char __user *buf, size_t size, loff_t *offp)
{
    struct pe_hashmap* map = PDE_DATA(file_inode(fp));
    struct hello_entry_param param;

    pr_debug("entry_write: 1, fp: %p, map: %p, off: %lld\n", fp, map, *offp);
    *offp += 1;

    if  (size != sizeof(param))  return -1;

    if  (copy_from_user(&param, buf, size) != 0)  return -1;

    pr_debug("entry_write: 2, operation: %lld, key: %lld, vlaue: %lld\n",
        param.operation, param.key, param.value);

    if  (param.operation == HELLO_ENTRY_INSERT)
    {
        if  (pe_map_insert(map, param.key, param.value) < 0)  return -1;
    }
    else if  (param.operation == HELLO_ENTRY_REMOVE)
    {
        if  (pe_map_remove(map, param.key) < 0)  return -1;
    }
    else
        return -1;

    return 0;
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
static struct pe_hashmap* hello_map;




static int __init proc_entry_hashmap1_init(void)
{

    example_entry = proc_mkdir("example", NULL);
    if  (!example_entry) 
        goto err_example;

    hello_map = pe_map_create(12);
    if  (!hello_map)
        goto err_hello_map;


    hello_entry = proc_create_data("hello", 0, example_entry, &entry_fops, hello_map);
    if  (!hello_entry)  
        goto err_hello_entry;
    pr_debug("hello_entry: %p, hello_map: %p\n", hello_entry, &hello_map);


    pr_info("proc_entry_hashmap1 inserted\n");
    return 0;


err_hello_entry:
    pe_map_destroy(hello_map);
err_hello_map:
    remove_proc_entry("example", NULL);
err_example:
    return -1;
}

static void __exit proc_entry_hashmap1_exit(void)
{

    remove_proc_entry("hello", example_entry);

    pe_map_destroy(hello_map);

	remove_proc_entry("example", NULL);
    
    pr_info("proc_entry_hashmap1 removed\n");
    pr_debug("-------------------------------------------------\n");
}


module_init(proc_entry_hashmap1_init);
module_exit(proc_entry_hashmap1_exit);
MODULE_LICENSE("GPL");
