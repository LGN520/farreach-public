#ifndef _NF_CONNTRACK_WRAPPER_H
#define _NF_CONNTRACK_WRAPPER_H

#include_next <net/netfilter/nf_conntrack.h>

#ifndef HAVE_NF_CT_GET_TUPLEPR_TAKES_STRUCT_NET
static inline bool rpl_nf_ct_get_tuplepr(const struct sk_buff *skb,
					 unsigned int nhoff,
					 u_int16_t l3num, struct net *net,
					 struct nf_conntrack_tuple *tuple)
{
	return nf_ct_get_tuplepr(skb, nhoff, l3num, tuple);
}
#define nf_ct_get_tuplepr rpl_nf_ct_get_tuplepr
#endif

#ifndef HAVE_NF_CT_SET
static inline void
nf_ct_set(struct sk_buff *skb, struct nf_conn *ct, enum ip_conntrack_info info)
{
       skb->nfct = &ct->ct_general;
       skb->nfctinfo = info;
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0)
int rpl_nf_ct_netns_get(struct net *net, u8 nfproto);
void rpl_nf_ct_netns_put(struct net *net, u8 nfproto);
#define nf_ct_netns_get rpl_nf_ct_netns_get
#define nf_ct_netns_put rpl_nf_ct_netns_put
#endif

#endif /* _NF_CONNTRACK_WRAPPER_H */
