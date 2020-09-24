

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
	spinlock_t       lock; // for rcu, this use as write lock
	long long num;
};


struct pe_map_entry
{
	struct hlist_node hlist;
    struct rcu_head rcu;
    struct kmem_cache* cache;
	
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
    int i;

    struct pe_hashmap* map = vmalloc(sizeof(struct pe_hashmap));
    if  (!map)  
        goto map_err;

    map->buckets_bits = buckets_bits;
    map->buckets_num = 1 << map->buckets_bits;
    map->buckets_mask = map->buckets_num - 1;

    map->buckets = vmalloc(sizeof(struct pe_map_bucket) * map->buckets_num);
    if  (!map->buckets)  
        goto buckets_err;

    for (i = 0; i < map->buckets_num; i++)
    {
        INIT_HLIST_HEAD(&map->buckets[i].head);
        spin_lock_init(&map->buckets[i].lock);
        map->buckets[i].num = 0;
    }

    map->cache = kmem_cache_create("pe_map_cache", 
	    sizeof(struct pe_map_entry), 0, SLAB_HWCACHE_ALIGN, NULL);
    if  (!map->cache)
        goto cache_err;



    return map;

cache_err:
    vfree(map->buckets);
buckets_err:
    vfree(map);
map_err:
    return NULL;
}

static void pe_map_flush(struct pe_hashmap* map);

static void pe_map_destroy(struct pe_hashmap* map)
{
    pe_map_flush(map);

    kmem_cache_destroy(map->cache);

    vfree(map->buckets);

    vfree(map);
}


static void pe_entry_free_rcu(struct rcu_head *head)
{
    struct pe_map_entry* entry = container_of(head, struct pe_map_entry, rcu);

    kmem_cache_free(entry->cache, entry);
}


static unsigned long long pe_entry_hash(long long key)
{
    unsigned long long src = key;
    unsigned long long ans = src;
    ans ^= src >> 16;
    ans ^= src >> 32;
    ans ^= src >> 48;

    return ans;
}


static int pe_map_insert(struct pe_hashmap* map, long long key, long long value)
{
    // pr_debug("pe_map_insert: 1, map: %p, key: %lld, vlaue: %lld\n", map, key, value);

    unsigned long long index = pe_entry_hash(key) & map->buckets_mask;
    struct pe_map_bucket* bucket = &map->buckets[index];
    struct hlist_head* head = &bucket->head;
    struct hlist_node* node;
    struct pe_map_entry* entry;
    struct pe_map_entry* old_entry = NULL;

    struct pe_map_entry* new_entry = kmem_cache_alloc(map->cache, GFP_ATOMIC);
    if  (!new_entry) 
        return -1;
    new_entry->key = key;
    new_entry->value = value;
    new_entry->cache = map->cache;

    
    spin_lock_bh(&bucket->lock);

    hlist_for_each_entry_safe(entry, node, head, hlist)
    {
        if  (entry->key == key)
        {
            // pr_debug("insert found existed key: %lld\n", key);

            hlist_replace_rcu(&entry->hlist, &new_entry->hlist);
            old_entry = entry;

            break;
        }
    }

    if  (!old_entry)
    {
        hlist_add_head_rcu(&new_entry->hlist, head);
        bucket->num++;
    }

    spin_unlock_bh(&bucket->lock);

    if  (old_entry)
    {
        call_rcu(&old_entry->rcu, pe_entry_free_rcu);
    }


    return 0;
}

static int pe_map_remove(struct pe_hashmap* map, long long key)
{
    // pr_debug("pe_map_remove: 1, map: %p, key: %lld\n", map, key);

    unsigned long long index = pe_entry_hash(key) & map->buckets_mask;
    struct pe_map_bucket* bucket = &map->buckets[index];
    struct hlist_head* head = &bucket->head;
    struct hlist_node* node;
    struct pe_map_entry* entry;
    struct pe_map_entry* old_entry = NULL;
    int ret = -1;

    spin_lock_bh(&bucket->lock);

    hlist_for_each_entry_safe(entry, node, head, hlist)
    {
        if  (entry->key == key)
        {
            hlist_del_rcu(&entry->hlist);
            old_entry = entry;
            bucket->num--;

            ret = 0;
            break;
        }
    }

    spin_unlock_bh(&bucket->lock);

    if  (old_entry)
    {
        call_rcu(&old_entry->rcu, pe_entry_free_rcu);
    }

    // if  (ret < 0)  pr_debug("remove failed, key: %lld\n", key);
    return ret;
}

