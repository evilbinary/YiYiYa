
#ifndef _NETINET_TCP_H
#define _NETINET_TCP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define TCP_NODELAY 1
#define TCP_MAXSEG	 2
#define TCP_CORK	 3
#define TCP_KEEPIDLE	 4
#define TCP_KEEPINTVL	 5
#define TCP_KEEPCNT	 6
#define TCP_SYNCNT	 7
#define TCP_LINGER2	 8
#define TCP_DEFER_ACCEPT 9
#define TCP_WINDOW_CLAMP 10
#define TCP_INFO	 11
#define	TCP_QUICKACK	 12
#define TCP_CONGESTION	 13
#define TCP_MD5SIG	 14
#define TCP_THIN_LINEAR_TIMEOUTS 16
#define TCP_THIN_DUPACK  17
#define TCP_USER_TIMEOUT 18
#define TCP_REPAIR       19
#define TCP_REPAIR_QUEUE 20
#define TCP_QUEUE_SEQ    21
#define TCP_REPAIR_OPTIONS 22
#define TCP_FASTOPEN     23
#define TCP_TIMESTAMP    24
#define TCP_NOTSENT_LOWAT 25
#define TCP_CC_INFO      26
#define TCP_SAVE_SYN     27
#define TCP_SAVED_SYN    28
#define TCP_REPAIR_WINDOW 29
#define TCP_FASTOPEN_CONNECT 30
#define TCP_ULP          31
#define TCP_MD5SIG_EXT   32
#define TCP_FASTOPEN_KEY 33
#define TCP_FASTOPEN_NO_COOKIE 34
#define TCP_ZEROCOPY_RECEIVE   35
#define TCP_INQ          36
#define TCP_TX_DELAY     37

#define TCP_CM_INQ TCP_INQ

#define TCP_ESTABLISHED  1
#define TCP_SYN_SENT     2
#define TCP_SYN_RECV     3
#define TCP_FIN_WAIT1    4
#define TCP_FIN_WAIT2    5
#define TCP_TIME_WAIT    6
#define TCP_CLOSE        7
#define TCP_CLOSE_WAIT   8
#define TCP_LAST_ACK     9
#define TCP_LISTEN       10
#define TCP_CLOSING      11

enum {
	TCP_NLA_PAD,
	TCP_NLA_BUSY,
	TCP_NLA_RWND_LIMITED,
	TCP_NLA_SNDBUF_LIMITED,
	TCP_NLA_DATA_SEGS_OUT,
	TCP_NLA_TOTAL_RETRANS,
	TCP_NLA_PACING_RATE,
	TCP_NLA_DELIVERY_RATE,
	TCP_NLA_SND_CWND,
	TCP_NLA_REORDERING,
	TCP_NLA_MIN_RTT,
	TCP_NLA_RECUR_RETRANS,
	TCP_NLA_DELIVERY_RATE_APP_LMT,
	TCP_NLA_SNDQ_SIZE,
	TCP_NLA_CA_STATE,
	TCP_NLA_SND_SSTHRESH,
	TCP_NLA_DELIVERED,
	TCP_NLA_DELIVERED_CE,
	TCP_NLA_BYTES_SENT,
	TCP_NLA_BYTES_RETRANS,
	TCP_NLA_DSACK_DUPS,
	TCP_NLA_REORD_SEEN,
	TCP_NLA_SRTT,
	TCP_NLA_TIMEOUT_REHASH,
	TCP_NLA_BYTES_NOTSENT,
};

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
#define TCPOPT_EOL              0
#define TCPOPT_NOP              1
#define TCPOPT_MAXSEG           2
#define TCPOPT_WINDOW           3
#define TCPOPT_SACK_PERMITTED   4
#define TCPOPT_SACK             5
#define TCPOPT_TIMESTAMP        8
#define TCPOLEN_SACK_PERMITTED  2
#define TCPOLEN_WINDOW          3
#define TCPOLEN_MAXSEG          4
#define TCPOLEN_TIMESTAMP       10

#define SOL_TCP 6

#endif
 

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  /* _NETINET_TCP_H */