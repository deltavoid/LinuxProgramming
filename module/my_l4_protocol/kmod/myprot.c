// code from zhaoya
/*
 * 注意：
 * 1. CREATE_TRACE_POINTS 必须define
 * 2. CREATE_TRACE_POINTS 必须在头文件之前define
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>

#define CREATE_TRACE_POINTS
#include "myprot.h"



#include <net/protocol.h> 
#include <linux/ip.h>
#include <linux/udp.h>

#define IPPROTO_MYPROTO  123
int myproto_rcv(struct sk_buff *skb)
{
    struct udphdr *uh;
    struct iphdr *iph;

    iph = ip_hdr(skb);
    uh = udp_hdr(skb);

    printk("get protocol 123 skb\n");
    // 这里增加一个tracepoint点
    trace_myprot_port(uh->dest, uh->source);

    kfree_skb(skb);
    return 0;
}

static const struct net_protocol myproto_protocol = {
    .handler = myproto_rcv,
    .no_policy = 1,
    .netns_ok = 1,
};

int init_module(void)
{
    int ret = 0;

    printk("init_module: 1, begin\n");
    ret = inet_add_protocol(&myproto_protocol, IPPROTO_MYPROTO);
    if (ret) {
        printk("failed\n");
        return ret;
    }

    printk("successful\n");
    return 0;
}
void cleanup_module(void)
{
    inet_del_protocol(&myproto_protocol, IPPROTO_MYPROTO);
}

int init_module(void);
void cleanup_module(void);
MODULE_LICENSE("GPL");