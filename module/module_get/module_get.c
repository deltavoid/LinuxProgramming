

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>

#include <linux/skbuff.h>
#include <linux/netdevice.h>

#include <net/tcp.h>
#include <net/inet_common.h>
#include <net/net_namespace.h>
#include <net/ipv6.h>
#include <net/transp_v6.h>
#include <net/sock.h>



// module init ----------------------------------------------------------

static int __init module_get_init(void)
{
    pr_info("module_get_init begin\n");

    struct inet_connection_sock_af_ops *ipv4_specific_p;
    ipv4_specific_p = (struct inet_connection_sock_af_ops *)kallsyms_lookup_name("ipv4_specific");
    if  (!ipv4_specific_p) {
        pr_debug("ipv4_specific not found\n");
        return -1;
    }
    unsigned long syn_recv_sock_v4_p = (unsigned long)&ipv4_specific_p->syn_recv_sock;


    preempt_disable();

    struct module* mod = __module_address((unsigned long)syn_recv_sock_v4_p);

    pr_debug("mod: %lx\n", (unsigned long)mod);



    preempt_enable();




    pr_info("module_get_init end\n");
    return 0;
}

static void __exit module_get_exit(void)
{
    pr_info("module_get_exit begin\n");


    pr_info("module_get_exit end\n");
    pr_debug("-------------------------------------------------\n");
}


module_init(module_get_init);
module_exit(module_get_exit);
MODULE_LICENSE("GPL");
