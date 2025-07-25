
#ifndef NETLINK_MSG_H_
#define NETLINK_MSG_H_

#include <netlink/netlink.h>
#include <netlink/object.h>
#include <netlink/attr.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NL_DONTPAD	0

/**
 * @ingroup msg
 * @brief
 * Will cause the netlink pid to be set to the pid assigned to
 * the netlink handle (socket) just before sending the message off.
 * @note Requires the use of nl_send_auto_complete()!
 */
#define NL_AUTO_PID	0

/**
 * @ingroup msg
 * @brief
 * May be used to refer to a sequence number which should be
 * automatically set just before sending the message off.
 * @note Requires the use of nl_send_auto_complete()!
 */
#define NL_AUTO_SEQ	0

struct nl_msg;
struct nl_tree;
struct ucred;

/* size calculations */
extern int		  nlmsg_msg_size(int);
extern int		  nlmsg_total_size(int);
extern int		  nlmsg_padlen(int);

/* payload access */
extern void *		  nlmsg_data(const struct nlmsghdr *);
extern int		  nlmsg_len(const struct nlmsghdr *);
extern void *		  nlmsg_tail(const struct nlmsghdr *);

/* attribute access */
extern struct nlattr *	  nlmsg_attrdata(const struct nlmsghdr *, int);
extern int		  nlmsg_attrlen(const struct nlmsghdr *, int);

/* message parsing */
extern int		  nlmsg_valid_hdr(const struct nlmsghdr *, int);
extern int		  nlmsg_ok(const struct nlmsghdr *, int);
extern struct nlmsghdr *  nlmsg_next(struct nlmsghdr *, int *);
extern int		  nlmsg_parse(struct nlmsghdr *, int, struct nlattr **,
				      int, struct nla_policy *);
extern struct nlattr *	  nlmsg_find_attr(struct nlmsghdr *, int, int);
extern int		  nlmsg_validate(struct nlmsghdr *, int, int,
					 struct nla_policy *);

/* Backward compatibility */
#define nlmsg_new()	   		nlmsg_alloc()
#define nlmsg_build_simple(a, b)	nlmsg_alloc_simple(a, b)
#define nlmsg_build(ptr)		nlmsg_inherit(ptr)

extern struct nl_msg *	  nlmsg_alloc(void);
extern struct nl_msg *	  nlmsg_alloc_size(size_t);
extern struct nl_msg *	  nlmsg_alloc_simple(int, int);
extern void		  nlmsg_set_default_size(size_t);
extern struct nl_msg *	  nlmsg_inherit(struct nlmsghdr *);
extern struct nl_msg *	  nlmsg_convert(struct nlmsghdr *);
extern void *		  nlmsg_reserve(struct nl_msg *, size_t, int);
extern int		  nlmsg_append(struct nl_msg *, void *, size_t, int);
extern int		  nlmsg_expand(struct nl_msg *, size_t);

extern struct nlmsghdr *  nlmsg_put(struct nl_msg *, uint32_t, uint32_t,
				    int, int, int);
extern struct nlmsghdr *  nlmsg_hdr(struct nl_msg *);
extern void		  nlmsg_free(struct nl_msg *);

/* attribute modification */
extern void		  nlmsg_set_proto(struct nl_msg *, int);
extern int		  nlmsg_get_proto(struct nl_msg *);
extern size_t		  nlmsg_get_max_size(struct nl_msg *);
extern void		  nlmsg_set_src(struct nl_msg *, struct sockaddr_nl *);
extern struct sockaddr_nl *nlmsg_get_src(struct nl_msg *);
extern void		  nlmsg_set_dst(struct nl_msg *, struct sockaddr_nl *);
extern struct sockaddr_nl *nlmsg_get_dst(struct nl_msg *);
extern void		  nlmsg_set_creds(struct nl_msg *, struct ucred *);
extern struct ucred *	  nlmsg_get_creds(struct nl_msg *);

extern char *		  nl_nlmsgtype2str(int, char *, size_t);
extern int		  nl_str2nlmsgtype(const char *);

extern char *		  nl_nlmsg_flags2str(int, char *, size_t);

extern int		  nl_msg_parse(struct nl_msg *,
				       void (*cb)(struct nl_object *, void *),
				       void *);

extern void		nl_msg_dump(struct nl_msg *, FILE *);

/**
 * @name Iterators
 * @{
 */

/**
 * @ingroup msg
 * Iterate over a stream of attributes in a message
 * @arg pos	loop counter, set to current attribute
 * @arg nlh	netlink message header
 * @arg hdrlen	length of family header
 * @arg rem	initialized to len, holds bytes currently remaining in stream
 */
#define nlmsg_for_each_attr(pos, nlh, hdrlen, rem) \
	nla_for_each_attr(pos, nlmsg_attrdata(nlh, hdrlen), \
			  nlmsg_attrlen(nlh, hdrlen), rem)

/**
 * Iterate over a stream of messages
 * @arg pos	loop counter, set to current message
 * @arg head	head of message stream
 * @arg len	length of message stream
 * @arg rem	initialized to len, holds bytes currently remaining in stream
 */
#define nlmsg_for_each_msg(pos, head, len, rem) \
	for (pos = head, rem = len; \
	     nlmsg_ok(pos, rem); \
	     pos = nlmsg_next(pos, &(rem)))

/** @} */

#ifdef __cplusplus
}
#endif

#endif
