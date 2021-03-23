/***********************************************************
** Copyright (C), 2008-2019, OPPO Mobile Comm Corp., Ltd.
** VENDOR_EDIT
** File: oppo_kernel2user.c
** Description: Add for kernel data info send to user space.
**
** Version: 1.0
** Date : 2019/10/02
** Author: Hao.Peng@TECH.CN.WiFi.Network.2527098,2019/10/02
**
** ------------------ Revision History:------------------------
** <author> <data> <version > <desc>
** penghao 2019/10/02 1.0 build this module
****************************************************************/

#include <linux/types.h>
#include <linux/ip.h>
#include <linux/netfilter.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/icmp.h>
#include <linux/kernel.h>
#include <linux/sysctl.h>
#include <net/route.h>
#include <net/ip.h>
#include <linux/bitops.h>
#include <linux/err.h>
#include <linux/version.h>
#include <net/tcp.h>
#include <linux/random.h>
#include <net/dst.h>
#include <linux/file.h>
#include <net/tcp_states.h>
#include <linux/netlink.h>
#include <net/sch_generic.h>
#include <net/pkt_sched.h>
#include <net/netfilter/nf_queue.h>
#include <linux/netfilter/xt_state.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter/xt_owner.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/ipv4/nf_conntrack_ipv4.h>
#include <net/oppo/oppo_kernel2user.h>


static DEFINE_MUTEX(kernel2user_netlink_mutex);
static struct ctl_table_header *oppo_kernel2user_table_hrd;

static u32 kernel2user_debug = 0;
static u32 threshold_retansmit = 10;
static u32 smooth_factor = 20;
static u32 protect_score = 60;
static u32 second_wifi_score = 101;
static u32 second_cell_score = 101;
static u32 background_score = 101;

u32 oppo_mptcp_uid[MAX_MPTCP_APP_LEN] = {0};
u32 oppo_mptcp_len = 0;
EXPORT_SYMBOL(oppo_mptcp_uid);
EXPORT_SYMBOL(oppo_mptcp_len);
u32 oppo_foreground_uid = 1;
//kernel sock
static struct sock *oppo_kernel2user_sock;

static int link_score_param[MAX_LINK_LEN][2] = {0}	/* record count_retransmit and count_normal */;
//static int count_retransmit = 0;
//static int count_normal = 0;
static int threshold_normal = 100;
static int special_pid = 100;
static int threshold_gap = 5;
static int last_score = 0;
static struct general_oppo_info notify_user_retransmit;

#define MARK_MASK    0x0fff

void send_score(int link_index, int score)
{
    printk("telluser score = %d, link_index=%d\n", score, link_index);

    link_score_param[link_index][1] = 0;
    link_score_param[link_index][0] = 0;

	if (last_score != 0) 
	{
		score = (score * (100 - smooth_factor) + last_score * smooth_factor) / 100;
	}

    score = score % 101;
    if (abs(score - last_score) < threshold_gap)
        return;

    if(score < protect_score && last_score < protect_score) {
	    return;
	}
    last_score = score;
    memset((void*)&notify_user_retransmit, 0x0, sizeof(notify_user_retransmit));
    notify_user_retransmit.para_type = OPPO_SEND_TCP_RETRANSMIT;
    notify_user_retransmit.para_one = oppo_foreground_uid; // uid
    notify_user_retransmit.para_two = score; //score
    oppo_kernel_send_to_user(OPPO_SEND_TCP_RETRANSMIT, (char *)&notify_user_retransmit, sizeof(notify_user_retransmit));
}