static int pe_map_get(struct pe_hashmap* map, long long key, long long* value)
{
    // pr_debug("pe_map_get: 1,    map: %p, key: %lld, vlaue: %p\n", map, key, value);

    unsigned long long index = pe_entry_hash(key) & map->buckets_mask;
    struct pe_map_bucket* bucket = &map->buckets[index];
    struct hlist_head* head = &bucket->head;
    struct pe_map_entry* entry;
    int ret = -1;

    rcu_read_lock();
  
    hlist_for_each_entry_rcu(entry, head, hlist)
    {
        if  (entry->key == key)
        {
            *value = entry->value;
            ret = 0;
            break;
        }
    }

    rcu_read_unlock();

    // if  (ret  < 0)  pr_debug("get error, key: %lld\n", key);
    return ret;
}

static void pe_map_flush(struct pe_hashmap* map)
{
    int i;
    long long count = 0;
    struct hlist_head* heads = vmalloc(sizeof(struct hlist_head) * map->buckets_num);

    for (i = 0; i < map->buckets_num; i++)
    {
        struct pe_map_bucket* bucket = &map->buckets[i];
        struct hlist_head* head = &bucket->head;

        spin_lock_bh(&bucket->lock);

        heads[i] = *head;

        INIT_HLIST_HEAD(head);
        bucket->num = 0;

        spin_unlock_bh(&bucket->lock);
    }

    synchronize_rcu();
    rcu_barrier();

    for (i = 0; i < map->buckets_num; i++)
    {
        struct hlist_head* head = &heads[i];
        struct hlist_node* node;
        struct pe_map_entry* entry;

        hlist_for_each_entry_safe(entry, node, head, hlist)
        {
            hlist_del_rcu(&entry->hlist);
            kmem_cache_free(map->cache, entry);
            count++;
        }
    }

    vfree(heads);

    pr_info("pe_map_flush, count: %lld\n", count);
}


// hello entry --------------------------------------------------------------


static ssize_t entry_read(struct file *fp, char __user *buf, size_t size, loff_t *offp )
{
    struct pe_hashmap* map = PDE_DATA(file_inode(fp));
    struct hello_entry_param param;

    
    // pr_debug("entry_read: 1, fp: %p, map: %p, off: %lld\n", fp, map, *offp); // (*offp)++
    *offp += 1;

    if  (size != sizeof(param))  return -1;

    if  (copy_from_user(&param, buf, size) != 0)  return -1;

    // pr_debug("entry_read: 2, operation: %lld, key: %lld, value: %lld\n", 
    //     param.operation, param.key, param.value);

    if  (param.operation != HELLO_ENTRY_GET)  return -1;

    if  (pe_map_get(map, param.key, &param.value) < 0)  return -1;

    if  (copy_to_user(buf, &param, sizeof(param)) != 0)  return -1;

    return 0;
}

static ssize_t entry_write(struct file *fp, const char __user *buf, size_t size, loff_t *offp)
{
    struct pe_hashmap* map = PDE_DATA(file_inode(fp));
    struct hello_entry_param param;

    // pr_debug("entry_write: 1, fp: %p, map: %p, off: %lld\n", fp, map, *offp);
    *offp += 1;

    if  (size != sizeof(param))  return -1;

    if  (copy_from_user(&param, buf, size) != 0)  return -1;

    // pr_debug("entry_write: 2, operation: %lld, key: %lld, vlaue: %lld\n",
    //     param.operation, param.key, param.value);

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




static int __init proc_entry_hashmap2_init(void)
{

    example_entry = proc_mkdir("example", NULL);
    if  (!example_entry) 
        goto err_example;

    hello_map = pe_map_create(16);
    if  (!hello_map)
        goto err_hello_map;


    hello_entry = proc_create_data("hello", 0, example_entry, &entry_fops, hello_map);
    if  (!hello_entry)  
        goto err_hello_entry;
    pr_debug("hello_entry: %p, hello_map: %p\n", hello_entry, &hello_map);


    pr_info("proc_entry_hashmap2 inserted\n");
    return 0;


err_hello_entry:
    pe_map_destroy(hello_map);
err_hello_map:
    remove_proc_entry("example", NULL);
err_example:
    return -1;
}

static void __exit proc_entry_hashmap2_exit(void)
{

    remove_proc_entry("hello", example_entry);

    pe_map_destroy(hello_map);

	remove_proc_entry("example", NULL);
    
    pr_info("proc_entry_hashmap2 removed\n");
    pr_debug("-------------------------------------------------\n");
}


module_init(proc_entry_hashmap2_init);
module_exit(proc_entry_hashmap2_exit);
MODULE_LICENSE("GPL");