void oppo_handle_retransmit(const struct sock *sk, int type)
{
    uid_t sk_uid;
    int total = 0;
    int score = 0;
    int link_index = 0;
    //mutex_lock(&kernel2user_netlink_mutex);
	link_index = get_link_index_from_sock(sk);

    sk_uid = get_uid_from_sock(sk);

    if (kernel2user_debug)
    {
	    printk("oppo_kernel2user_netlink:uid = %u, type= %d, link_index = %d\n", sk_uid, type, link_index);
    }
	if (kernel2user_debug && sk != NULL)
    {
	    printk("oppo_kernel2user_netlink:sk->sk_rcv_saddr = %pI4, sk->sk_daddr=%d\n,", &sk->sk_rcv_saddr, sk->sk_daddr);
		if (sk->sk_rcv_saddr == 0x100007f) {
			printk("oppo_kernel2user_netlink:127.0.0.1 addr should not stats.\n,");
			return;
		}
    }
    if (sk_uid != oppo_foreground_uid && link_index == 0) {
        link_index = MAX_LINK_LEN - 1;
    }

    total = link_score_param[link_index][0] + link_score_param[link_index][1];
    if (type == 0)
    {
        link_score_param[link_index][1]++;
		if (link_score_param[link_index][1] % threshold_normal == 0 && total > 10) {
			score = 100 * link_score_param[link_index][1] / (total + 1); //score
			if (link_index == 0){
				send_score(link_index, score);
			} else if (link_index == 1) {
				second_wifi_score = score;
			} else if (link_index == 2) {
				second_cell_score = score;
			} else if (link_index == (MAX_LINK_LEN - 1)) {
				background_score = score;
			} 
		}
    } else if(type == 1) {
        link_score_param[link_index][0]++;
		if (link_score_param[link_index][0] % threshold_retansmit == 0 && total > 10) {
			score = 100 * link_score_param[link_index][1] / (total + 1); //score
			if (link_index == 0){
				send_score(link_index, score);
			} else if (link_index == 1) {
				second_wifi_score = score;
			} else if (link_index == 2) {
				second_cell_score = score;
			} else if (link_index == (MAX_LINK_LEN - 1)) {
				background_score = score;
			} 
		}
    } else if(type == -1) {
        link_score_param[link_index][1]--;
    }
    if (kernel2user_debug)
    {
        printk("oppo_kernel2user_netlink:count_retransmit=%d,count_normal=%d\n", link_score_param[link_index][0], link_score_param[link_index][1]);
    }
    //mutex_unlock(&kernel2user_netlink_mutex);

}

uid_t get_uid_from_sock(const struct sock *sk)
{
    uid_t sk_uid;
    #if (LINUX_VERSION_CODE < KERNEL_VERSION(4,9,0))
    const struct file *filp = NULL;
    #endif
    if(NULL == sk || !sk_fullsock(sk) || NULL == sk->sk_socket){
        return 0;
    }
    #if (LINUX_VERSION_CODE < KERNEL_VERSION(4,9,0))
    filp = sk->sk_socket->file;
    if(NULL == filp){
        return 0;
    }
    sk_uid = __kuid_val(filp->f_cred->fsuid);
    #else
    sk_uid = __kuid_val(sk->sk_uid);
    #endif
    return sk_uid;
}

#define WLAN1_MARK 0x200
#define CELL_MARK  0x400

// 0 -> default, 1 -> wlan1, 2 -> cell(not default).
int get_link_index_from_sock(const struct sock *sk)
{
    int result = 0;

    if(NULL == sk || 0 == sk->oppo_sla_mark){
        return result;
    }
	if ((sk->oppo_sla_mark & MARK_MASK) == WLAN1_MARK) {
		printk("get_link_index_from_sock:WLAN1_MARK \n");
		return 1;
	}
	if ((sk->oppo_sla_mark & MARK_MASK) == CELL_MARK) {
		printk("get_link_index_from_sock:CELL_MARK \n");
		return 2;
	}
    return result;
}


/* send to user space */
int oppo_kernel_send_to_user(int msg_type, char *payload, int payload_len)
{
	int ret = 0;
	struct sk_buff *skbuff;
	struct nlmsghdr *nlh;

	/*allocate new buffer cache */
	skbuff = alloc_skb(NLMSG_SPACE(payload_len), GFP_ATOMIC);
	if (skbuff == NULL) {
		printk("oppo_kernel2user_netlink: skbuff alloc_skb failed\n");
		return -1;
	}

	/* fill in the data structure */
	nlh = nlmsg_put(skbuff, 0, 0, msg_type, NLMSG_ALIGN(payload_len), 0);
	if (nlh == NULL) {
		printk("oppo_kernel2user_netlink:nlmsg_put failaure\n");
		nlmsg_free(skbuff);
		return -1;
	}

	//compute nlmsg length
	nlh->nlmsg_len = NLMSG_HDRLEN + NLMSG_ALIGN(payload_len);

	if(NULL != payload){
		memcpy((char *)NLMSG_DATA(nlh),payload,payload_len);
	}

	/* set control field,sender's pid */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,7,0))
	NETLINK_CB(skbuff).pid = 0;
#else
	NETLINK_CB(skbuff).portid = 0;
#endif

	NETLINK_CB(skbuff).dst_group = 0;

	/* send data */
	if(oppo_foreground_uid){
	    ret = netlink_unicast(oppo_kernel2user_sock, skbuff, special_pid, MSG_DONTWAIT);
	} else {
	    printk(KERN_ERR "oppo_kernel2user_netlink: can not unicast skbuff, oppo_foreground_uid=0\n");
	}
	if(ret < 0){
		printk(KERN_ERR "oppo_kernel2user_netlink: can not unicast skbuff,ret = %d\n",ret);
		return 1;
	}

	return 0;
}



static struct ctl_table oppo_kernel2user_sysctl_table[] = {
	{
		.procname	= "oplus_foreground_uid",
		.data		= &oppo_foreground_uid,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname	= "kernel2user_debug",
		.data		= &kernel2user_debug,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname	= "threshold_retansmit",
		.data		= &threshold_retansmit,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname	= "threshold_normal",
		.data		= &threshold_normal,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname	= "threshold_gap",
		.data		= &threshold_gap,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname	= "smooth_factor",
		.data		= &smooth_factor,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname	= "protect_score",
		.data		= &protect_score,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname	= "second_wifi_score",
		.data		= &second_wifi_score,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname	= "second_cell_score",
		.data		= &second_cell_score,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname	= "background_score",
		.data		= &background_score,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{ }
};

static int oppo_kernel2user_sysctl_init(void)
{
	oppo_kernel2user_table_hrd = register_net_sysctl(&init_net, "net/oplus_kernel2user",
		                                          oppo_kernel2user_sysctl_table);
	return oppo_kernel2user_table_hrd == NULL ? -ENOMEM : 0;
}

static int oppo_kernel2user_set_foreground_uid(struct nlmsghdr *nlh)
{
    u32 *data;
    data = (u32 *)NLMSG_DATA(nlh);
    oppo_foreground_uid = *data;
    //count_retransmit = 0;
    //count_normal = 0;
	link_score_param[0][0] = 0;
	link_score_param[1][1] = 0;
    printk("oppo_kernel2user_set_foreground_uid set uid=%d\n", oppo_foreground_uid);
    return 0;
}

static int oppo_kernel2user_set_mptcp_uid(struct nlmsghdr *nlh)
{
    u32 *data;
    int i = 0;

    data = (u32 *)NLMSG_DATA(nlh);
    memset(oppo_mptcp_uid, 0x0, MAX_MPTCP_APP_LEN * sizeof(u32));
    oppo_mptcp_len = *data;
    for (i = 0; i < oppo_mptcp_len; i++) {
        oppo_mptcp_uid[i] = *(data + i + 1);
	printk("oppo_kernel2user_set_mptcp_uid set uid=%d\n", oppo_mptcp_uid[i]);
    }
    return 0;
}

static int kernel2user_netlink_rcv_msg(struct sk_buff *skb, struct nlmsghdr *nlh, struct netlink_ext_ack *extack)
{
	int ret = 0;
	printk("kernel2user_netlink_rcv_msg type=%d.\n", nlh->nlmsg_type);
	switch (nlh->nlmsg_type) {
    	case OPPO_FOREGROUND_ANDROID_UID:
    		ret = oppo_kernel2user_set_foreground_uid(nlh);
    		break;
	case OPPO_MPTCP_UID:
	        ret = oppo_kernel2user_set_mptcp_uid(nlh);
    		break;
    	default:
    		return -EINVAL;
	}

	return ret;
}


static void kernel2user_netlink_rcv(struct sk_buff *skb)
{
	printk("kernel2user_netlink_rcv.\n");
	mutex_lock(&kernel2user_netlink_mutex);
	netlink_rcv_skb(skb, &kernel2user_netlink_rcv_msg);
	mutex_unlock(&kernel2user_netlink_mutex);
}

static int oppo_kernel2user_netlink_init(void)
{
	struct netlink_kernel_cfg cfg = {
		.input	= kernel2user_netlink_rcv,
	};

	oppo_kernel2user_sock = netlink_kernel_create(&init_net, NETLINK_OPPO_KERNEL2USER, &cfg);
	return oppo_kernel2user_sock == NULL ? -ENOMEM : 0;
}

static void oppo_kernel2user_netlink_exit(void)
{
	netlink_kernel_release(oppo_kernel2user_sock);
	oppo_kernel2user_sock = NULL;
}

static int __init oppo_kernel2user_init(void)
{
	int ret = 0;

	ret = oppo_kernel2user_netlink_init();
	if (ret < 0) {
		printk("oppo_kernel2user_init module failed to init netlink.\n");
	} else {
		printk("oppo_kernel2user_init module init netlink successfully.\n");
	}

	ret |= oppo_kernel2user_sysctl_init();

	if (ret < 0) {
		printk("oppo_kernel2user_init module failed to register netfilter ops.\n");
	} else {
		printk("oppo_kernel2user_init module register netfilter ops successfully.\n");
	}

	return ret;
}

static void __exit oppo_kernel2user_fini(void)
{
	oppo_kernel2user_netlink_exit();

	if(oppo_kernel2user_table_hrd){
		unregister_net_sysctl_table(oppo_kernel2user_table_hrd);
	}
}

module_init(oppo_kernel2user_init);
module_exit(oppo_kernel2user_fini);